#!/bin/bash

set -e

echo Checking test result for $TEST_NAME
echo Running ${TEST}
# doesn't work on Debian
#TEST_OUTPUT=$(eval time ${TEST})
TEST_OUTPUT=$(eval ${TEST})

if [[ "$TEST_OUTPUT" =~ .*$EXPECTED_OUTPUT.* ]]; then
    SUCCESS="yes"
else
    echo "----------EXPECTED OUTPUT----------"
    echo $EXPECTED_OUTPUT
    echo "------------TEST OUTPUT------------"
    echo $TEST_OUTPUT
    echo "-----------------------------------"
fi

if [ "$SUCCESS" != "yes" ]; then
    echo "FAILURE"
    exit 1
fi

echo "SUCCESS"
