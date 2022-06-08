CC = gcc
FLAGS = -Wall -lpthread -lrt
PROGRAM = main
NAME1 = Krzysiek
NAME2 = Marek
SERVER_ADDRESS = socket
SERVER_PORT = 5000


all:
	make clean
	make server
	make client
	./server $(SERVER_PORT) $(SERVER_ADDRESS)

client:
	$(CC) $(FLAGS) -o client client.c

server:
	$(CC) $(FLAGS) -o server server.c


client1_local:
	./client $(NAME1) local $(SERVER_ADDRESS)

client2_local:
	./client $(NAME2) local $(SERVER_ADDRESS)

client1_network:
	./client $(NAME1) network $(SERVER_PORT)

client2_network:
	./client $(NAME2) network $(SERVER_PORT)

clean:
	rm -f $(obj) client server $(SERVER_ADDRESS)