#!/bin/bash
for test in tests/*.as; do
    echo "----- Assembling: $test -----"
    ./assembler "$test"
    if [[ -f "${test%.as}.ob" ]]; then
        echo "  ✅ Output file created"
    else
        echo "  ❌ No output file"
    fi
    echo ""
done
