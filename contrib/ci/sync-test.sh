#!/usr/bin/env bash
# Syncs a fresh litedoged from the real LDOGE network for a while and reports
# how far it got, which peers it used and whether any block was rejected.
# Used by CI as a check that the wallet still follows the LDOGE chain.
#
# Usage: SYNC_MINUTES=30 contrib/ci/sync-test.sh <directory with litedoged and litedoge-cli>
# Writes sync-test/debug.log. Fails if a block was rejected as invalid or if
# no block past the genesis block was downloaded.
set -u

BIN="$(readlink -f "$1")"
MINUTES="${SYNC_MINUTES:-30}"
OUT="$PWD/sync-test"
DATA="$(mktemp -d)"
mkdir -p "$OUT"
cat > "$DATA/litedoge.conf" <<CONF
server=1
listen=0
upnp=0
natpmp=0
debug=net
debug=validation
logips=1
logtimemicros=0
rpcuser=ci
rpcpassword=ci-$RANDOM-$RANDOM
CONF

CLI() { "$BIN/litedoge-cli" -datadir="$DATA" "$@"; }

"$BIN/litedoged" -datadir="$DATA" -daemon=0 > "$OUT/stdout.log" 2>&1 &
PID=$!
trap 'kill $PID 2>/dev/null; sleep 5; cp "$DATA/debug.log" "$OUT/debug.log" 2>/dev/null' EXIT

for _ in $(seq 1 60); do CLI getblockcount >/dev/null 2>&1 && break; sleep 2; done

summary() { [ -n "${GITHUB_STEP_SUMMARY:-}" ] && echo "$*" >> "$GITHUB_STEP_SUMMARY"; echo "$*"; }

START=$(date +%s)
LAST=0
summary "### Mainnet sync test ($MINUTES minutes)"
summary ""
summary "| minute | blocks | headers | peers | blocks/min |"
summary "|---|---|---|---|---|"
for m in $(seq 1 "$MINUTES"); do
    sleep 60
    if ! kill -0 $PID 2>/dev/null; then summary "litedoged exited early"; break; fi
    B=$(CLI getblockcount 2>/dev/null || echo "?")
    H=$(CLI getblockchaininfo 2>/dev/null | python3 -c 'import json,sys; print(json.load(sys.stdin)["headers"])' 2>/dev/null || echo "?")
    P=$(CLI getconnectioncount 2>/dev/null || echo "?")
    R="?"; [ "$B" != "?" ] && R=$((B - LAST)) && LAST=$B
    summary "| $m | $B | $H | $P | $R |"
    if grep -qE 'InvalidChainFound|ConnectBlock.*failed|ERROR: (Accept|Connect|Process)Block|CheckProofOfStake.*failed' "$DATA/debug.log"; then
        # keep going for a minute so the log shows what happened next
        sleep 30
        break
    fi
done

summary ""
summary "**Peers:**"
summary '```'
CLI getpeerinfo 2>/dev/null | python3 -c '
import json,sys
for p in json.load(sys.stdin):
    print(p.get("addr"), p.get("version"), p.get("subver"), "start", p.get("startingheight"), "synced", p.get("synced_blocks"))' | tee -a "${GITHUB_STEP_SUMMARY:-/dev/null}"
summary '```'
summary "**Chain:**"
summary '```'
CLI getblockchaininfo 2>&1 | tee -a "${GITHUB_STEP_SUMMARY:-/dev/null}"
summary '```'

cp "$DATA/debug.log" "$OUT/debug.log"
echo "::group::Problems in debug.log"
grep -nE 'ERROR|error:|invalid|InvalidChainFound|bad-|failed|Misbehaving|disconnecting' "$OUT/debug.log" | head -n 300
echo "::endgroup::"
echo "::group::Last 300 lines of debug.log"
tail -n 300 "$OUT/debug.log"
echo "::endgroup::"

FAIL=0
if grep -qE 'InvalidChainFound|ConnectBlock.*failed|ERROR: (Accept|Connect|Process)Block|CheckProofOfStake.*failed' "$OUT/debug.log"; then
    summary "**A block was rejected - see the log above.**"
    grep -nE -m 20 -B5 -A5 'InvalidChainFound|ConnectBlock.*failed|ERROR: (Accept|Connect|Process)Block|CheckProofOfStake.*failed' "$OUT/debug.log" | head -n 200
    FAIL=1
fi
B=$(CLI getblockcount 2>/dev/null || echo 0)
if [ "${B:-0}" = "0" ] || [ "$B" = "?" ]; then
    summary "**No blocks were downloaded.**"
    FAIL=1
fi
CLI stop >/dev/null 2>&1
exit $FAIL
