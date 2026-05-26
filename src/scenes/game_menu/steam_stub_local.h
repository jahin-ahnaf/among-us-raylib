// Minimal local Steam stub for Windows demo builds
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>

using uint64 = unsigned long long;

struct CSteamID {
    uint64 id = 0;
    CSteamID() = default;
    void SetFromUint64(uint64 v) { id = v; }
    bool IsValid() const { return id != 0; }
    uint64 ConvertToUint64() const { return id; }
    bool operator==(const CSteamID &o) const { return id == o.id; }
};

// Minimal callback structs (only fields used by main.cpp)
struct LobbyCreated_t { unsigned long long m_ulSteamIDLobby; int m_eResult; };
struct LobbyMatchList_t { int m_nLobbiesMatching; };
struct LobbyEnter_t { unsigned long long m_ulSteamIDLobby; int m_EChatRoomEnterResponse; };
struct LobbyDataUpdate_t { unsigned long long m_ulSteamIDLobby; };
struct LobbyChatUpdate_t { unsigned long long m_ulSteamIDLobby; };
struct LobbyChatMsg_t { unsigned long long m_ulSteamIDLobby; int m_iChatID; };
struct LobbyKicked_t { unsigned long long m_ulSteamIDLobby; };

enum { k_EResultOK = 1 };
enum { k_EChatRoomEnterResponseSuccess = 1 };

using SteamAPICall_t = uint64;

// Minimal CCallResult template stub used in main.cpp
template <class T, class P>
struct CCallResult {
    void Set(SteamAPICall_t, T*, void (T::*)(P*, bool)) {}
    bool IsActive() const { return false; }
    void Cancel() {}
};

#define STEAM_CALLBACK(classname, func, param) void func(param*)

// Minimal matchmaking/friends/user stubs
class MatchmakingStub {
public:
    struct Lobby { uint64 id; std::string code; std::vector<uint64> members; std::vector<std::string> chat; std::vector<int> chatIds; };
    std::mutex mu;
    std::unordered_map<uint64, Lobby> lobbies;
    std::vector<std::string> pendingFilters;
    uint64 nextLobby = 1001;

    MatchmakingStub() {}

    SteamAPICall_t CreateLobby(int /*type*/, int /*maxMembers*/) {
        std::lock_guard<std::mutex> lk(mu);
        uint64 id = nextLobby++;
        Lobby L; L.id = id; L.members.push_back(1); // local user
        lobbies[id] = L;
        return id;
    }

    void AddRequestLobbyListStringFilter(const char*, const char *value, int) {
        pendingFilters.clear();
        if (value) pendingFilters.push_back(std::string(value));
    }

    void AddRequestLobbyListDistanceFilter(int) {}

    SteamAPICall_t RequestLobbyList() {
        // synchronous stub: will populate match list count in caller via GetLobbyByIndex
        std::lock_guard<std::mutex> lk(mu);
        int found = 0;
        for (auto &kv : lobbies) {
            if (pendingFilters.empty() || lobbies[kv.first].code == pendingFilters[0]) found++;
        }
        // encode result count into return value
        return static_cast<SteamAPICall_t>(found);
    }

    CSteamID GetLobbyByIndex(int index) {
        std::lock_guard<std::mutex> lk(mu);
        int i = 0;
        for (auto &kv : lobbies) {
            if (i == index) { CSteamID id; id.SetFromUint64(kv.first); return id; }
            ++i;
        }
        return CSteamID();
    }

    SteamAPICall_t JoinLobby(CSteamID id) {
        std::lock_guard<std::mutex> lk(mu);
        auto it = lobbies.find(id.ConvertToUint64());
        if (it != lobbies.end()) {
            it->second.members.push_back(1);
            return id.ConvertToUint64();
        }
        return 0;
    }

    void LeaveLobby(CSteamID id) {
        std::lock_guard<std::mutex> lk(mu);
        auto it = lobbies.find(id.ConvertToUint64());
        if (it != lobbies.end()) {
            auto &m = it->second.members;
            m.erase(std::remove(m.begin(), m.end(), 1), m.end());
        }
    }

    int GetNumLobbyMembers(CSteamID id) {
        std::lock_guard<std::mutex> lk(mu);
        auto it = lobbies.find(id.ConvertToUint64());
        if (it == lobbies.end()) return 0;
        return (int)it->second.members.size();
    }

    CSteamID GetLobbyMemberByIndex(CSteamID id, int idx) {
        std::lock_guard<std::mutex> lk(mu);
        auto it = lobbies.find(id.ConvertToUint64());
        if (it == lobbies.end()) return CSteamID();
        if (idx < 0 || idx >= (int)it->second.members.size()) return CSteamID();
        CSteamID r; r.SetFromUint64(it->second.members[idx]); return r;
    }

    const char *GetLobbyData(CSteamID id, const char *key) {
        std::lock_guard<std::mutex> lk(mu);
        auto it = lobbies.find(id.ConvertToUint64());
        if (it == lobbies.end()) return "";
        if (!it->second.code.empty()) return it->second.code.c_str();
        return "";
    }

    bool SetLobbyData(CSteamID id, const char *key, const char *value) {
        std::lock_guard<std::mutex> lk(mu);
        auto it = lobbies.find(id.ConvertToUint64());
        if (it == lobbies.end()) return false;
        if (value) it->second.code = std::string(value);
        return true;
    }

    bool SendLobbyChatMsg(CSteamID id, const void *pvMsgBody, int cubMsgBody) {
        std::lock_guard<std::mutex> lk(mu);
        auto it = lobbies.find(id.ConvertToUint64());
        if (it == lobbies.end()) return false;
        std::string s((const char*)pvMsgBody, cubMsgBody>0?cubMsgBody:0);
        it->second.chat.push_back(s);
        return true;
    }

    int GetLobbyChatEntry(CSteamID id, int chatIndex, CSteamID *pSteamIDUser, void *pvData, int cubData, int *peChatEntryType) {
        std::lock_guard<std::mutex> lk(mu);
        auto it = lobbies.find(id.ConvertToUint64());
        if (it == lobbies.end()) return 0;
        if (chatIndex < 0 || chatIndex >= (int)it->second.chat.size()) return 0;
        std::string &msg = it->second.chat[chatIndex];
        if (pSteamIDUser) pSteamIDUser->SetFromUint64(1);
        int copy = std::min(cubData-1, (int)msg.size());
        if (pvData && copy>0) memcpy(pvData, msg.data(), copy);
        if (pvData && cubData>0) ((char*)pvData)[copy]=0;
        return copy;
    }

};

class FriendsStub {
public:
    std::string GetFriendPersonaName(CSteamID id) { return "Player" + std::to_string(id.ConvertToUint64()); }
};

class UserStub { public: CSteamID GetSteamID() { CSteamID s; s.SetFromUint64(1); return s; } };

// singletons
static MatchmakingStub s_matchmaking;
static FriendsStub s_friends;
static UserStub s_user;

inline bool SteamAPI_Init() { return true; }
inline void SteamAPI_Shutdown() {}
inline void SteamAPI_RunCallbacks() {}
inline bool SteamAPI_IsSteamRunning() { return true; }

inline MatchmakingStub* SteamMatchmaking() { return &s_matchmaking; }
inline FriendsStub* SteamFriends() { return &s_friends; }
inline UserStub* SteamUser() { return &s_user; }
