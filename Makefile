.PHONY:clean ALL

CC=gcc
CPPFLAGS=-I./include
CFLAGS=-Wall
LIBS=-pthread

TEST_PATH=test
SOURCE_PATH=source

main=main

$(main):$(TEST_PATH)/main.o \
	$(SOURCE_PATH)/threadpool.o \
	$(SOURCE_PATH)/condition.o
	$(CC) $^ $(LIBS) -o $@

%.o:%.c
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS)

clean:
	-rm -rf *.o $(main) $(TEST_PATH)/*.o $(SOURCE_PATH)/*.o
