#!/bin/sh

set -e

echo Checking test result for $TEST_NAME
echo Running ${TEST}
# doesn't work on Debian
#TEST_OUTPUT=$(eval time ${TEST})
TEST_OUTPUT=$(eval ${TEST})

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

