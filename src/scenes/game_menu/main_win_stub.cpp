// Game Menu Scene (Windows stub build)
#include <bits/stdc++.h>
#include "steam_stub_local.h"
#include "../../objects/player.cpp"
#include "../../objects/header.cpp"
#include "../../objects/button.cpp"

#include "raylib.h"

using namespace std;

const float buttonDefaultWidth = 300.0f, buttonDefaultHeight = 100.0f;
const float buttonDefaultFontSize = 64.0f;
Color initButtonBgColor = BLACK;
Color initButtonTextColor = WHITE;

static const char *kLobbyCodeKey = "room_code";

enum class SceneMode {
    Menu,
    Settings,
    Lobby
};

enum class ScreenModePreset {
    MaximizedWindowed,
    Windowed,
    Fullscreen
};

struct ResolutionPreset {
    const char *label;
    int width;
    int height;
};

static const array<ResolutionPreset, 3> kResolutionPresets = {{
    {"1280 x 720", 1280, 720},
    {"1600 x 900", 1600, 900},
    {"1920 x 1080", 1920, 1080}
}};

struct LobbyState {
    enum class Mode {
        Idle,
        Creating,
        Searching,
        Joining,
        InLobby,
        Failed
    };

    Mode mode = Mode::Idle;
    CSteamID lobbyId;
    string activeCode;
    string joinCodeInput;
    string statusMessage;
    vector<string> memberNames;
    vector<uint64_t> memberIds;
    bool isHost = false;
    struct ChatMessage { string sender; string message; uint64_t ts; };
    vector<ChatMessage> chatMessages; // recent chat
    unordered_map<uint64_t, uint32_t> pings; // steamid -> ms
    string chatInput;
    int chatScroll = 0; // lines scrolled up from bottom
    bool chatFocused = false;
};

static bool MouseOver(const Rectangle &rect) {
    return CheckCollisionPointRec(GetMousePosition(), rect);
}

static string NormalizeLobbyCode(const string &value) {
    string result;
    result.reserve(6);

    for (char ch : value) {
        if (isalpha(static_cast<unsigned char>(ch))) {
            result.push_back(static_cast<char>(toupper(static_cast<unsigned char>(ch))));
        }

        if (result.size() == 6) {
            break;
        }
    }

    return result;
}

static string GenerateLobbyCode() {
    static random_device randomDevice;
    static mt19937 generator(randomDevice());
    static uniform_int_distribution<int> letterIndex(0, 25);

    string code;
    code.reserve(6);
    for (int i = 0; i < 6; ++i) {
        code.push_back(static_cast<char>('A' + letterIndex(generator)));
    }
    return code;
}

static void DrawHud(const string &playerName, bool steamInitialized) {
    string fpsText = "Client FPS: " + to_string(GetFPS());
    DrawText(fpsText.c_str(), 10, 10, 24, WHITE);
    DrawText(TextFormat("Welcome %s", steamInitialized ? playerName.c_str() : "Guest"), 10, 40, 24, WHITE);
}

static const char *GetScreenModeLabel(ScreenModePreset mode) {
    switch (mode) {
        case ScreenModePreset::MaximizedWindowed:
            return "Maximized Windowed";
        case ScreenModePreset::Windowed:
            return "Windowed";
        default:
            return "Fullscreen";
    }
}

static const char *GetResolutionLabel(int index) {
    if (index < 0 || index >= static_cast<int>(kResolutionPresets.size())) {
        return kResolutionPresets.front().label;
    }

    return kResolutionPresets[index].label;
}

static void ApplyScreenMode(ScreenModePreset mode, const ResolutionPreset &resolution) {
    if (mode == ScreenModePreset::Fullscreen) {
        if (!IsWindowState(FLAG_FULLSCREEN_MODE)) {
            ToggleFullscreen();
        }
        return;
    }

    if (IsWindowState(FLAG_FULLSCREEN_MODE)) {
        ToggleFullscreen();
    }

    ClearWindowState(FLAG_WINDOW_MAXIMIZED);
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowSize(resolution.width, resolution.height);
    SetWindowPosition(
        (GetMonitorWidth(0) - resolution.width) / 2,
        (GetMonitorHeight(0) - resolution.height) / 2
    );

    if (mode == ScreenModePreset::MaximizedWindowed) {
        MaximizeWindow();
    }
}

