#!/bin/bash

GREEN='\033[0;32m'
BLUE='\033[0;36m'
PURPLE='\033[1;35m'
YELLOW='\033[0;33m'
NORMAL='\033[0m'  # Define NORMAL as reset (in ASNI)


clear

# Print a message to the user
echo -e "${BLUE}Welcome to the SH13 Online Game from Vasileios Filippos Skarleas and Manu Guerinel. Once you are ready to start exploring the capabilities of the software simply press ENTER"
echo -e "${GREEN}Bienvenue au jeu en ligne SH13 de Vasileios Filippos Skarleas et Manu Guerinel. Une fois que vous êtes prêt à commencer à explorer les capacités du logiciel, appuyez simplement sur ENTRÉE."

# Read a line of input from the user
read input

# If the user presses Enter, run the specified commands
if [[ -n "$input" ]]; then
  echo -e "${PURPLE}You did not press ENTER and the program will terminate"
else
  # Run the initial commands
  clear
  echo -e "${PURPLE}Initialising environment..."
  sleep 2
  make clean
  make
  clear
  echo -e "${NORMAL}This software allows you to play the SH13 game online.\n"
  echo -e "${BLUE}NOTE:${NORMAL} You have the option to play either on 3 or 4 people. You are asked to choose which mode you wish below. More details about the game and its implimentation at https://github.com/vskarleas/ROB4-S7-SH13.\n"
  echo -e "++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

  # Infinite loop with user options
  while true; do
    echo -e "\n- 4p: Runs the game for 4 people."
    echo -e "- 3p: Runs the game for 3 people."
    echo -e "- exit: Exits the program."
    echo -e "${BLUE}========================================================\n${NORMAL}Enter a command (4p, 3p, exit):"
    read command

    case "$command" in
      4p)
        clear
        # Free port 32000 if it's in use
        # Free port 32000 if it's in use
        PORT=32000
        PID=$(lsof -ti tcp:$PORT)
        if [[ -n "$PID" ]]; then
          echo -e "${YELLOW}Port $PORT is in use by process $PID. Releasing it...${NORMAL}"
          kill -9 $PID
          echo -e "${GREEN}Port $PORT is now available.${NORMAL}"
        fi

        echo -e "${GREEN}You selected 4-player mode.${NORMAL}"
        # Start the server
        echo -e "${PURPLE}Starting the server...${NORMAL}"
        ./server_4 32000 &
        SERVER_PID=$!  # Save the server process ID
        sleep 2

        # Check if the server is running
        if ! ps -p $SERVER_PID > /dev/null; then
          echo -e "${YELLOW}Failed to start the server. Exiting 4-player mode.${NORMAL}"
          continue
        fi

        # player names
        for i in {1..4}; do
          clear
          echo -n "Enter name for player $i: "
          read "p$i"
        done

        for i in {1..4}; do
          eval "./sh13_4 127.0.0.1 32000 127.0.0.1 $((32000 + $i)) \${p$i} &"
        done

        # Wait for all player processes to finish
        wait
        ;;
      3p)
        clear
        # Free port 32000 if it's in use
        PORT=32000
        # Free port 32000 if it's in use
        PORT=32000
        PID=$(lsof -ti tcp:$PORT)
        if [[ -n "$PID" ]]; then
          echo -e "${YELLOW}Port $PORT is in use by process $PID. Releasing it...${NORMAL}"
          kill -9 $PID
          echo -e "${GREEN}Port $PORT is now available.${NORMAL}"
        fi

        echo -e "${GREEN}You selected 3-player mode.${NORMAL}"
        # Start the server
        echo -e "${PURPLE}Starting the server...${NORMAL}"
        ./server_3 32000 &
        SERVER_PID=$! 
        sleep 2

        # Check if the server is running
        if ! ps -p $SERVER_PID > /dev/null; then
          echo -e "${YELLOW}Failed to start the server. Exiting 3-player mode.${NORMAL}"
          continue
        fi

        for i in {1..3}; do
          clear
          echo -n "Enter name for player $i: "
          read "p$i"
        done

        for i in {1..3}; do
          eval "./sh13_3 127.0.0.1 32000 127.0.0.1 $((32000 + $i)) \${p$i} &"
        done

        # Wait for all player processes to finish
        wait
        ;;
      exit)
        break  # Exit the loop
        ;;
      *)
        echo -e "${YELLOW}\nInvalid command. Please try again!${NORMAL}"
        ;;
    esac
  done

  # Commands to execute after exiting the loop
  make clean
  PORT=32000
  PID=$(lsof -ti tcp:$PORT)
  if [[ -n "$PID" ]]; then
    echo -e "${YELLOW}Port $PORT is in use by process $PID. Releasing it...${NORMAL}"
    kill -9 $PID
    echo -e "${GREEN}Port $PORT is now available.${NORMAL}"
  fi
  clear
  echo -e "${GREEN}SH13 Online Game has been successfully terminated\n"
  echo -e "${NORMAL}Thank you! | ${BLUE}Copyright © 2024 - $(date +%Y) ${NORMAL} Vasileios Filippos Skarleas, Manu Guerinel - All rights reserved"
fi
