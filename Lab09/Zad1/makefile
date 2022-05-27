CC = gcc
FLAGS = -Wall -lpthread -lrt
PROGRAM = main


test:
	make clean
	$(CC) $(FLAGS) -o $(PROGRAM) $(PROGRAM).c
	./$(PROGRAM)

clean:
	rm -f $(PROGRAM) $(OUTPUT) $(REPORT)