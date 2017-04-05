#!/bin/bash

for f in tests/*.in; do
    echo "Testing $f"
    ./a.out < "$f" 2>/dev/null | sed 's/ *$//g' | diff "$f.out" -
done
