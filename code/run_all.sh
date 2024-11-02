#!/bin/bash

# Clear the terminal
clear

# Clean the project
make clean

# Build the project
make all

# Run cable simulator in the background
sudo make run_cable &
