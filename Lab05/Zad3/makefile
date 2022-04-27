CC = gcc
FLAGS = -Wall
PROGRAM_NAME = main
PRODUCER_NAME = producer
CONSUMER_NAME = consumer

test:
	make producer
	make consumer
	make main
	./$(PROGRAM_NAME)
	make clean

main:
	$(CC) $(FLAGS) $(PROGRAM_NAME).c -o $(PROGRAM_NAME)

producer:
	$(CC) $(FLAGS) $(PRODUCER_NAME).c -o $(PRODUCER_NAME)

consumer:
	$(CC) $(FLAGS) $(CONSUMER_NAME).c -o $(CONSUMER_NAME)

clean:
	rm $(PROGRAM_NAME) $(PRODUCER_NAME) $(CONSUMER_NAME)