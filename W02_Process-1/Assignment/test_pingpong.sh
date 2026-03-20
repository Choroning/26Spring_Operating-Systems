#!/bin/bash
#
# @file        test_pingpong.sh
# @brief       Automated test script for pingpong assignment
# @author      Cheolwon Park
# @date        2026-03-08
#
# Usage: ./test_pingpong.sh [path_to_pingpong.c]
#        Default: ./pingpong.c
#
# Test items:
#   1. Compilation success
#   2. Normal exit on execution
#   3. Output format validation ("received ping", "received pong")
#   4. PID format check
#   5. Round-trip time output check
#   6. Repeated execution stability

set -e

PASS=0
FAIL=0
TOTAL=0

# Color settings (disabled if not a terminal)
if [ -t 1 ]; then
    GREEN='\033[0;32m'
    RED='\033[0;31m'
    YELLOW='\033[0;33m'
    NC='\033[0m'
else
    GREEN=''
    RED=''
    YELLOW=''
    NC=''
fi

pass() {
    PASS=$((PASS + 1))
    TOTAL=$((TOTAL + 1))
    echo -e "  ${GREEN}[PASS]${NC} $1"
}

fail() {
    FAIL=$((FAIL + 1))
    TOTAL=$((TOTAL + 1))
    echo -e "  ${RED}[FAIL]${NC} $1"
}

info() {
    echo -e "  ${YELLOW}[INFO]${NC} $1"
}

# Determine source file path
SRC="${1:-./pingpong.c}"
if [ ! -f "$SRC" ]; then
    echo "Error: Source file not found: $SRC"
    echo "Usage: $0 [path_to_pingpong.c]"
    exit 1
fi

TMPDIR=$(mktemp -d)
trap "rm -rf $TMPDIR" EXIT

BIN="$TMPDIR/pingpong"

echo "========================================="
echo " pingpong Test"
echo "========================================="
echo ""
echo "Source: $SRC"
echo ""

# Test 1: Compilation
echo "[Test 1] Compilation"
if gcc -Wall -o "$BIN" "$SRC" 2>"$TMPDIR/compile_err.txt"; then
    pass "gcc -Wall compilation succeeded"
else
    fail "Compilation failed"
    echo "    Compiler output:"
    sed 's/^/    /' "$TMPDIR/compile_err.txt"
    echo ""
    echo "========================================="
    echo " Result: Skipping remaining tests due to compilation failure."
    echo "========================================="
    exit 1
fi

# Check for warnings
if [ -s "$TMPDIR/compile_err.txt" ]; then
    info "Compilation warnings present:"
    sed 's/^/    /' "$TMPDIR/compile_err.txt"
fi
echo ""

# Test 2: Execution and exit (exit 0 within 5 seconds)
echo "[Test 2] Execution and exit"
if timeout 5 "$BIN" > "$TMPDIR/output.txt" 2>"$TMPDIR/stderr.txt"; then
    pass "Normal exit (exit code 0)"
else
    EXIT_CODE=$?
    if [ $EXIT_CODE -eq 124 ]; then
        fail "Timeout (ran for more than 5 seconds - possible infinite wait)"
    else
        fail "Abnormal exit (exit code $EXIT_CODE)"
    fi
    if [ -s "$TMPDIR/stderr.txt" ]; then
        echo "    stderr:"
        sed 's/^/    /' "$TMPDIR/stderr.txt"
    fi
fi
echo ""

# Test 3: Output format - "received ping"
echo "[Test 3] Output format - ping"
if grep -q "received ping" "$TMPDIR/output.txt" 2>/dev/null; then
    pass "'received ping' message present"
else
    fail "'received ping' message missing"
    echo "    Actual output:"
    sed 's/^/    /' "$TMPDIR/output.txt"
fi

# Check PID format in ping line
if grep -qE '^[0-9]+: received ping$' "$TMPDIR/output.txt" 2>/dev/null; then
    pass "ping line format correct (<pid>: received ping)"
