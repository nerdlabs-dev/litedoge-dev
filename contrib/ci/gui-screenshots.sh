#!/usr/bin/env bash
# Takes screenshots of the LiteDoge GUI wallet on a virtual display (used by
# CI, so GUI changes can be checked without a desktop).
#
# Usage: contrib/ci/gui-screenshots.sh <path to litedoge-qt> <output directory>
# Needs: Xvfb, xdotool, ImageMagick (import)
# SIZE=WxH sets the virtual screen size (default 1280x900).
# PRINT_SCREENSHOTS_BASE64=1 also prints them into the log as base64.
#
# Uses a throwaway data directory with networking off and a new empty wallet,
# so no real wallet or chain data is involved.
set -u

BIN="$(readlink -f "$1")"
mkdir -p "$2"
OUT="$(cd "$2" && pwd)"
DATA="$(mktemp -d)"
cat > "$DATA/litedoge.conf" <<CONF
listen=0
dnsseed=0
connect=0
upnp=0
natpmp=0
CONF
rm -rf "$HOME/.config/LiteDoge"

Xvfb :99 -screen 0 "${SIZE:-1280x900}x24" >/dev/null 2>&1 &
XVFB=$!
export DISPLAY=:99
sleep 2

"$BIN" -datadir="$DATA" -splash=0 -choosedatadir=0 -wallet=ci > "$OUT/litedoge-qt.log" 2>&1 &
APP=$!

cleanup() {
    kill "$APP" 2>/dev/null; sleep 3; kill -9 "$APP" 2>/dev/null
    kill "$XVFB" 2>/dev/null
}

WID=""
for _ in $(seq 1 90); do
    WID="$(xdotool search --onlyvisible --name '[Ll]ite[Dd]oge' 2>/dev/null | head -n1)"
    [ -n "$WID" ] && break
    sleep 2
done
if [ -z "$WID" ]; then
    echo "The wallet window did not appear"
    import -window root "$OUT/00-screen.png"
    cat "$OUT/litedoge-qt.log"
    cleanup
    exit 1
fi
sleep 8
echo "Window title: $(xdotool getwindowname "$WID")"
import -window root "$OUT/00-screen.png"

focus() { xdotool windowfocus --sync "$WID" 2>/dev/null || true; }
shot()  { sleep 2; import -window "$1" "$OUT/$2.png" && echo "saved $2.png"; }

names=(overview send receive transactions minting explorer multisig)
for n in 1 2 3 4 5 6 7; do
    focus
    xdotool key "alt+$n"
    shot "$WID" "$(printf '%02d' "$n")-${names[$((n-1))]}"
done

# Smaller and larger window sizes, to check the layouts resize properly
focus; xdotool key alt+1
xdotool windowsize "$WID" 850 550; shot "$WID" "20-overview-small"
xdotool key alt+4; shot "$WID" "21-transactions-small"
xdotool windowsize "$WID" 1260 860; xdotool key alt+1; shot "$WID" "22-overview-large"

dialog() {
    local title="$1" file="$2"; shift 2
    focus
    for k in "$@"; do xdotool key "$k"; sleep 0.7; done
    sleep 2
    local d
    d="$(xdotool search --onlyvisible --name "$title" 2>/dev/null | head -n1)"
    if [ -n "$d" ]; then shot "$d" "$file"; else echo "Window '$title' not found"; import -window root "$OUT/$file-screen.png"; fi
    xdotool key Escape
    sleep 1
}
dialog '^Options'          10-options alt+s o
dialog 'Node window'       11-node    ctrl+i
dialog '^About'            12-about   alt+h a

cleanup

if [ "${PRINT_SCREENSHOTS_BASE64:-0}" = 1 ]; then
    for f in "$OUT"/*.png; do
        echo "=== BEGIN SCREENSHOT $(basename "$f") ==="
        base64 -w 1000 "$f"
        echo "=== END SCREENSHOT ==="
    done
fi
exit 0
