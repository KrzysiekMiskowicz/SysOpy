CC = gcc
FLAGS = -Wall
PROGRAM_NAME = main
CHILD_NAME = child
HANDLER = handler
IGNORE = ignore
MASK = mask
PENDING = pending

test:
	$(CC) $(PROGRAM_NAME).c -o $(PROGRAM_NAME)
	$(CC) $(CHILD_NAME).c -o $(CHILD_NAME)
	./$(PROGRAM_NAME) $(MASK)

clean:
	rm $(PROGRAM_NAME) $(CHILD_NAME)