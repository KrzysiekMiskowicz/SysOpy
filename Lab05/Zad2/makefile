CC = gcc
FLAGS = -Wall
PROGRAM_NAME = main
TEST_FILE_PATH = ./test.txt
ONE_ARG_TYPE_1 = nadawca
ONE_ARG_TYPE_2 = data
ADDRESS =  krzysiekmiskowicz
SUBJECT = test
MSG = wiadomość_testowa

test:
	$(CC) $(FLAGS) $(PROGRAM_NAME).c -o $(PROGRAM_NAME)
	./$(PROGRAM_NAME) $(ONE_ARG_TYPE_1)
	./$(PROGRAM_NAME) $(ONE_ARG_TYPE_2)
	./$(PROGRAM_NAME) $(ADDRESS) $(SUBJECT) $(MSG)

clean:
	rm $(PROGRAM_NAME)