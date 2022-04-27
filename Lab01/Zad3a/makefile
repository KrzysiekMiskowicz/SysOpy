CC = gcc
FLAGS = -Wall
LIBRARY_PATH = ../Zad1
LIBRARY_NAME = lib_wc
SOURCE_NAME = library
PROGRAM_PATH = ../Zad2
PROGRAM_NAME = main
CURRENT_DIR_PATH = ../Zad3a
DYNAMIC_LIB_MACRO = DYNAMIC_LINKING
RAPORT_NAME = raport.txt

static:
	$(CC) $(FLAGS) -c $(LIBRARY_PATH)/$(SOURCE_NAME).c -o $(LIBRARY_PATH)/$(LIBRARY_NAME).o
	ar rcs $(LIBRARY_PATH)/$(LIBRARY_NAME).a $(LIBRARY_PATH)/$(LIBRARY_NAME).o
	$(CC) $(FLAGS) $(PROGRAM_PATH)/$(PROGRAM_NAME).c -o $(PROGRAM_PATH)/$(PROGRAM_NAME) -I$(LIBRARY_PATH) $(LIBRARY_PATH)/$(LIBRARY_NAME).a -L. $(LIBRARY_PATH)/$(LIBRARY_NAME).a

shared:
	$(CC) -fPIC $(FLAGS) -c $(LIBRARY_PATH)/$(SOURCE_NAME).c -o $(LIBRARY_PATH)/$(LIBRARY_NAME).o
	$(CC) -fPIC $(FLAGS) $(LIBRARY_PATH)/$(LIBRARY_NAME).o -shared -o $(LIBRARY_PATH)/$(LIBRARY_NAME).so
	$(CC) $(FLAGS) $(PROGRAM_PATH)/$(PROGRAM_NAME).c -o $(PROGRAM_PATH)/$(PROGRAM_NAME) -I$(LIBRARY_PATH) -L$(LIBRARY_PATH) $(LIBRARY_PATH)/$(LIBRARY_NAME).so

dynamic:
	$(CC) -fPIC $(FLAGS) -c $(LIBRARY_PATH)/$(SOURCE_NAME).c -o $(LIBRARY_PATH)/$(LIBRARY_NAME).o
	$(CC) -fPIC $(FLAGS) $(LIBRARY_PATH)/$(LIBRARY_NAME).o -shared -o $(LIBRARY_PATH)/$(LIBRARY_NAME).so
	$(CC) $(FLAGS) $(PROGRAM_PATH)/$(PROGRAM_NAME).c -o $(PROGRAM_PATH)/$(PROGRAM_NAME) -I$(LIBRARY_PATH) -ldl -D $(DYNAMIC_LIB_MACRO)

test:
	cd $(PROGRAM_PATH); ./$(PROGRAM_NAME) $(CURRENT_DIR_PATH) $(shell cat $(PROGRAM_PATH)/args.txt)

test_all:
	echo "Static Library:" > $(RAPORT_NAME)
	make static
	make test
	echo "\nShared Library:" >> $(RAPORT_NAME)
	make shared
	make test
	echo "\nDynamic Library:" >> $(RAPORT_NAME)
	make dynamic
	make test

clean:
	cd $(PROGRAM_PATH); rm -f main main.o tmp.txt
	cd $(LIBRARY_PATH); rm -f *.o *.a
	rm $(RAPORT_NAME)
