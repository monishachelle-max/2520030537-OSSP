#!/bin/bash

echo "Q22 AUTOMATED TEST REPORT" > test_report.txt

./memory_test > actual_output.txt

if diff -q expected_output.txt actual_output.txt; then
    echo "Output Test: PASS" | tee -a test_report.txt
else
    echo "Output Test: FAIL" | tee -a test_report.txt
fi

valgrind --leak-check=full \
    --error-exitcode=1 \
    --log-file=valgrind_report.txt \
    ./memory_test > /dev/null

if [ $? -eq 0 ]; then
    echo "Memory Test: PASS" | tee -a test_report.txt
else
    echo "Memory Test: FAIL" | tee -a test_report.txt
file

