#!/bin/bash
# Builds NMIntelFB.kext (the built-in Intel panel's framebuffer, kexts/NMIntelFB) on macOS.
#   bash build/intelfb_build.sh kexts/NMIntelFB /tmp/out
# NMIntelFB uses only Apple's kernel interfaces, so unlike NVRMFB it needs no NVIDIA sources.
set -euo pipefail

SRC=$1
OUT=$2
mkdir -p "$OUT/NMIntelFB.kext/Contents/MacOS"

SDK=$(xcrun --show-sdk-path)
KHDR="$SDK/System/Library/Frameworks/Kernel.framework/Headers"
CFLAGS=(
  -arch x86_64 -fapple-kext -mkernel -nostdinc -I"$KHDR" -I"$SRC"
  -DKERNEL -DKERNEL_PRIVATE -DDRIVER_PRIVATE -DAPPLE -DNeXT
  -std=c++17 -fno-rtti -fno-exceptions -fno-builtin -fno-common
  -Wall -Wno-unused-parameter -Wno-inconsistent-missing-override -Wno-unused-function -O2
)

xcrun clang++ "${CFLAGS[@]}" -c "$SRC/intelfb.cpp" -o "$OUT/intelfb.o" 2>"$OUT/intelfb.err" || {
    grep -E 'error:|fatal error:' "$OUT/intelfb.err" | head -40
    exit 1
}
xcrun clang++ -arch x86_64 -fapple-kext -nostdlib -Xlinker -kext \
  -lkmodc++ -lkmod -lcc_kext "$OUT/intelfb.o" \
  -o "$OUT/NMIntelFB.kext/Contents/MacOS/NMIntelFB" 2>"$OUT/intelfb-ld.err" || {
    head -40 "$OUT/intelfb-ld.err"
    exit 1
}
cp "$SRC/Info.plist" "$OUT/NMIntelFB.kext/Contents/Info.plist"
echo "NMIntelFB $(md5 -q "$OUT/NMIntelFB.kext/Contents/MacOS/NMIntelFB") $(stat -f %z "$OUT/NMIntelFB.kext/Contents/MacOS/NMIntelFB") B"
