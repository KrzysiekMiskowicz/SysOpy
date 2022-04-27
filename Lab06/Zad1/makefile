CC = gcc
FLAGS = -Wall
SERVER_NAME = server
CLIENT_NAME = client
HEADER = utils
LOG_FILE = log.txt

test:
	make server
	make client

server:
	$(CC) $(FLAGS) $(SERVER_NAME).c $(HEADER).c -o $(SERVER_NAME)

client:
	$(CC) $(FLAGS) $(CLIENT_NAME).c $(HEADER).c -o $(CLIENT_NAME)

clean:
	rm $(SERVER_NAME) $(CLIENT_NAME) $(LOG_FILE)