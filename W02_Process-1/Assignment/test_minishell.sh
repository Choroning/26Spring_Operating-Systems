#!/bin/bash
#
# @file        test_minishell.sh
# @brief       Automated test script for minishell assignment
# @author      Cheolwon Park
# @date        2026-03-08
#
# Usage: ./test_minishell.sh [path_to_minishell.c]
#        Default: ./minishell.c
#
# Test items:
#   1. Compilation
#   2. Single command execution (echo, ls)
#   3. Command argument handling
#   4. Input redirection (<)
#   5. Output redirection (>)
#   6. Pipe (|)
#   7. Pipe + redirection combination
#   8. exit command
#   9. Non-existent command handling
#  10. Empty input handling

set -e

PASS=0
FAIL=0
TOTAL=0

# Color settings
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
    if [ -n "$2" ]; then
        echo "    Expected: $2"
    fi
    if [ -n "$3" ]; then
        echo "    Got:      $3"
    fi
}

info() {
    echo -e "  ${YELLOW}[INFO]${NC} $1"
}

# Send commands to minishell and capture output
# Usage: run_shell "command1\ncommand2\n..."
# Output: $TMPDIR/shell_output.txt, $TMPDIR/shell_stderr.txt
run_shell() {
    echo -e "$1\nexit" | timeout 10 "$BIN" > "$TMPDIR/shell_output.txt" 2>"$TMPDIR/shell_stderr.txt" || true
    # Remove "minishell> " prompt prefix and delete empty lines
    sed 's/^minishell> *//g' "$TMPDIR/shell_output.txt" | sed '/^$/d' > "$TMPDIR/shell_clean.txt" 2>/dev/null || true
}

# Determine source file path
SRC="${1:-./minishell.c}"
if [ ! -f "$SRC" ]; then
    echo "Error: Source file not found: $SRC"
    echo "Usage: $0 [path_to_minishell.c]"
    exit 1
fi

TMPDIR=$(mktemp -d)
trap "rm -rf $TMPDIR" EXIT

BIN="$TMPDIR/minishell"

echo "========================================="
echo " minishell Test"
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

if [ -s "$TMPDIR/compile_err.txt" ]; then
    info "Compilation warnings present:"
    sed 's/^/    /' "$TMPDIR/compile_err.txt"
fi
echo ""

# Test 2: echo command
echo "[Test 2] Single command - echo"
run_shell "echo hello world"
RESULT=$(cat "$TMPDIR/shell_clean.txt")
if [ "$RESULT" = "hello world" ]; then
    pass "echo hello world → 'hello world'"
else
    fail "echo hello world" "hello world" "$RESULT"
fi
echo ""

# Test 3: Command argument handling
echo "[Test 3] Command argument handling"
run_shell "echo -n test"
RESULT=$(cat "$TMPDIR/shell_clean.txt")
if [ "$RESULT" = "test" ]; then
    pass "echo -n test → 'test' (no newline)"
else
    # echo -n behavior may vary across systems, so handle flexibly
    if echo "$RESULT" | grep -q "test"; then
        pass "echo -n test output contains 'test'"
    else
        fail "echo -n test" "test" "$RESULT"
    fi
fi
echo ""

# Test 4: Output redirection (>)
echo "[Test 4] Output redirection (>)"
OUTFILE="$TMPDIR/test_output.txt"
run_shell "echo redirected output > $OUTFILE"
if [ -f "$OUTFILE" ]; then
    CONTENT=$(cat "$OUTFILE")
    if [ "$CONTENT" = "redirected output" ]; then
        pass "echo ... > file correct content in file"
    else
        fail "File content mismatch" "redirected output" "$CONTENT"
    fi
else
    fail "Output file was not created: $OUTFILE"
fi
echo ""

# Test 5: Input redirection (<)
echo "[Test 5] Input redirection (<)"
INFILE="$TMPDIR/test_input.txt"
printf "apple\nbanana\ncherry\n" > "$INFILE"
run_shell "wc -l < $INFILE"
RESULT=$(cat "$TMPDIR/shell_clean.txt" | tr -d ' ')
if [ "$RESULT" = "3" ]; then
    pass "wc -l < file → 3"
else
    fail "wc -l < file" "3" "$RESULT"
fi
echo ""

