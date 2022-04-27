CC = gcc
FLAGS = -Wall
PROGRAM_NAME = main
SRC_FILE = ../Files/test_file.txt
DST_FILE = ../Files/copied_test_file.txt

test:
	$(CC) $(PROGRAM_NAME).c -o $(PROGRAM_NAME)
	./$(PROGRAM_NAME) $(SRC_FILE) $(DST_FILE)

test_input:
	$(CC) $(PROGRAM_NAME).c -o $(PROGRAM_NAME)
	./$(PROGRAM_NAME)

clean:
	rm main $(DST_FILE)