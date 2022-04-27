CC = gcc
FLAGS = -Wall
LIBRARY_NAME = lib_wc
SOURCE_NAME = library
PROGRAM_NAME = main

static:
	$(CC) $(FLAGS) -c $(SOURCE_NAME).c -o $(LIBRARY_NAME).o
	ar rcs $(LIBRARY_NAME).a $(LIBRARY_NAME).o
	$(CC) $(FLAGS) -c $(PROGRAM_NAME).c -o $(PROGRAM_NAME).o
	$(CC) $(FLAGS) -o $(PROGRAM_NAME) $(PROGRAM_NAME).o -L. $(LIBRARY_NAME).a

shared:
	$(CC) -fPIC $(FLAGS) -c $(SOURCE_NAME).c -o $(LIBRARY_NAME).o
	$(CC) -fPIC $(FLAGS) $(LIBRARY_NAME).o -shared -o $(LIBRARY_NAME).so
	$(CC) $(FLAGS) -c $(PROGRAM_NAME).c -o $(PROGRAM_NAME).o
	$(CC) $(FLAGS) -o $(PROGRAM_NAME) $(PROGRAM_NAME).o -L. $(LIBRARY_NAME).so

run:
	./$(PROGRAM_NAME)

clean:
	rm -f *.o
	rm main $(LIBRARY_NAME).a $(LIBRARY_NAME).so