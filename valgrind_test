#!/bin/bash

task="sfl"
test_number="34"
line_number="14"
errors=0

# Check if directory does not exist, then create it
if [ ! -d valgrind_out ]; then
    mkdir valgrind_out
fi

# Build the program
make build > /dev/null

# Loop over all of the test cases (00 - test_number)
for i in $(seq -w 00 $test_number); do
    # Run Valgrind on the "task" program with input from test cases
    valgrind --log-file="valgrind_out/$i" --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./$task <./tasks/$task/tests/$i-$task/$i-$task.in >valgrind-out
    echo -n Test $i done:

    # Check for memory leaks
    correct=$(cat valgrind_out/$i | grep 'All heap blocks were freed -- no leaks are possible' | wc -l)
    lines=$(cat valgrind_out/$i | wc -l)

    # If there are exactly 15 lines in the Valgrind output and no leaks, remove the log file
    if [ "$lines" -eq "$line_number" ] && [ "$correct" -eq 1 ]; then
        rm valgrind_out/$i
        echo " no leaks"
    else
        errors=$((errors + 1))
        if [ "$correct" -ne 1 ]; then
            echo -n " memory leaks detected"
        fi
        if [ "$correct" -ne 1 ] && [ "$lines" -ne "$line_number" ]; then
            echo -n " and"
        fi
        if [ "$lines" -ne "$line_number" ]; then
            echo -n " additional lines detected"
        fi
        echo
    fi
done

# Remove temporary output file
rm valgrind-out
make clean > /dev/null

echo
echo -n Done testing:

# Print the number of errors
if [ "$errors" -eq 0 ]; then
    echo " all tests passed"
else
    echo " $errors/$test_number tests failed"
fi
