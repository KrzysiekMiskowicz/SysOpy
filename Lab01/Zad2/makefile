CC = gcc
FLAGS = -Wall
LIBRARY_PATH = ../Zad1
LIBRARY_NAME = lib_wc
SOURCE_NAME = library
PROGRAM_NAME = main

#static:
#	$(CC) $(FLAGS) -c $(LIBRARY_PATH)/$(SOURCE_NAME).c -o $(LIBRARY_NAME).o
#	ar rcs $(LIBRARY_NAME).a $(LIBRARY_NAME).o
#	$(CC) $(FLAGS) $(PROGRAM_NAME).c -o $(PROGRAM_NAME) -I$(LIBRARY_PATH) $(LIBRARY_NAME).a -L. $(LIBRARY_NAME).a

static:
	$(CC) $(FLAGS) -c $(LIBRARY_PATH)/$(SOURCE_NAME).c -o $(LIBRARY_PATH)/$(LIBRARY_NAME).o
	ar rcs $(LIBRARY_PATH)/$(LIBRARY_NAME).a $(LIBRARY_PATH)/$(LIBRARY_NAME).o
	$(CC) $(FLAGS) $(PROGRAM_NAME).c -o $(PROGRAM_NAME) -I$(LIBRARY_PATH) $(LIBRARY_PATH)/$(LIBRARY_NAME).a -L. $(LIBRARY_PATH)/$(LIBRARY_NAME).a

run:
	./$(PROGRAM_NAME) . $(shell cat args.txt)

clean:
	rm -f main main.o raport.txt tmp.txt
	cd $(LIBRARY_PATH); rm -f *.o *.a
