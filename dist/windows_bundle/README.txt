Windows bundle notes:
- This bundle contains only game resources and steam_appid.txt.
- You MUST add a Windows-built somethingcool.exe (matching architecture) and steam_api.dll.
- The checked-in bundle does not ship steam_api.dll, so the launcher will stop with a clear error until you copy the Steam redistributable next to the exe.
- If you want a build that does not depend on Steamworks at runtime, use the Windows stub entry point in src/scenes/game_menu/main_win_stub.cpp instead of the Steam-enabled entry point.
- To build on Windows:
  * Use MSVC or MinGW and link against raylib and the Steamworks import library.
  * Include the Steam redistributable steam_api.dll next to the .exe.
- To test locally on Windows, run Steam, log in, and run somethingcool.exe.
