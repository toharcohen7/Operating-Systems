#!/bin/bash

# Check if there are exactly 2 arguments
if [ $# -ne 2 ]; then
    echo "Usage: $0 <source_file> <destination_directory>"
    exit 1
fi

# Check if the file does not exist
if [ ! -f "$1" ]; then
    echo "Error: File $1 does not exist."
    exit 1
fi

# Check if the destination directory does not exist; if it does not exist, create it
if [ ! -e "$2" ]; then
    mkdir "$2"
    echo "Created directory $2."
fi

# Amount of lines in file
amountOfLine=$(wc -l < "$1")

# Kind of a boolean flag - 0 if we didn't copy the second section, 1 if we did
finish=0

# Kind of a boolean flag - 0 if we need to create a new file, 1 if we are already writing to one
created=0

# Get the name of the file
fileName=$(basename "$1")
fileName="${fileName%????}"

# Get the total number of lines in the file
amountOfLine=$(wc -l < "$1")

i=1
createdFiles=1
while [ $i -le "$amountOfLine" ]
do 
    line=$(head -n $i "$1" | tail -n 1)

    # Check if the file has been created yet, if not, create it
    if [ $created -eq 0 ]; then
        created=1
        touch "$2/${fileName}_${createdFiles}.pgn"
    fi

    # If the line is not empty and the file is created, append to the file
    if [ ! -z "$line" ] && [ $created -eq 1 ]; then
        echo "$line" >> "$2/${fileName}_${createdFiles}.pgn"
        ((i++))
        continue
    fi

    # If the line is empty and we haven't finished, handle empty lines
    if [ -z "$line" ] && [ $finish -eq 0 ]; then
        echo "$line" >> "$2/${fileName}_${createdFiles}.pgn"
        finish=1
        ((i++))
        continue
    fi    

    # If we've encountered an empty line and finished handling it, prepare for the next file
    if [ -z "$line" ] && [ $finish -eq 1 ]; then
        finish=0
        created=0
        echo "Saved game to $2/${fileName}_${createdFiles}.pgn"
        ((createdFiles++))
    fi    

    ((i++))
done