class SteamLobbyController {
public:
    SteamLobbyController(SceneMode &sceneMode, LobbyState &lobbyState)
        : sceneMode(sceneMode), lobbyState(lobbyState) {}

    void CreateLobby() {
        if (!SteamAPI_IsSteamRunning()) {
            Fail("Steam is not running.");
            return;
        }

        lobbyState.mode = LobbyState::Mode::Creating;
        lobbyState.statusMessage = "Creating lobby...";
        lobbyState.isHost = true;
        lobbyState.activeCode = GenerateLobbyCode();
        lobbyState.memberNames.clear();

        SteamAPICall_t call = SteamMatchmaking()->CreateLobby(k_ELobbyTypeInvisible, 10);
        m_createLobbyResult.Set(call, this, &SteamLobbyController::OnLobbyCreated);
    }

    void JoinLobbyByCode(const string &rawCode) {
        if (!SteamAPI_IsSteamRunning()) {
            Fail("Steam is not running.");
            return;
        }

        string code = NormalizeLobbyCode(rawCode);
        if (code.size() != 6) {
            Fail("Enter a 6-letter code.");
            return;
        }

        lobbyState.mode = LobbyState::Mode::Searching;
        lobbyState.statusMessage = TextFormat("Searching for lobby %s...", code.c_str());
        lobbyState.isHost = false;
        lobbyState.activeCode.clear();
        lobbyState.memberNames.clear();

        m_pendingJoinCode = code;
        SteamMatchmaking()->AddRequestLobbyListStringFilter(kLobbyCodeKey, code.c_str(), k_ELobbyComparisonEqual);
        SteamMatchmaking()->AddRequestLobbyListDistanceFilter(k_ELobbyDistanceFilterWorldwide);

        SteamAPICall_t call = SteamMatchmaking()->RequestLobbyList();
        m_lobbyMatchListResult.Set(call, this, &SteamLobbyController::OnLobbyMatchList);
    }

    void LeaveLobby() {
        if (lobbyState.lobbyId.IsValid()) {
            SteamMatchmaking()->LeaveLobby(lobbyState.lobbyId);
        }

        ResetToMenu("Left lobby.");
    }

    void HandleLobbyFrame() {
        if (sceneMode == SceneMode::Lobby && lobbyState.lobbyId.IsValid()) {
            RefreshMembers();
        }
    }

private:
    SceneMode &sceneMode;
    LobbyState &lobbyState;
    string m_pendingJoinCode;
    CCallResult<SteamLobbyController, LobbyCreated_t> m_createLobbyResult;
    CCallResult<SteamLobbyController, LobbyMatchList_t> m_lobbyMatchListResult;
    CCallResult<SteamLobbyController, LobbyChatMsg_t> m_chatResult; // unused but reserve
    STEAM_CALLBACK(SteamLobbyController, OnLobbyEnter, LobbyEnter_t);
    STEAM_CALLBACK(SteamLobbyController, OnLobbyDataUpdate, LobbyDataUpdate_t);
    STEAM_CALLBACK(SteamLobbyController, OnLobbyChatUpdate, LobbyChatUpdate_t);
    STEAM_CALLBACK(SteamLobbyController, OnLobbyChatMessage, LobbyChatMsg_t);
    STEAM_CALLBACK(SteamLobbyController, OnLobbyKicked, LobbyKicked_t);

    void ResetToMenu(const string &message) {
        lobbyState = LobbyState{};
        lobbyState.statusMessage = message;
        m_pendingJoinCode.clear();
        sceneMode = SceneMode::Menu;
    }

    void Fail(const string &message) {
        lobbyState.mode = LobbyState::Mode::Failed;
        lobbyState.statusMessage = message;
    }

    void RefreshMembers() {
        if (!lobbyState.lobbyId.IsValid()) {
            return;
        }

        lobbyState.memberNames.clear();
        lobbyState.memberIds.clear();
        int memberCount = SteamMatchmaking()->GetNumLobbyMembers(lobbyState.lobbyId);
        lobbyState.memberNames.reserve(memberCount);

        for (int index = 0; index < memberCount; ++index) {
            CSteamID memberId = SteamMatchmaking()->GetLobbyMemberByIndex(lobbyState.lobbyId, index);
            lobbyState.memberNames.emplace_back(SteamFriends()->GetFriendPersonaName(memberId));
            lobbyState.memberIds.emplace_back(memberId.ConvertToUint64());
        }
    }

public:
    static uint64_t NowMs() {
        using namespace chrono;
        return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
    }

