#!/bin/bash

if [ -z "$1" ]; then
    echo "Usage: $0 <size_in_MB>"
    exit 1
fi

# Define the size of the file in bytes (1 GB)
SIZE=$(($1*1024*1024))

# Function to calculate percentage completion
get_percentage() {
    local current_size=$1
    local total_size=$2
    echo "$(printf "%.3f" "$(echo "scale=3; ($current_size * 100) / $total_size" | bc)")"
}

# Generate random text and append it to the file until it reaches the desired size
current_size=0
while [ $current_size -lt $SIZE ]; do
    # Generate random text and append it to the file
    cat /dev/urandom | tr -dc 'a-zA-Z0-9' | head -c 1000 >> random_text.txt
    # Update current size
    current_size=$(du -b random_text.txt | awk '{print $1}')
    # Calculate percentage completion
    percentage=$(get_percentage $current_size $SIZE)
    # Clear the previous progress and print the new progress
    printf "Progress: %.3f%%\r" $percentage
done

echo "File generated successfully!"

