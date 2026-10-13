# Sourced by the build scripts: Open Watcom v2 (https://github.com/open-watcom/open-watcom-v2)
# on the PATH. WATCOM points to its folder (default: ~/.local/opt/watcom).
WATCOM=${WATCOM:-$HOME/.local/opt/watcom}
case "$(uname -s)-$(uname -m)" in
  Darwin-arm64) WATCOM_BIN=armo64 ;;
  Darwin-*) WATCOM_BIN=bino64 ;;
  Linux-aarch64) WATCOM_BIN=arml64 ;;
  *) WATCOM_BIN=binl64 ;;
esac
export WATCOM PATH="$WATCOM/$WATCOM_BIN:$PATH"
if ! command -v wpp >/dev/null 2>&1; then
  echo "Error: Open Watcom not found in $WATCOM (set WATCOM=<folder>)" >&2
  exit 1
fi

# obj <source>: the object file of a source in $OBJ (folders joined with '_').
obj() {
  printf '%s/%s.obj' "$OBJ" "$(printf '%s' "${1%.*}" | tr '/' '_')"
}

# compile <compiler> "<flags>" <source>...
# The module name is cut to 24 characters: the 16-bit compiler of the large model crashes
# on a longer one (its code segment is named after it).
compile() {
  compiler=$1
  flags=$2
  shift 2
  for source in "$@"; do
    module=$(basename "${source%.*}" | cut -c1-24)
    $compiler "$source" $flags -nm="$module" -fo="$(obj "$source")"
  done
}

# objs <source>...: their object files, separated by commas, for wlink.
objs() {
  for source in "$@"; do printf '%s,' "$(obj "$source")"; done | sed 's/,$//'
}