    void SendLobbyText(const string &text) {
        if (!lobbyState.lobbyId.IsValid()) return;
        SteamMatchmaking()->SendLobbyChatMsg(lobbyState.lobbyId, text.c_str(), (int)text.size() + 1);
    }

    void SendPing() {
        if (!lobbyState.lobbyId.IsValid()) return;
        uint64_t ts = NowMs();
        string msg = TextFormat("PING:%llu", (unsigned long long)ts);
        SendLobbyText(msg);
    }

    void FinalizeLobbyEntry(CSteamID lobbyId) {
        lobbyState.lobbyId = lobbyId;
        lobbyState.isHost = SteamMatchmaking()->GetLobbyOwner(lobbyId) == SteamUser()->GetSteamID();
        lobbyState.mode = LobbyState::Mode::InLobby;
        lobbyState.statusMessage = lobbyState.isHost ? "Hosting lobby." : "Joined lobby.";

        if (lobbyState.isHost) {
            SteamMatchmaking()->SetLobbyType(lobbyId, k_ELobbyTypeInvisible);
            SteamMatchmaking()->SetLobbyJoinable(lobbyId, true);
            if (lobbyState.activeCode.empty()) {
                lobbyState.activeCode = GenerateLobbyCode();
            }
            SteamMatchmaking()->SetLobbyData(lobbyId, kLobbyCodeKey, lobbyState.activeCode.c_str());
        } else {
            const char *code = SteamMatchmaking()->GetLobbyData(lobbyId, kLobbyCodeKey);
            lobbyState.activeCode = code ? code : "";
            if (lobbyState.activeCode.empty()) {
                lobbyState.activeCode = m_pendingJoinCode;
            }
        }

        m_pendingJoinCode.clear();
        sceneMode = SceneMode::Lobby;
        RefreshMembers();
    }

    void OnLobbyCreated(LobbyCreated_t *pParam, bool bIOFailure) {
        if (bIOFailure || pParam->m_eResult != k_EResultOK) {
            Fail("Failed to create lobby.");
            return;
        }

        CSteamID lobbyId;
        lobbyId.SetFromUint64(pParam->m_ulSteamIDLobby);
        lobbyState.lobbyId = lobbyId;
        lobbyState.statusMessage = "Lobby created. Waiting to enter...";
        RefreshMembers();
    }

    void OnLobbyMatchList(LobbyMatchList_t *pParam, bool bIOFailure) {
        if (bIOFailure) {
            Fail("Steam could not search for lobbies.");
            return;
        }

        if (pParam->m_nLobbiesMatching == 0) {
            Fail(TextFormat("No lobby matched code %s.", m_pendingJoinCode.c_str()));
            return;
        }

        CSteamID lobbyId = SteamMatchmaking()->GetLobbyByIndex(0);
        lobbyState.mode = LobbyState::Mode::Joining;
        lobbyState.statusMessage = TextFormat("Joining lobby %s...", m_pendingJoinCode.c_str());
        SteamMatchmaking()->JoinLobby(lobbyId);
    }

};

inline void SteamLobbyController::OnLobbyEnter(LobbyEnter_t *pParam) {
    CSteamID lobbyId;
    lobbyId.SetFromUint64(pParam->m_ulSteamIDLobby);

    if (pParam->m_EChatRoomEnterResponse != k_EChatRoomEnterResponseSuccess) {
        Fail("Failed to enter lobby.");
        return;
    }

    FinalizeLobbyEntry(lobbyId);
}

inline void SteamLobbyController::OnLobbyDataUpdate(LobbyDataUpdate_t *pParam) {
    if (!lobbyState.lobbyId.IsValid() || pParam->m_ulSteamIDLobby != lobbyState.lobbyId.ConvertToUint64()) {
        return;
    }

    if (lobbyState.isHost) {
        const char *code = SteamMatchmaking()->GetLobbyData(lobbyState.lobbyId, kLobbyCodeKey);
        if (code && *code) {
            lobbyState.activeCode = code;
        }
    }

    RefreshMembers();
}

