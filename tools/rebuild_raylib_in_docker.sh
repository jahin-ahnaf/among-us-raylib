#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUTDIR="$ROOT/build-output"
PKG_LIB_DIR="$ROOT/package/opt/somethingcool/lib"

mkdir -p "$OUTDIR" "$PKG_LIB_DIR"

build_in_container(){
  local image="$1"; shift
  local tag_name="$1"; shift
  echo "Building raylib in container $image (tag=$tag_name)"

  docker run --rm -v "$ROOT":/work -w /work -e DEBIAN_FRONTEND=noninteractive "$image" /bin/bash -ex -c "\
    apt-get update && apt-get install -y --no-install-recommends \
      build-essential git cmake pkg-config libx11-dev libxrandr-dev libxi-dev libxcursor-dev libxinerama-dev libxfixes-dev libgl1-mesa-dev libopenal-dev libasound2-dev libsndfile1-dev libpulse-dev libxcb1-dev ca-certificates libglu1-mesa-dev && \
    rm -rf /var/lib/apt/lists/* && \
    git clone --depth 1 https://github.com/raysan5/raylib.git raylib-src && \
    mkdir -p raylib-src/build && cd raylib-src/build && \
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local .. && \
    make -j\"$(nproc)\" && \
    make install DESTDIR=/work/build-output/$tag_name && \
    if [ -f /work/build-output/$tag_name/usr/local/lib/libraylib.so.6.0.0 ]; then \
      cp /work/build-output/$tag_name/usr/local/lib/libraylib.so.6.0.0 /work/package/opt/somethingcool/lib/libraylib.$tag_name.so.600; \
      echo "copied libraylib for $tag_name"; \
    else \
      echo "libraylib not found after install"; exit 1; \
    fi && \
    # Build the somethingcool executable inside the container so it links against the installed raylib
    mkdir -p /work/package/opt/somethingcool && \
    cp -a /work/resources /work/package/opt/somethingcool/ || true && \
    CXX=g++ && \
    SRCS=\"$(find /work/src -name '*.cpp' -print | tr '\n' ' ')\" && \
    echo "Compiling sources: $SRCS" && \
    $CXX -std=c++17 -O2 -g $SRCS -I/work/include -L/usr/local/lib -o /work/package/opt/somethingcool/somethingcool -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -Wl,-rpath,/work/package/opt/somethingcool/lib && \
    strip /work/package/opt/somethingcool/somethingcool || true"
}

build_in_container "ubuntu:20.04" "ubuntu20.04"
build_in_container "kalilinux/kali-rolling" "kali-rolling"

echo "Built libraries are in: $PKG_LIB_DIR"
ls -l "$PKG_LIB_DIR"

echo "To use a rebuilt library, copy it to package/opt/somethingcool/lib/libraylib.so.600 (backup existing first)."
