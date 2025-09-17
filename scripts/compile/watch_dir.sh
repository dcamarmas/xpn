#!/bin/bash

# Declare variables to hold the states in memory
declare -g current_state=""
declare -g previous_state=""
watch_dir() {
    # Check if directory argument is provided
    if [ -z "$1" ]; then
        echo "Usage: $0 <directory>"
        exit 1
    fi

    directory="$1"

    # Function to compare states and display changes
    compare_states() {
        if [ -n "$previous_state" ]; then
            diff <(echo "$previous_state") <(echo "$current_state") | while read line; do
                if [[ "$line" == "<"* ]]; then
                    echo "File modified: ${line:2}"
                    return 1 # Indicate changes found
                elif [[ "$line" == ">"* ]]; then
                    echo "File added: ${line:2}"
                    return 1 # Indicate changes found
                fi
            done
            if [ $? -eq "1" ]; then
                return 0 # Indicate changes found
            else
                return 1 # Indicate no changes found
            fi
        else
            return 1 # Indicate no changes found (first run)
        fi
    }
 
    # Main loop
    while true; do
        # start_time=$(date +%s.%N)
        # Get current state of files and store in memory
        current_state=$(find "$directory" -type f -not -path "*../build/*" -printf "%p %s %T+\n")

        # Compare states and display changes
        compare_states
        ret=$?

        # end_time=$(date +%s.%N)
        # duration=$(echo "$end_time - $start_time" | bc)
        # echo "Iteration took $duration seconds."
        if [ $ret -eq "1" ]; then
            sleep 2 # Wait 2 seconds before next check
        else
            echo "Changes detected. Exiting..."
            previous_state="$current_state" # Move current state to previous state
            break # Exit the loop
        fi

        # Move current state to previous state
        previous_state="$current_state"
    done
}