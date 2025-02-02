#!/bin/bash

set -e

echo Checking test result for $TEST_NAME
echo Running ${TEST}
# doesn't work on Debian
#TEST_OUTPUT=$(eval time ${TEST})
TEST_OUTPUT=$(eval ${TEST})

if [[ ! "$TEST_OUTPUT" =~ .*$EXPECTED_OUTPUT.* ]]; then
    echo "----------EXPECTED OUTPUT----------"
    echo $EXPECTED_OUTPUT
    echo "------------TEST OUTPUT------------"
    echo $TEST_OUTPUT
    echo "-----------------------------------"
    echo "FAILURE"
    exit 1
fi

echo "SUCCESS"

