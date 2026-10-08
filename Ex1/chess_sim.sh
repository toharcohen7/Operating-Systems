#!/bin/bash

#  

# Global variables
INITIAL_BOARD=(
    "r" "n" "b" "q" "k" "b" "n" "r"  # Rank 8
    "p" "p" "p" "p" "p" "p" "p" "p"  # Rank 7
    "." "." "." "." "." "." "." "."  # Rank 6
    "." "." "." "." "." "." "." "."  # Rank 5
    "." "." "." "." "." "." "." "."  # Rank 4
    "." "." "." "." "." "." "." "."  # Rank 3
    "P" "P" "P" "P" "P" "P" "P" "P"  # Rank 2
    "R" "N" "B" "Q" "K" "B" "N" "R"  # Rank 1
)
BOARD=("${INITIAL_BOARD[@]}")
CURRENT_MOVE=0

declare -a MOVES_HISTORY
MOVES_HISTORY[0]="${BOARD[*]}"

# Functions
# Print the board function
function print_board() {

    printf "  a b c d e f g h\n"
    local rank=8
    local index=0
    local col

    while [ $rank -ge 1 ]
    do
        printf "%s " "$rank"

        for col in {0..7}
        do
            printf "%s " "${BOARD[$index]}"
            index=$((index + 1))
        done
        printf "%s\n" "$rank"
        rank=$((rank - 1))
    done

    printf "  a b c d e f g h\n"
}

# Map letters to numbers for calculation
function letter_to_col() {
    case "$1" in
        a) echo 0 ;;
        b) echo 1 ;;
        c) echo 2 ;;
        d) echo 3 ;;
        e) echo 4 ;;
        f) echo 5 ;;
        g) echo 6 ;;
        h) echo 7 ;;
    esac
}

# Function to calculate the place in the board to the array 
function calc_place() {
    local loc=$1
    local temp=${loc:0:1}
    local row=${loc:1}
    local col=$(letter_to_col "$temp")
    local place=$(( (8 - row) * 8 + col ))
    echo $place
}

# Make a move function
function make_move() {
    local move=$1
    local first_half=${move:0:2}
    local sec_half=${move:2:2}
    local promotion=${move:4:1}
    local from=$(calc_place "$first_half")
    local to_place=$(calc_place "$sec_half")
    local piece="${BOARD[$from]}"

    # Handle promotion- I chose to make cases for safety some websites says there is Uppercase in UCI.
    if [ -n "$promotion" ]
    then
        case "$piece" in
            p)
                case "$promotion" in
                    q|r|b|n) piece="$promotion" ;;
                    Q|R|B|N) piece=$(printf '%s' "$promotion" | tr '[:upper:]' '[:lower:]') ;;
                esac
                ;;
            P)
                case "$promotion" in
                    q|r|b|n) piece=$(printf '%s' "$promotion" | tr '[:lower:]' '[:upper:]') ;;
                    Q|R|B|N) piece="$promotion" ;;
                esac
                ;;
        esac
    fi

    # Castling handking, bonus, I'm assuming the input is valid
    if [[ "$piece" == "K" || "$piece" == "k" ]]
    then
        case "$move" in
            e1g1)
                BOARD[60]='.'
                BOARD[62]='K'
                BOARD[63]='.'
                BOARD[61]='R'
                return
                ;;
            e1c1)
                BOARD[60]='.'
                BOARD[58]='K'
                BOARD[56]='.'
                BOARD[59]='R'
                return
                ;;
            e8g8)
                BOARD[4]='.'
                BOARD[6]='k'
                BOARD[7]='.'
                BOARD[5]='r'
                return
                ;;
            e8c8)
                BOARD[4]='.'
                BOARD[2]='k'
                BOARD[0]='.'
                BOARD[3]='r'
                return
                ;;
        esac 
    fi   

    # En Passant handling, bonus.
    if [[ "$piece" == "p" || "$piece" == "P" ]]
    then
        local from_row=$((from / 8))
        local to_row=$((to_place / 8))
        local from_col=$((from % 8))
        local to_col=$((to_place % 8))

        # Row- check for the gap of 1 row, Col- it can be 0 or 1 difference so as long as they are not equal
        if [[ ($((from_row - to_row)) -eq 1 || $((to_row - from_row)) -eq 1) && ${from_col} -ne ${to_col} && "${BOARD[$to_place]}" == "." ]]
        then
            if [[ "$piece" == "p" ]]
            then
                BOARD[$((to_place + 8))]='.'
            elif [[ "$piece" == "P" ]]
            then
                BOARD[$((to_place - 8))]='.'
            fi
        fi
    fi 


    BOARD[$from]='.'
    BOARD[$to_place]="$piece"
}

# Getting and checking the GPN file from the command line argument
if [ ! -f "$1" ]
then
    echo "File does not exist: $1"
    exit 1
fi
PGN_file=$1

# Extract and print the meta datafrom the PGN file
echo "Metadata from PGN file:"
grep -Eo '\[[^]]*\]' "$PGN_file"
echo ""

# Extract the moves and place them in array
MOVES_PGN=$(grep -v '\[.*\]' "$PGN_file" | tr '\n' ' ')
MOVES_PY=$(python3 parse_moves.py "$MOVES_PGN")

if [ $? -ne 0 ]
then
    echo "Error parsing moves."
    exit 1
fi

MOVES_UCI=$(echo "$MOVES_PY" | tr -d ',"[]')
read -a MOVES_ARR <<< "$MOVES_UCI"
NUM_MOVES=${#MOVES_ARR[@]}

echo "Move $CURRENT_MOVE/$NUM_MOVES"
print_board

# Main loop
while true
do
    echo "Press 'd' to move forward, 'a' to move back, 'w' to go to the start, 's' to go to the end, 'q' to quit:"
    read -r key

    case "$key" in
        d)
            if [ $CURRENT_MOVE -lt $NUM_MOVES ]
            then
                make_move "${MOVES_ARR[$CURRENT_MOVE]}"
                CURRENT_MOVE=$((CURRENT_MOVE + 1))
                MOVES_HISTORY[$CURRENT_MOVE]="${BOARD[*]}"
                echo "Move $CURRENT_MOVE/$NUM_MOVES"
                print_board
            else
                echo "No more moves available."
            fi
            ;;
        a)
            if [ $CURRENT_MOVE -gt 0 ]
            then
                CURRENT_MOVE=$((CURRENT_MOVE - 1))
                IFS=' ' read -r -a BOARD <<< "${MOVES_HISTORY[$CURRENT_MOVE]}"
                echo "Move $CURRENT_MOVE/$NUM_MOVES"
                print_board
            else
                echo "Move $CURRENT_MOVE/$NUM_MOVES"
                print_board
            fi
            ;;
        w)
            CURRENT_MOVE=0
            IFS=' ' read -r -a BOARD <<< "${MOVES_HISTORY[$CURRENT_MOVE]}"
            echo "Move $CURRENT_MOVE/$NUM_MOVES"
            print_board
            ;;
        s)
            while [ $CURRENT_MOVE -lt $NUM_MOVES ]
            do
                make_move "${MOVES_ARR[$CURRENT_MOVE]}"
                CURRENT_MOVE=$((CURRENT_MOVE + 1))
                MOVES_HISTORY[$CURRENT_MOVE]="${BOARD[*]}"
            done
            echo "Move $CURRENT_MOVE/$NUM_MOVES"
            print_board
            ;;
        q)
            echo "Exiting."
            echo "End of game."
            exit 0
            ;;
        *)
            echo "Invalid key pressed: $key"
            ;;
    esac
done