inline void SteamLobbyController::OnLobbyChatUpdate(LobbyChatUpdate_t *pParam) {
    if (!lobbyState.lobbyId.IsValid() || pParam->m_ulSteamIDLobby != lobbyState.lobbyId.ConvertToUint64()) {
        return;
    }

    RefreshMembers();
}

inline void SteamLobbyController::OnLobbyChatMessage(LobbyChatMsg_t *pParam) {
    if (!lobbyState.lobbyId.IsValid() || pParam->m_ulSteamIDLobby != lobbyState.lobbyId.ConvertToUint64()) return;

    CSteamID userId;
    char buffer[1024];
    int entryType;
    int bytes = SteamMatchmaking()->GetLobbyChatEntry(lobbyState.lobbyId, pParam->m_iChatID, &userId, buffer, sizeof(buffer), &entryType);
    if (bytes <= 0) return;

    string msg(buffer, bytes);
    string sender = SteamFriends()->GetFriendPersonaName(userId);

    // Handle ping protocol
    if (msg.rfind("PING:", 0) == 0) {
        // someone pinged; reply with PONG:<ts>
        if (userId != SteamUser()->GetSteamID()) {
            // extract timestamp
            string tsstr = msg.substr(5);
            string pong = TextFormat("PONG:%s", tsstr.c_str());
            SendLobbyText(pong);
        }
        return;
    }

    if (msg.rfind("PONG:", 0) == 0) {
        string tsstr = msg.substr(5);
        uint64_t sentTs = 0;
        try { sentTs = stoull(tsstr); } catch (...) { sentTs = 0; }
        if (sentTs != 0) {
            uint64_t now = NowMs();
            uint32_t rtt = static_cast<uint32_t>(now - sentTs);
            lobbyState.pings[userId.ConvertToUint64()] = rtt;
            lobbyState.statusMessage = TextFormat("Ping %s: %ums", sender.c_str(), rtt);
        }
        return;
    }

    // Normal chat message
    lobbyState.chatMessages.push_back({sender, msg, NowMs()});
    // auto-scroll to bottom when new message arrives
    lobbyState.chatScroll = 0;
    // keep recent messages bounded
    if (lobbyState.chatMessages.size() > 200) lobbyState.chatMessages.erase(lobbyState.chatMessages.begin());
}

inline void SteamLobbyController::OnLobbyKicked(LobbyKicked_t *pParam) {
    if (!lobbyState.lobbyId.IsValid() || pParam->m_ulSteamIDLobby != lobbyState.lobbyId.ConvertToUint64()) {
        return;
    }

    ResetToMenu("You were removed from the lobby.");
}

