CC = gcc
FLAGS = -Wall
PROGRAM_NAME = main
DIR_PATH = ../../
SEARCH_PATTERN = Czas
MAX_DEPTH = 5

test:
	$(CC) $(PROGRAM_NAME).c -o $(PROGRAM_NAME)
	./$(PROGRAM_NAME) $(DIR_PATH) $(SEARCH_PATTERN) $(MAX_DEPTH)

clean:
	rm $(PROGRAM_NAME)