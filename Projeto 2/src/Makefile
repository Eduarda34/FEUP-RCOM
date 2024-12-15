CC = gcc
CFLAGS = -Wall -Wextra -g

# Source files
SRC = application.c clientTCP.c getIP.c

# Object files
OBJ = $(SRC:.c=.o)

# Output executable
OUT = application  # Change from 'download' to 'application'

# Default target
all: $(OUT)

# Link object files to create the executable
$(OUT): $(OBJ)
	$(CC) $(OBJ) -o $(OUT)

# Compile .c files into .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean object and executable files
clean:
	rm -f $(OBJ) $(OUT)
