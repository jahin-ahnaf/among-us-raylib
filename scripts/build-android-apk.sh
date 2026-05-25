#!/usr/bin/env bash
set -euo pipefail

cat <<'EOF'
Android APK packaging needs the raylib Android project template plus the Android SDK/NDK.

Quick setup:
1. Install Android Studio.
2. Install SDK Platform, Build-Tools, and NDK from SDK Manager.
3. Clone the raylib Android template project.
4. Copy this game's source into the template's native C++ folder.
5. Keep the touch controls enabled and build the template from Android Studio.

The game code already supports fullscreen, joystick input, and room UI on mobile.

If you want, I can wire this repo into a specific raylib Android template next.
EOF
exit 1
