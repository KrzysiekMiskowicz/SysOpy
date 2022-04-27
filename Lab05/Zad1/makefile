CC = gcc
FLAGS = -Wall
PROGRAM_NAME = main
TEST_FILE_PATH = ./test.txt

test:
	$(CC) $(FLAGS) $(PROGRAM_NAME).c -o $(PROGRAM_NAME)
	./$(PROGRAM_NAME) $(TEST_FILE_PATH)

clean:
	rm $(PROGRAM_NAME)