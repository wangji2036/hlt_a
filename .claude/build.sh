#!/bin/bash
# CDS Build Script
set -e

# Step 1: Setup
CDS_HOME="/c/Program Files (x86)/C-Sky/C-Sky Development Suite"
export PATH="$CDS_HOME/MinGW/csky-abiv2-elf-toolchain/bin:$CDS_HOME/MinGW/bin:$CDS_HOME/MinGW/msys/1.0/bin:$PATH"

PROJ_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
CONFIG="Debug"

echo "[INFO] PROJ_ROOT=$PROJ_ROOT"
echo "[INFO] CONFIG=$CONFIG"

# Build C_INCLUDE_PATH from known source directories (Windows-style, semicolon-separated)
C_INC=""
for base in app fml gauge hal osal power startup usbpd util; do
  d="$PROJ_ROOT/$base"
  if [ -d "$d" ]; then
    DWIN="$(cygpath -w "$d")"
    if [ -z "$C_INC" ]; then C_INC="$DWIN"; else C_INC="$C_INC;$DWIN"; fi
  fi
done
export C_INCLUDE_PATH="$C_INC"
echo "[INFO] C_INCLUDE_PATH=$C_INC"

# Step 2: Fix makefile if needed
cd "$PROJ_ROOT/$CONFIG"

if [ ! -f makefile.orig ]; then
  echo "[INFO] First time - fixing makefile..."
  cp makefile makefile.orig

  LD_REL=""
  while IFS= read -r f; do
    LD_REL="${f#$PROJ_ROOT/}"
  done < <(find "$PROJ_ROOT" -name '*.ld' ! -path '*/Debug/*' ! -path '*/out/*' ! -path '*/Release/*' 2>/dev/null | head -1)

  if [ -n "$LD_REL" ]; then
    echo "[INFO] LD=$LD_REL"
    LC_ALL=C sed -i "s| [A-Za-z]:[^\$]*[/\\\\][^\$]*\.ld| ../$LD_REL|" makefile
    LC_ALL=C sed -i "s|-T\"[A-Za-z]:[^\"]*\"|-T\"../$LD_REL\"|g" makefile
  fi

  LC_ALL=C sed -i "s|-L\"[A-Za-z]:[^\"]*[/\\\\]\([^/\\\\\"]*\)\"|-L\"../\1\"|g" makefile

  while IFS= read -r libfile; do
    LIBDIR="${libfile%/*}"
    LIBDIR_REL="${LIBDIR#$PROJ_ROOT/}"
    LIBNAME="$(basename "$libfile")"
    LNAME="${LIBNAME#lib}"; LNAME="${LNAME%.a}"
    if ! grep -q "\-l$LNAME" makefile; then
      LC_ALL=C sed -i "s|\$(LIBS)|\$(LIBS) -L\"../$LIBDIR_REL\" -l$LNAME|" makefile
      echo "[FIX] Added -l$LNAME"
    fi
  done < <(find "$PROJ_ROOT" -name 'lib*.a' ! -path '*/Debug/*' ! -path '*/out/*' ! -path '*/Release/*' 2>/dev/null)

  echo "[DONE] Makefile patched"
else
  echo "[INFO] Makefile already patched (makefile.orig exists)"
fi

# Step 3: Build
echo "[BUILD] make clean && make all"
make clean 2>&1
make all 2>&1

echo "[BUILD] Complete"
