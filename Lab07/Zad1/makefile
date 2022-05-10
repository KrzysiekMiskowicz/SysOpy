CC = gcc
FLAGS = -Wall
WORKER_1 = cookman
WORKER_2 = deliveryman
PROGRAM = main
HEADER = utils

test:
	make program
	make worker_1
	make worker_2
	./$(PROGRAM)

program:
	$(CC) $(FLAGS) $(PROGRAM).c $(HEADER).c -o $(PROGRAM)

worker_1:
	$(CC) $(FLAGS) $(WORKER_1).c $(HEADER).c -o $(WORKER_1)

worker_2:
	$(CC) $(FLAGS) $(WORKER_2).c $(HEADER).c -o $(WORKER_2)

clean:
	rm $(PROGRAM) $(WORKER_1) $(WORKER_2)