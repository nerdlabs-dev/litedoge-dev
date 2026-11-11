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

# Make an empty wallet with the daemon first (litedoged and litedoge-cli are
# next to litedoge-qt), so the wallet pages can be shown
BINDIR="$(dirname "$BIN")"
if [ -x "$BINDIR/litedoged" ]; then
    "$BINDIR/litedoged" -datadir="$DATA" -daemon=0 -server=1 -rpcuser=ci -rpcpassword=ci > /dev/null 2>&1 &
    D=$!
    for _ in $(seq 1 30); do "$BINDIR/litedoge-cli" -datadir="$DATA" -rpcuser=ci -rpcpassword=ci getblockcount >/dev/null 2>&1 && break; sleep 1; done
    "$BINDIR/litedoge-cli" -datadir="$DATA" -rpcuser=ci -rpcpassword=ci createwallet ci || true
    "$BINDIR/litedoge-cli" -datadir="$DATA" -rpcuser=ci -rpcpassword=ci stop || true
    wait $D
fi

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
    [ -z "$WID" ] && WID="$(xdotool search --onlyvisible --class 'litedoge' 2>/dev/null | head -n1)"
    [ -z "$WID" ] && WID="$(xdotool search --onlyvisible --classname 'litedoge' 2>/dev/null | head -n1)"
    [ -n "$WID" ] && break
    sleep 2
done
print_shots() {
    if [ "${PRINT_SCREENSHOTS_BASE64:-0}" = 1 ]; then
        for f in "$OUT"/*.png; do
            [ -f "$f" ] || continue
            echo "=== BEGIN SCREENSHOT $(basename "$f") ==="
            base64 -w 1000 "$f"
            echo "=== END SCREENSHOT ==="
        done
    fi
}
if [ -z "$WID" ]; then
    echo "The wallet window did not appear"
    import -window root "$OUT/00-screen.png"
    xdotool search --name '.' getwindowname %@ 2>/dev/null | head -n 20
    cat "$OUT/litedoge-qt.log"
    echo "--- last lines of debug.log"
    tail -n 60 "$DATA/debug.log" 2>/dev/null
    if ! kill -0 "$APP" 2>/dev/null && command -v gdb >/dev/null; then
        echo "--- the wallet exited; running it again under gdb for a backtrace"
        timeout 240 gdb -batch -ex run -ex "thread apply all bt 25" --args "$BIN" -datadir="$DATA" -splash=0 -choosedatadir=0 -wallet=ci 2>&1 | grep -v "^\[New Thread\|^\[Thread" | tail -n 250
    fi
    print_shots
    cleanup
    exit 1
fi
sleep 8
# The sync overlay covers the pages: hide it (it is shown again by the wallet
# only when asked)
xdotool windowfocus --sync "$WID" 2>/dev/null; xdotool key Escape; sleep 1
eval "$(xdotool getwindowgeometry --shell "$WID")"
xdotool mousemove --window "$WID" $((WIDTH - 96)) $((HEIGHT - 108)) click 1; sleep 1
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
print_shots
exit 0
