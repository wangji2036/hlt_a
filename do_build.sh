#!/bin/bash
PROJ_ROOT="$(pwd)"
CDSHOME="/c/Program Files (x86)/C-Sky/C-Sky Development Suite"

# Build C_INCLUDE_PATH: use find to locate all subdirs containing .h files
C_INC=""
while IFS= read -r dirpath; do
  base="$(basename "$dirpath")"
  case "$base" in Debug|Release|out|FW|.git|.claude|.settings|log|nfc_ref|平台代码) continue;; esac
  DWIN="$(cygpath -w "$dirpath")"
  if [ -z "$C_INC" ]; then C_INC="$DWIN"; else C_INC="$C_INC;$DWIN"; fi
done < <(find "$PROJ_ROOT" -maxdepth 1 -mindepth 1 -type d | sort)

echo "[build] C_INCLUDE_PATH dirs: $(echo "$C_INC" | tr ';' '\n' | wc -l)"

export PATH="$CDSHOME/MinGW/csky-abiv2-elf-toolchain/bin:$CDSHOME/MinGW/bin:$CDSHOME/MinGW/msys/1.0/bin:$PATH"
export C_INCLUDE_PATH="$C_INC"

cd "$PROJ_ROOT/Debug"
make clean && make all 2>&1
