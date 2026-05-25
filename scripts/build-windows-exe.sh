#!/usr/bin/env bash
set -euo pipefail

echo "Windows cross-build requires a MinGW toolchain plus Windows raylib/ENet libraries."
echo "Set up a Windows SDK/toolchain, then compile main.cpp and enet.cpp into main.exe."
exit 1
