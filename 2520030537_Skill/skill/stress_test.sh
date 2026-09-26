#!/usr/bin/env bash

set -u

REPORT="stress_report.txt"
FAILURES=0

{
    echo "Q23 - STRESS TEST REPORT"
    echo "Date: $(date)"
    echo "========================"
} > "$REPORT"

echo "TEST 1: Large pipeline" | tee -a "$REPORT"

if seq 1 100000 |
   grep -E '^[0-9]+$' |
   awk '$1 % 2 == 0' |
   sort -nr |
   head -n 1000 |
   wc -l > pipeline_output.txt
then
    RESULT=$(cat pipeline_output.txt)

    if [ "$RESULT" -eq 1000 ]; then
        echo "Pipeline: PASS" | tee -a "$REPORT"
    else
        echo "Pipeline: FAIL" | tee -a "$REPORT"
        FAILURES=$((FAILURES + 1))
    fi
else
    echo "Pipeline: FAIL" | tee -a "$REPORT"
    FAILURES=$((FAILURES + 1))
fi

echo "TEST 2: Multiple background jobs" | tee -a "$REPORT"

pids=()

for i in 1 2 3 4 5; do
    sleep 3 &
    pids+=("$!")
done

echo "Launched ${#pids[@]} jobs" | tee -a "$REPORT"

for pid in "${pids[@]}"; do
    if ! wait "$pid"; then
        echo "Job PID $pid: FAIL" | tee -a "$REPORT"
        FAILURES=$((FAILURES + 1))
    fi
done

echo "Background jobs finished." | tee -a "$REPORT"

echo "TEST 3: Performance and resources" | tee -a "$REPORT"

/usr/bin/time -v \
    -o performance.txt \
    bash -c 'seq 1 1000000 | wc -l' \
    > performance_output.txt

if [ "$?" -eq 0 ]; then
    echo "Performance test: PASS" | tee -a "$REPORT"
else
    echo "Performance test: FAIL" | tee -a "$REPORT"
    FAILURES=$((FAILURES + 1))
fi

echo "TEST 4: Error detection" | tee -a "$REPORT"

if bash -c 'command_that_does_not_exist' \
    > /dev/null 2> failure_output.txt
then
    echo "Error detection: FAIL" | tee -a "$REPORT"
    FAILURES=$((FAILURES + 1))
else
    echo "Error detection: PASS" | tee -a "$REPORT"
fi

echo "Total failures: $FAILURES" | tee -a "$REPORT"

if [ "$FAILURES" -eq 0 ]; then
    echo "FINAL RESULT: PASS" | tee -a "$REPORT"
else
    echo "FINAL RESULT: FAIL" | tee -a "$REPORT"
fi
