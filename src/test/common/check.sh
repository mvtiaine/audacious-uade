#!/bin/sh

echo Checking test result for $TEST_NAME

if [ "$TEST_OUTPUT" != "$EXPECTED_OUTPUT" ]; then
    echo "FAILURE"
    echo "----------EXPECTED OUTPUT----------"
    echo $EXPECTED_OUTPUT
    echo "------------TEST OUTPUT------------"
    echo $TEST_OUTPUT
    echo "-----------------------------------"
    exit 1
fi

echo "SUCCESS"

