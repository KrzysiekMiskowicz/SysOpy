CC = gcc
FLAGS = -Wall
SENDER = sender
CATCHER = catcher

all:
	make catcher
	make sender

catcher:
	$(CC) $(CATCHER).c -o $(CATCHER)

sender:
	$(CC) $(SENDER).c -o $(SENDER)

clean:
	rm -rf $(CATCHER) $(SENDER)