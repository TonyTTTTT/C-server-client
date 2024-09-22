SRC_DIR = ./src
OUT_DIR = ./out
UTIL_H = $(SRC_DIR)/util.h

all: server client

server: $(OUT_DIR)/server.o $(OUT_DIR)/word_occur.o $(OUT_DIR)/util.o
	g++ $^ -fopenmp -o ./$@

$(OUT_DIR)/server.o: $(SRC_DIR)/server.cpp $(UTIL_H)
	g++ $< -c -fopenmp -o $@

$(OUT_DIR)/word_occur.o: $(SRC_DIR)/word_occur.cpp $(UTIL_H)
	g++ $< -c -fopenmp -o $@

$(OUT_DIR)/util.o: $(SRC_DIR)/util.cpp $(UTIL_H)
	g++ $< -c -o $@


client: $(SRC_DIR)/client.cpp $(OUT_DIR)/util.o $(UTIL_H)
	g++ $^ -o ./$@