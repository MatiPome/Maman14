#!/bin/sh

ASSEMBLER=../assembler   # <- path to your assembler executable

echo "Running all .as tests in the current folder..."

for f in *.as
do
    testname=$(basename "$f" .as)
    echo "---------------------------"
    echo "Testing: $f"
    $ASSEMBLER "$f" > "$testname.log" 2>&1

    # Check for .ob file as an indicator of successful assembly
    if [ -f "$testname.ob" ]; then
        echo "✅ $f: Output file $testname.ob created."
    else
        echo "❌ $f: No .ob file created. (Likely error, see $testname.log)"
    fi

    # Optionally show last 5 lines of log if there was an error
    if [ ! -f "$testname.ob" ]; then
        echo "--- Last lines from $testname.log ---"
        tail -n 5 "$testname.log"
    fi

    # Clean up output files for next test if you wish:
    # rm -f "$testname.ob" "$testname.ent" "$testname.ext"
done

echo "---------------------------"
echo "All tests finished!"
