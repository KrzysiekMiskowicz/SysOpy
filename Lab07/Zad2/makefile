CC = gcc
FLAGS = -Wall -lpthread -lrt
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
	$(CC) $(FLAGS) -o $(PROGRAM) $(PROGRAM).c $(HEADER).c

worker_1:
	$(CC) $(FLAGS) -o $(WORKER_1) $(WORKER_1).c $(HEADER).c

worker_2:
	$(CC) $(FLAGS) -o $(WORKER_2) $(WORKER_2).c $(HEADER).c

clean:
	rm $(PROGRAM) $(WORKER_1) $(WORKER_2)

#CC = gcc
#FLAGS = -Wall -lpthread -lrt
#WORKER_1 = cookman
#WORKER_2 = deliveryman
#PROGRAM = main
#HEADER = utils
#
#test:
#	make worker_1
#	make worker_2
#	make program
#	./$(PROGRAM)
#
#program:
#	$(CC) $(PROGRAM).c -o $(PROGRAM) $(FLAGS)
#
#worker_1:
#	$(CC) $(WORKER_1).c -o $(WORKER_1) $(FLAGS)
#
#worker_2:
#	$(CC) $(WORKER_2).c -o $(WORKER_2) $(FLAGS)
#
#clean:
#	rm $(PROGRAM) $(WORKER_1) $(WORKER_2)