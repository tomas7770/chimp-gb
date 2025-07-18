#!/bin/bash

total_pass_count=0
total_test_count=0

for file in $2/*.json;
do
    result=$($1 $file)
    pass_count=0
    test_count=0
    while IFS= read -r line ; do
        test_count=$((test_count+1))
        total_test_count=$((total_test_count+1))
        if [[ $line == *"CYCLES"* ]]; then
            pass_count=$((pass_count+1))
            total_pass_count=$((total_pass_count+1))
        fi
    done <<< "$result"
    echo "$file: $((100*pass_count/test_count))% ($pass_count/$test_count)"
done

echo "Total tests passed: $((100*total_pass_count/total_test_count))% ($total_pass_count/$total_test_count)"
echo "Warning: Memory bus activity not tested yet!"
