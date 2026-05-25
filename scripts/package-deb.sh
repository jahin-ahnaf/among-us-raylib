#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DIST_DIR="$ROOT_DIR/dist/deb"
PKG_ROOT="$DIST_DIR/pkg"
APP_NAME="somethingcool"
APP_BIN="$APP_NAME"
VERSION="1.0"

cd "$ROOT_DIR"
./build.sh

rm -rf "$DIST_DIR"
mkdir -p "$PKG_ROOT/DEBIAN" \
         "$PKG_ROOT/usr/bin" \
         "$PKG_ROOT/usr/lib/$APP_NAME" \
         "$PKG_ROOT/usr/share/applications" \
         "$PKG_ROOT/usr/share/icons/hicolor/256x256/apps"

install -m 755 main "$PKG_ROOT/usr/lib/$APP_NAME/$APP_BIN"
cp -r resources "$PKG_ROOT/usr/lib/$APP_NAME/"

cat > "$PKG_ROOT/usr/bin/$APP_BIN" <<EOF
#!/usr/bin/env bash
exec /usr/lib/$APP_NAME/$APP_BIN "\$@"
EOF
chmod 755 "$PKG_ROOT/usr/bin/$APP_BIN"

cat > "$PKG_ROOT/usr/share/applications/$APP_NAME.desktop" <<EOF
[Desktop Entry]
Type=Application
Name=Something Cool
Exec=$APP_BIN
Icon=$APP_NAME
Categories=Game;
Terminal=false
EOF

cat > "$PKG_ROOT/DEBIAN/control" <<EOF
Package: $APP_NAME
Version: $VERSION
Section: games
Priority: optional
Architecture: amd64
Maintainer: Copilot CLI
Description: Raylib/ENet room-based multiplayer game
EOF

dpkg-deb --root-owner-group --build "$PKG_ROOT" "$DIST_DIR/${APP_NAME}_${VERSION}_amd64.deb"
echo "Created $DIST_DIR/${APP_NAME}_${VERSION}_amd64.deb"