static void DrawScreenModeSelector(Font bodyFont, ScreenModePreset &screenModePreset, int &resolutionIndex, float x, float y, float width) {
    Rectangle holder = {x, y, width, 256.0f};
    DrawRectangleRounded(holder, 0.16f, 10, Color{24, 24, 24, 255});
    DrawRectangleRoundedLines(holder, 0.16f, 10, Color{70, 70, 70, 255});

    DrawTextEx(bodyFont, "Display", {x + 24.0f, y + 16.0f}, 28.0f, 1.0f, WHITE);
    DrawTextEx(bodyFont, "Keep window mode and resolution functional.", {x + 24.0f, y + 44.0f}, 18.0f, 1.0f, LIGHTGRAY);

    const float buttonGap = 12.0f;
    const float modeButtonWidth = (width - 24.0f * 2.0f - buttonGap * 2.0f) / 3.0f;
    const float modeButtonY = y + 72.0f;
    const char *modeLabels[] = {"Maximized", "Windowed", "Fullscreen"};
    ScreenModePreset modes[] = {ScreenModePreset::MaximizedWindowed, ScreenModePreset::Windowed, ScreenModePreset::Fullscreen};

    for (int index = 0; index < 3; ++index) {
        Rectangle buttonRect = {x + 24.0f + index * (modeButtonWidth + buttonGap), modeButtonY, modeButtonWidth, 40.0f};
        bool isActive = screenModePreset == modes[index];
        Button optionButton(
            buttonRect.x,
            buttonRect.y,
            isActive ? WHITE : Color{40, 40, 40, 255},
            isActive ? BLACK : WHITE,
            18.0f,
            buttonRect.width,
            buttonRect.height,
            modeLabels[index],
            bodyFont,
            "center",
            1.0f,
            WHITE,
            isActive ? 2 : 1,
            0.2f,
            4.0f
        );

        if (MouseOver(buttonRect) && !isActive) {
            optionButton.backgroundColor = Color{62, 62, 62, 255};
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && MouseOver(buttonRect)) {
            screenModePreset = modes[index];
            ApplyScreenMode(screenModePreset, kResolutionPresets[resolutionIndex]);
        }

        optionButton.Draw();
    }

    DrawTextEx(bodyFont, "Resolution", {x + 24.0f, y + 126.0f}, 28.0f, 1.0f, WHITE);
    DrawTextEx(bodyFont, "Applies immediately in windowed modes.", {x + 24.0f, y + 154.0f}, 18.0f, 1.0f, LIGHTGRAY);

    const float resolutionButtonWidth = (width - 24.0f * 2.0f - buttonGap * 2.0f) / 3.0f;
    const float resolutionButtonY = y + 182.0f;

    for (int index = 0; index < static_cast<int>(kResolutionPresets.size()); ++index) {
        Rectangle buttonRect = {x + 24.0f + index * (resolutionButtonWidth + buttonGap), resolutionButtonY, resolutionButtonWidth, 40.0f};
        bool isActive = resolutionIndex == index;
        Button optionButton(
            buttonRect.x,
            buttonRect.y,
            isActive ? WHITE : Color{40, 40, 40, 255},
            isActive ? BLACK : WHITE,
            18.0f,
            buttonRect.width,
            buttonRect.height,
            kResolutionPresets[index].label,
            bodyFont,
            "center",
            1.0f,
            WHITE,
            isActive ? 2 : 1,
            0.2f,
            4.0f
        );

        if (MouseOver(buttonRect) && !isActive) {
            optionButton.backgroundColor = Color{62, 62, 62, 255};
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && MouseOver(buttonRect)) {
            resolutionIndex = index;
            ApplyScreenMode(screenModePreset, kResolutionPresets[resolutionIndex]);
        }

        optionButton.Draw();
    }
}

