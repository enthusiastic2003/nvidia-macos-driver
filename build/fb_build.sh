#!/bin/bash
set -euo pipefail

SRC=$1
OUT=$2
mkdir -p "$OUT/NVRMFB.kext/Contents/MacOS"

OGKM=${OGKM:-$HOME/ogkm610}
SDK=$(xcrun --show-sdk-path)
KHDR="$SDK/System/Library/Frameworks/Kernel.framework/Headers"
NV="$OGKM/src/nvidia"
KMS="$OGKM/src/nvidia-modeset"

RMINC=(
  -I"$NV/arch/nvalloc/unix/include" -I"$NV/arch/nvalloc/common/inc"
  -I"$NV/arch/nvalloc/common/inc/gsp" -I"$OGKM/src/common/sdk/nvidia/inc"
  -I"$OGKM/src/common/sdk/nvidia/inc/hw" -I"$OGKM/src/common/inc"
  -I"$OGKM/src/common/shared/inc" -I"$NV/inc" -I"$NV/inc/os"
  -I"$NV/inc/kernel" -I"$NV/kernel/inc" -I"$NV/interface"
)
KMSINC=(
  -I"$KMS/os-interface/include" -I"$KMS/kapi/interface" -I"$KMS/kapi/include"
  -I"$OGKM/src/common/unix/nvidia-push/interface" -I"$KMS/interface" -I"$KMS/include"
  -I"$OGKM/src/common/unix/common/inc" -I"$OGKM/src/common/modeset"
  -I"$OGKM/src/common/unix/common/utils/interface"
)
CFLAGS=(
  -arch x86_64 -fapple-kext -mkernel -nostdinc -I"$KHDR" -I"$SRC"
  "${RMINC[@]}" "${KMSINC[@]}"
  -DKERNEL -DKERNEL_PRIVATE -DDRIVER_PRIVATE -DAPPLE -DNeXT
  -std=c++17 -fno-rtti -fno-exceptions -fno-builtin -fno-common
  -Wall -Wno-unused-parameter -Wno-inconsistent-missing-override
  -Wno-unused-function -O2
)

xcrun clang++ "${CFLAGS[@]}" -I"$SRC/fb" -c "$SRC/fb/nvrm-fb.cpp" \
  -o "$OUT/nvrm-fb.o" 2>"$OUT/fb.err" || {
    grep -E 'error:|fatal error:' "$OUT/fb.err" | head -40
    exit 1
  }
xcrun clang++ -arch x86_64 -fapple-kext -nostdlib -Xlinker -kext \
  -lkmodc++ -lkmod -lcc_kext "$OUT/nvrm-fb.o" \
  -o "$OUT/NVRMFB.kext/Contents/MacOS/NVRMFB" 2>"$OUT/fbld.err" || {
    head -40 "$OUT/fbld.err"
    exit 1
  }
cp "$SRC/fb/Info.plist" "$OUT/NVRMFB.kext/Contents/Info.plist"
echo "NVRMFB $(md5 -q "$OUT/NVRMFB.kext/Contents/MacOS/NVRMFB") $(stat -f %z "$OUT/NVRMFB.kext/Contents/MacOS/NVRMFB") B"
