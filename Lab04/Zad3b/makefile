CC = gcc
FLAGS = -Wall
LIB_PATH = lib
LIB_NAME = lib
SENDER = sender
CATCHER = catcher

all:
	make catcher
	make sender

lib:
	$(CC) -fPIC $(FLAGS) -c $(LIB_PATH).c -o $(LIB_NAME).o
	$(CC) -fPIC $(FLAGS) $(LIB_NAME).o -shared -o $(LIB_NAME).so

catcher:
	make lib
	$(CC) $(FLAGS) -c $(CATCHER).c -o $(CATCHER).o
	$(CC) $(FLAGS) -o $(CATCHER) $(CATCHER).o -L. $(LIB_NAME).so

sender:
	make lib
	$(CC) $(FLAGS) -c $(SENDER).c -o $(SENDER).o
	$(CC) $(FLAGS) -o $(SENDER) $(SENDER).o -L. $(LIB_NAME).so

clean:
	rm -rf $(CATCHER) $(SENDER) $(LIB_NAME).so

clean_all:
	make clean
	rm -rf $(CATCHER).[^c] $(SENDER).[^c] $(LIB_NAME).[^ch]