static void DrawMenuScene(Font titleFont, Font bodyFont, SceneMode &sceneMode, LobbyState &lobbyState, SteamLobbyController &lobbyController, const string &playerName, bool steamInitialized) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();

    Header header;

    Button hostButton(
        screenW / 2.0f - buttonDefaultWidth / 2.0f,
        560.0f,
        initButtonBgColor,
        initButtonTextColor,
        44.0f,
        buttonDefaultWidth,
        72.0f,
        "Host Lobby",
        bodyFont,
        "center",
        1.0f,
        WHITE,
        2,
        0.28f,
        10.0f
    );

    Button joinButton(
        screenW / 2.0f - buttonDefaultWidth / 2.0f,
        670.0f,
        initButtonBgColor,
        initButtonTextColor,
        44.0f,
        buttonDefaultWidth,
        72.0f,
        "Join Lobby",
        bodyFont,
        "center",
        1.0f,
        WHITE,
        2,
        0.28f,
        10.0f
    );

    Button settingsButton(
        screenW / 2.0f - buttonDefaultWidth / 2.0f,
        780.0f,
        initButtonBgColor,
        initButtonTextColor,
        44.0f,
        buttonDefaultWidth,
        72.0f,
        "Settings",
        bodyFont,
        "center",
        1.0f,
        WHITE,
        2,
        0.28f,
        10.0f
    );

    if (MouseOver(hostButton.rect)) {
        hostButton.backgroundColor = WHITE;
        hostButton.textColor = BLACK;
    }

    if (MouseOver(joinButton.rect)) {
        joinButton.backgroundColor = WHITE;
        joinButton.textColor = BLACK;
    }

    if (MouseOver(settingsButton.rect)) {
        settingsButton.backgroundColor = WHITE;
        settingsButton.textColor = BLACK;
    }

    Rectangle joinCodeBox = {screenW / 2.0f - 150.0f, 470.0f, 300.0f, 52.0f};
    DrawRectangleRounded(joinCodeBox, 0.18f, 8, Color{22, 22, 22, 255});
    DrawRectangleRoundedLines(joinCodeBox, 0.18f, 8, Color{70, 70, 70, 255});
    DrawTextEx(bodyFont, "Join code", {joinCodeBox.x, joinCodeBox.y - 28.0f}, 22.0f, 1.0f, LIGHTGRAY);

    bool codeBoxHovered = MouseOver(joinCodeBox);
    if (codeBoxHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // Nothing else to track yet; the join box stays active while the user types.
    }

    if (codeBoxHovered || !lobbyState.joinCodeInput.empty()) {
        while (int key = GetCharPressed()) {
            if (isalpha(key) && lobbyState.joinCodeInput.size() < 6) {
                lobbyState.joinCodeInput.push_back(static_cast<char>(toupper(key)));
            }
        }

        if (IsKeyPressed(KEY_BACKSPACE) && !lobbyState.joinCodeInput.empty()) {
            lobbyState.joinCodeInput.pop_back();
        }
    }

    if (lobbyState.joinCodeInput.size() > 6) {
        lobbyState.joinCodeInput.resize(6);
    }

    string shownCode = lobbyState.joinCodeInput;
    while (shownCode.size() < 6) {
        shownCode.push_back('_');
    }
    DrawTextEx(bodyFont, shownCode.c_str(), {joinCodeBox.x + 18.0f, joinCodeBox.y + 14.0f}, 28.0f, 1.0f, WHITE);

    if (steamInitialized && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (MouseOver(hostButton.rect)) {
            lobbyController.CreateLobby();
        } else if (MouseOver(joinButton.rect)) {
            lobbyController.JoinLobbyByCode(lobbyState.joinCodeInput);
        } else if (MouseOver(settingsButton.rect)) {
            sceneMode = SceneMode::Settings;
        }
    }

    if (!steamInitialized) {
        DrawTextEx(bodyFont, "Steam is required for lobby play.", {screenW / 2.0f - 150.0f, 870.0f}, 20.0f, 1.0f, ORANGE);
    }

    header.Draw(titleFont, "@", "v0.1 (Alpha)");
    hostButton.Draw();
    joinButton.Draw();
    settingsButton.Draw();

    DrawTextEx(bodyFont, lobbyState.statusMessage.empty() ? "Create a lobby or join with a 6-letter code." : lobbyState.statusMessage.c_str(), {screenW / 2.0f - 320.0f, 920.0f}, 22.0f, 1.0f, LIGHTGRAY);
    DrawHud(playerName, steamInitialized);

    (void)screenH;
}

static void DrawSettingsScene(Font titleFont, Font bodyFont, SceneMode &sceneMode, ScreenModePreset &screenModePreset, int &resolutionIndex, const string &playerName, bool steamInitialized) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    float margin = 72.0f;

    Button backButton(
        screenW - 220.0f,
        50.0f,
        Color{40, 40, 40, 255},
        WHITE,
        32.0f,
        160.0f,
        56.0f,
        "Back",
        bodyFont,
        "center",
        1.0f,
        WHITE,
        2,
        0.25f,
        6.0f
    );

    if (MouseOver(backButton.rect)) {
        backButton.backgroundColor = WHITE;
        backButton.textColor = BLACK;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && MouseOver(backButton.rect)) {
        sceneMode = SceneMode::Menu;
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        sceneMode = SceneMode::Menu;
    }

    DrawTextEx(titleFont, "Settings", {margin, 42.0f}, 72.0f, 2.0f, WHITE);
    DrawTextEx(bodyFont, "Only the display controls stay here.", {margin, 108.0f}, 24.0f, 1.0f, LIGHTGRAY);
    backButton.Draw();

    float panelWidth = screenW - margin * 2.0f;
    DrawScreenModeSelector(bodyFont, screenModePreset, resolutionIndex, margin, 150.0f, panelWidth);

    DrawTextEx(bodyFont, TextFormat("Active mode: %s", GetScreenModeLabel(screenModePreset)), {margin + 8.0f, 440.0f}, 20.0f, 1.0f, LIGHTGRAY);
    DrawTextEx(bodyFont, TextFormat("Active resolution: %s", GetResolutionLabel(resolutionIndex)), {margin + 8.0f, 470.0f}, 20.0f, 1.0f, LIGHTGRAY);

    DrawHud(playerName, steamInitialized);

    (void)screenH;
}

