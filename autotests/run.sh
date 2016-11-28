#!/usr/bin/env bash

EXE="../build/bin/rsphp"
failed=0
succeeded=0

for file in *.rsphp; do
    echo "Running $file"
    expected=$(cat "$file".out);
    out=$($EXE < $file);
    if [ "$expected" != "$out" ]; then
        echo "FAIL!"
        echo "Expected:"
        echo "----"
        echo "$expected"
        echo "----"
        echo "Output:"
        echo "----"
        echo "$out"
        echo "----"
        failed=$((failed + 1))
    else
        echo "SUCCESS"
        succeeded=$((succeeded + 1))
    fi
    echo ""
done

echo "> Succeeded: $succeeded Failed: $failed Total:" $((failed + succeeded))