else
    fail "ping line format is not '<pid>: received ping'"
    PING_LINE=$(grep "received ping" "$TMPDIR/output.txt" 2>/dev/null || echo "(none)")
    echo "    Actual: $PING_LINE"
fi
echo ""

# Test 4: Output format - "received pong"
echo "[Test 4] Output format - pong"
if grep -q "received pong" "$TMPDIR/output.txt" 2>/dev/null; then
    pass "'received pong' message present"
else
    fail "'received pong' message missing"
fi

if grep -qE '^[0-9]+: received pong$' "$TMPDIR/output.txt" 2>/dev/null; then
    pass "pong line format correct (<pid>: received pong)"
else
    fail "pong line format is not '<pid>: received pong'"
    PONG_LINE=$(grep "received pong" "$TMPDIR/output.txt" 2>/dev/null || echo "(none)")
    echo "    Actual: $PONG_LINE"
fi
echo ""

# Test 5: Verify that ping and pong PIDs are different
echo "[Test 5] PID verification"
PING_PID=$(grep "received ping" "$TMPDIR/output.txt" 2>/dev/null | grep -oE '^[0-9]+' || echo "")
PONG_PID=$(grep "received pong" "$TMPDIR/output.txt" 2>/dev/null | grep -oE '^[0-9]+' || echo "")

if [ -n "$PING_PID" ] && [ -n "$PONG_PID" ] && [ "$PING_PID" != "$PONG_PID" ]; then
    pass "ping PID($PING_PID) != pong PID($PONG_PID) (different processes)"
else
    if [ "$PING_PID" = "$PONG_PID" ] && [ -n "$PING_PID" ]; then
        fail "ping PID and pong PID are the same ($PING_PID) - they should be different processes"
    else
        fail "PID extraction failed"
    fi
fi
echo ""

# Test 6: Round-trip time output
echo "[Test 6] Round-trip time output"
if grep -qE 'Round-trip time:.*us' "$TMPDIR/output.txt" 2>/dev/null; then
    pass "Round-trip time output present"
    RTT=$(grep "Round-trip time" "$TMPDIR/output.txt")
    info "$RTT"
else
    fail "Round-trip time output missing"
fi
echo ""

# Test 7: Output order (ping should be printed before pong)
echo "[Test 7] Output order"
PING_LINE_NUM=$(grep -n "received ping" "$TMPDIR/output.txt" 2>/dev/null | head -1 | cut -d: -f1 || echo "0")
PONG_LINE_NUM=$(grep -n "received pong" "$TMPDIR/output.txt" 2>/dev/null | head -1 | cut -d: -f1 || echo "0")

if [ "$PING_LINE_NUM" -gt 0 ] 2>/dev/null && [ "$PONG_LINE_NUM" -gt 0 ] 2>/dev/null; then
    if [ "$PING_LINE_NUM" -lt "$PONG_LINE_NUM" ]; then
        pass "ping printed before pong"
    else
        fail "pong printed before ping (ping should come first)"
    fi
else
    fail "Unable to verify output order"
fi
echo ""

# Test 8: Repeated execution stability
echo "[Test 8] Repeated execution stability (5 runs)"
STABLE=true
for i in $(seq 1 5); do
    if ! timeout 5 "$BIN" > /dev/null 2>&1; then
        STABLE=false
        fail "Repeated execution $i/5 failed"
        break
    fi
done
if $STABLE; then
    pass "All 5 repeated executions succeeded"
fi
echo ""

# Summary
echo "========================================="
echo " Result Summary"
echo "========================================="
echo -e " Passed: ${GREEN}${PASS}${NC} / ${TOTAL}"
echo -e " Failed: ${RED}${FAIL}${NC} / ${TOTAL}"
echo ""

if [ $FAIL -eq 0 ]; then
    echo -e " ${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e " ${RED}Some tests failed${NC}"
    exit 1
fi
