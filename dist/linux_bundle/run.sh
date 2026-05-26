#!/bin/sh
# Run from the bundle root. Ensure Steam is running.
DIR="$(dirname "$0")"
cd "$DIR"
export LD_LIBRARY_PATH="$PWD"
./somethingcool