static void DrawLobbyScene(Font titleFont, Font bodyFont, SceneMode &sceneMode, LobbyState &lobbyState, SteamLobbyController &lobbyController, const string &playerName, bool steamInitialized) {
    int screenW = GetScreenWidth();
    int screenH = GetScreenHeight();
    float margin = 72.0f;

    Button leaveButton(
        screenW - 220.0f,
        50.0f,
        Color{120, 40, 40, 255},
        WHITE,
        32.0f,
        160.0f,
        56.0f,
        "Leave",
        bodyFont,
        "center",
        1.0f,
        WHITE,
        2,
        0.25f,
        6.0f
    );

    if (MouseOver(leaveButton.rect)) {
        leaveButton.backgroundColor = Color{170, 55, 55, 255};
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && MouseOver(leaveButton.rect)) {
        lobbyController.LeaveLobby();
    }

    DrawTextEx(titleFont, "Lobby", {margin, 42.0f}, 72.0f, 2.0f, WHITE);
    DrawTextEx(bodyFont, lobbyState.isHost ? "You are hosting." : "You joined a lobby.", {margin, 108.0f}, 24.0f, 1.0f, LIGHTGRAY);
    DrawTextEx(bodyFont, TextFormat("Code: %s", lobbyState.activeCode.empty() ? "------" : lobbyState.activeCode.c_str()), {margin, 150.0f}, 36.0f, 1.0f, WHITE);
    DrawTextEx(bodyFont, lobbyState.statusMessage.c_str(), {margin, 192.0f}, 22.0f, 1.0f, LIGHTGRAY);

    Rectangle memberPanel = {margin, 248.0f, screenW - margin * 2.0f, screenH - 360.0f};
    DrawRectangleRounded(memberPanel, 0.03f, 10, Color{16, 16, 16, 255});
    DrawRectangleRoundedLines(memberPanel, 0.03f, 10, Color{60, 60, 60, 255});
    DrawTextEx(bodyFont, "Members", {memberPanel.x + 24.0f, memberPanel.y + 20.0f}, 28.0f, 1.0f, WHITE);

    float memberY = memberPanel.y + 70.0f;
    if (lobbyState.memberNames.empty()) {
        DrawTextEx(bodyFont, "Waiting for members...", {memberPanel.x + 24.0f, memberY}, 22.0f, 1.0f, LIGHTGRAY);
    } else {
        for (size_t index = 0; index < lobbyState.memberNames.size(); ++index) {
            uint64_t id = lobbyState.memberIds.size() > index ? lobbyState.memberIds[index] : 0;
            uint32_t ping = 0;
            auto it = lobbyState.pings.find(id);
            if (it != lobbyState.pings.end()) ping = it->second;

            DrawTextEx(bodyFont, TextFormat("%zu. %s  (%ums)", index + 1, lobbyState.memberNames[index].c_str(), ping), {memberPanel.x + 24.0f, memberY + static_cast<float>(index) * 36.0f}, 22.0f, 1.0f, WHITE);
        }
    }

    leaveButton.Draw();
    // Chat area
    Rectangle chatBox = {margin, screenH - 220.0f, screenW - margin * 2.0f - 200.0f, 160.0f};
    DrawRectangleRounded(chatBox, 0.03f, 10, Color{18,18,18,255});
    DrawRectangleRoundedLines(chatBox, 0.03f, 10, Color{60,60,60,255});

    float chatY = chatBox.y + 12.0f;
    int visibleLines = 6;
    int total = (int)lobbyState.chatMessages.size();

    // mouse wheel scrolling when hovering chat box
    if (MouseOver(chatBox)) {
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            lobbyState.chatScroll = std::clamp(lobbyState.chatScroll - (int)wheel, 0, std::max(0, total - visibleLines));
        }
    }

    int start = 0;
    if (total > visibleLines) {
        start = total - visibleLines - lobbyState.chatScroll;
        if (start < 0) start = 0;
    }

    const Color nameColors[] = {SKYBLUE, GREEN, ORANGE, BLUE, PURPLE, YELLOW};
    for (int i = start; i < total && i < start + visibleLines; ++i) {
        auto &cm = lobbyState.chatMessages[i];
        uint64_t now = SteamLobbyController::NowMs();
        uint64_t ageS = (now > cm.ts) ? ((now - cm.ts) / 1000) : 0;
        string timeLabel;
        if (ageS < 60) timeLabel = TextFormat("%llus", (unsigned long long)ageS);
        else timeLabel = TextFormat("%llum", (unsigned long long)(ageS / 60));

        size_t h = std::hash<string>{}(cm.sender);
        Color nameColor = nameColors[h % (sizeof(nameColors)/sizeof(nameColors[0]))];

        DrawTextEx(bodyFont, TextFormat("[%s]", timeLabel.c_str()), {chatBox.x + 12.0f, chatY}, 16.0f, 1.0f, LIGHTGRAY);
        DrawTextEx(bodyFont, TextFormat("%s:", cm.sender.c_str()), {chatBox.x + 80.0f, chatY}, 18.0f, 1.0f, nameColor);
        DrawTextEx(bodyFont, cm.message.c_str(), {chatBox.x + 80.0f + MeasureTextEx(bodyFont, (cm.sender + ":").c_str(), 18.0f, 1.0f).x + 8.0f, chatY}, 18.0f, 1.0f, LIGHTGRAY);
        chatY += 24.0f;
    }

    Rectangle chatInputRect = {chatBox.x + chatBox.width + 16.0f, chatBox.y, 184.0f, 120.0f};
    DrawRectangleRounded(chatInputRect, 0.03f, 6, Color{22,22,22,255});
    DrawRectangleRoundedLines(chatInputRect, 0.03f, 6, Color{60,60,60,255});
    DrawTextEx(bodyFont, "Chat", {chatInputRect.x + 8.0f, chatInputRect.y + 8.0f}, 22.0f, 1.0f, WHITE);

    Rectangle chatTextField = {chatBox.x, chatBox.y + chatBox.height + 8.0f, chatBox.width, 44.0f};
    DrawRectangleRounded(chatTextField, 0.03f, 6, Color{24,24,24,255});
    DrawRectangleRoundedLines(chatTextField, 0.03f, 6, Color{70,70,70,255});
    DrawTextEx(bodyFont, lobbyState.chatInput.c_str(), {chatTextField.x + 8.0f, chatTextField.y + 8.0f}, 20.0f, 1.0f, WHITE);

    // Click to focus chat field
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (MouseOver(chatTextField)) lobbyState.chatFocused = true;
        else lobbyState.chatFocused = false;
    }

    // Chat typing only when focused
    if (lobbyState.chatFocused) {
        while (int key = GetCharPressed()) {
            if (key >= 32 && key < 127 && lobbyState.chatInput.size() < 256) {
                lobbyState.chatInput.push_back((char)key);
            }
        }
        if (IsKeyPressed(KEY_BACKSPACE) && !lobbyState.chatInput.empty()) lobbyState.chatInput.pop_back();
        // Send on Enter
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER)) {
            if (!lobbyState.chatInput.empty()) {
                string text = lobbyState.chatInput;
                if (text == "/ping") {
                    lobbyController.SendPing();
                } else {
                    lobbyController.SendLobbyText(text);
                }
                lobbyState.chatInput.clear();
                // auto-scroll to bottom when sending
                lobbyState.chatScroll = 0;
            }
        }
    }
    DrawHud(playerName, steamInitialized);
}

int main() {
    SetConfigFlags(FLAG_FULLSCREEN_MODE);
    InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "Among Us (Alpha)");
    SetTargetFPS(60);

    bool steamInitialized = SteamAPI_Init();
    if (!steamInitialized) {
        // silent fallback for stub
    }

    string playerName = steamInitialized ? SteamFriends()->GetFriendPersonaName(SteamUser()->GetSteamID()) : "Guest";

    Font font = Font{};
    Font settingsFont = Font{};

    SceneMode sceneMode = SceneMode::Menu;
    ScreenModePreset screenModePreset = ScreenModePreset::Fullscreen;
    int resolutionIndex = 0;
    LobbyState lobbyState;
    SteamLobbyController lobbyController(sceneMode, lobbyState);

    // single frame run in stub
    lobbyController.HandleLobbyFrame();

    // exit immediately for stub
    return 0;
}