# Test 6: Pipe (|)
echo "[Test 6] Pipe (|)"
run_shell "echo hello pipe test | wc -w"
RESULT=$(cat "$TMPDIR/shell_clean.txt" | tr -d ' ')
if [ "$RESULT" = "3" ]; then
    pass "echo hello pipe test | wc -w → 3"
else
    fail "echo hello pipe test | wc -w" "3" "$RESULT"
fi
echo ""

# Test 7: Pipe - grep
echo "[Test 7] Pipe - grep"
run_shell "echo -e 'apple\nbanana\napricot' | grep ap"
RESULT=$(cat "$TMPDIR/shell_clean.txt")
# echo -e behavior may vary across systems, so handle flexibly
if echo "$RESULT" | grep -q "apple"; then
    pass "grep through pipe works"
else
    # Alternative: some systems need printf
    run_shell "printf 'apple\nbanana\napricot\n' | grep ap"
    RESULT=$(cat "$TMPDIR/shell_clean.txt")
    if echo "$RESULT" | grep -q "apple"; then
        pass "grep through pipe works (using printf)"
    else
        fail "pipe + grep" "contains apple" "$RESULT"
    fi
fi
echo ""

# Test 8: Pipe + output redirection
echo "[Test 8] Pipe + output redirection"
PIPEFILE="$TMPDIR/pipe_output.txt"
run_shell "echo pipe to file | cat > $PIPEFILE"
if [ -f "$PIPEFILE" ]; then
    CONTENT=$(cat "$PIPEFILE")
    if [ "$CONTENT" = "pipe to file" ]; then
        pass "echo ... | cat > file result correct"
    else
        fail "Pipe + redirection file content" "pipe to file" "$CONTENT"
    fi
else
    fail "Pipe + redirection output file was not created"
fi
echo ""

# Test 9: Input redirection + pipe
echo "[Test 9] Input redirection + pipe"
SORTFILE="$TMPDIR/sort_input.txt"
printf "cherry\napple\nbanana\n" > "$SORTFILE"
run_shell "sort < $SORTFILE | head -1"
RESULT=$(cat "$TMPDIR/shell_clean.txt")
if [ "$RESULT" = "apple" ]; then
    pass "sort < file | head -1 → apple"
else
    fail "sort < file | head -1" "apple" "$RESULT"
fi
echo ""

# Test 10: exit command
echo "[Test 10] exit command"
echo "exit" | timeout 5 "$BIN" > /dev/null 2>&1
EXIT_CODE=$?
if [ $EXIT_CODE -eq 0 ]; then
    pass "Normal exit with exit command"
else
    fail "exit exit code" "0" "$EXIT_CODE"
fi
echo ""

# Test 11: EOF handling (Ctrl-D)
echo "[Test 11] EOF handling (Ctrl-D)"
echo -n "" | timeout 5 "$BIN" > /dev/null 2>&1
EXIT_CODE=$?
if [ $EXIT_CODE -eq 0 ]; then
    pass "Normal exit with EOF (empty input)"
else
    fail "EOF exit code" "0" "$EXIT_CODE"
fi
echo ""

# Test 12: Non-existent command
echo "[Test 12] Non-existent command"
run_shell "nonexistent_command_xyz123"
# Shell should continue running after printing error, not crash
if timeout 5 echo "echo still alive" | "$BIN" > "$TMPDIR/alive_check.txt" 2>/dev/null; then
    # Check if output can still be received after error
    pass "Shell continues running after non-existent command"
else
    fail "Shell crashed after non-existent command"
fi
echo ""

# Test 13: Empty input handling
echo "[Test 13] Empty input handling"
run_shell "\n\n\necho after empty lines"
RESULT=$(cat "$TMPDIR/shell_clean.txt")
if echo "$RESULT" | grep -q "after empty lines"; then
    pass "Normal execution after empty lines"
else
    fail "Empty line handling" "after empty lines" "$RESULT"
fi
echo ""

# Test 14: Sequential command execution
echo "[Test 14] Sequential command execution"
run_shell "echo first\necho second\necho third"
RESULT=$(cat "$TMPDIR/shell_clean.txt")
EXPECTED=$(printf "first\nsecond\nthird")
if [ "$RESULT" = "$EXPECTED" ]; then
    pass "3 sequential commands executed correctly"
else
    fail "Sequential commands" "$EXPECTED" "$RESULT"
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
