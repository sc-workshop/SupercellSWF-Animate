#!/bin/bash
set -e

GN_EXECUTABLE=$1
SKIA_ROOT=$2
SKIA_BUILD_DIR=$3
SKIA_ARGS=$4
BUILD_TYPE=$5
TARGET_CPU=$6

if [ "$TARGET_CPU" = "arm64" ]; then
    SKIA_ARGS="$SKIA_ARGS target_cpu=\"arm64\""
elif [ "$TARGET_CPU" = "x86_64h" ]; then
    SKIA_ARGS="$SKIA_ARGS target_cpu=\"x86_64\" extra_cflags=[\"-arch\",\"x86_64h\"]"
fi

echo "Running GN gen for $SKIA_ROOT..."
echo "Target CPU: $TARGET_CPU"

"$GN_EXECUTABLE" gen "$SKIA_BUILD_DIR" --root="$SKIA_ROOT" --args="$SKIA_ARGS"