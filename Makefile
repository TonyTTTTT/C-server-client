SRC_DIR = ./src
OUT_DIR = ./out
UTIL_H = $(SRC_DIR)/util.h

all: server client

server: $(OUT_DIR)/server.o $(OUT_DIR)/word_occur.o
	g++ $^ -fopenmp -o ./$@

$(OUT_DIR)/server.o: $(SRC_DIR)/server.c $(UTIL_H)
	gcc $< -c -fopenmp -o $@

$(OUT_DIR)/word_occur.o: $(SRC_DIR)/word_occur.cpp $(UTIL_H)
	g++ $< -c -fopenmp -o $@


client: $(SRC_DIR)/client.c
	gcc $(SRC_DIR)/client.c -o ./client