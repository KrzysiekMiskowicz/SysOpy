CC = gcc
FLAGS = -Wall -lpthread -lrt
PROGRAM = main
IMAGE = mona_lisa.ascii.pgm
OUTPUT = output.pgm
REPORT = Times.txt

test:
	make clean
	$(CC) $(FLAGS) -o $(PROGRAM) $(PROGRAM).c
	make test_value
	make test_block

test_value:
	./$(PROGRAM) 1 value $(IMAGE) $(OUTPUT)
	./$(PROGRAM) 2 value $(IMAGE) $(OUTPUT)
	./$(PROGRAM) 4 value $(IMAGE) $(OUTPUT)
	./$(PROGRAM) 8 value $(IMAGE) $(OUTPUT)
	./$(PROGRAM) 16 value $(IMAGE) $(OUTPUT)

test_block:
	./$(PROGRAM) 1 block $(IMAGE) $(OUTPUT)
	./$(PROGRAM) 2 block $(IMAGE) $(OUTPUT)
	./$(PROGRAM) 4 block $(IMAGE) $(OUTPUT)
	./$(PROGRAM) 8 block $(IMAGE) $(OUTPUT)
	./$(PROGRAM) 16 block $(IMAGE) $(OUTPUT)

clean:
	rm -f $(PROGRAM) $(OUTPUT) $(REPORT)