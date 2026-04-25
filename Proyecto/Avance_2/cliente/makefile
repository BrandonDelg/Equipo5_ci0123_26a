CXX = g++
STD = -std=c++20
WFLAGS = -Wall -Wextra
CXXFLAGS = $(STD) $(WFLAGS) -g -pthread

SRC = src
BUILD = build
BIN = bin

SERVER = $(BIN)/server.out
CLIENT = $(BIN)/client.out
INTERMEDIARIO = $(BIN)/intermediario.out

# 🔹 argumentos por defecto (por si no pasás nada)
ARGS ?= 127.0.0.1 0

DIRS = $(shell find -L $(SRC) -type d)
FSOURCE = $(wildcard $(DIRS:%=%/*.cpp))

SERVER_SRC = $(filter %ThreadMirrorServer.cpp %FileSystem.cpp %Socket.cpp %VSocket.cpp, $(FSOURCE))
CLIENT_SRC = $(filter %main.cpp %Client.cpp %Logger.cpp %Socket.cpp %VSocket.cpp, $(FSOURCE))
INTERMEDIARIO_SRC = $(filter %Intermediario.cpp %Socket.cpp %VSocket.cpp, $(FSOURCE))

SERVER_OBJ = $(SERVER_SRC:$(SRC)/%.cpp=$(BUILD)/%.o)
CLIENT_OBJ = $(CLIENT_SRC:$(SRC)/%.cpp=$(BUILD)/%.o)
INTERMEDIARIO_OBJ = $(INTERMEDIARIO_SRC:$(SRC)/%.cpp=$(BUILD)/%.o)

.PHONY: all server client intermediario run-server run-client run-intermediario clean

all: server client intermediario

server: $(SERVER)

$(SERVER): $(SERVER_OBJ) | $(BIN)
	$(CXX) $(CXXFLAGS) $^ -o $@

client: $(CLIENT)

$(CLIENT): $(CLIENT_OBJ) | $(BIN)
	$(CXX) $(CXXFLAGS) $^ -o $@

intermediario: $(INTERMEDIARIO)

$(INTERMEDIARIO): $(INTERMEDIARIO_OBJ) | $(BIN)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD)/%.o: $(SRC)/%.cpp | $(BUILD)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD):
	mkdir -p $(BUILD)

$(BIN):
	mkdir -p $(BIN)

# 🔥 RUN CON ARGUMENTOS
run-server:
	./$(SERVER) $(ARGS)

run-client:
	./$(CLIENT) $(ARGS)

run-intermediario:
	./$(INTERMEDIARIO) $(ARGS)

clean:
	rm -rf $(BUILD) $(BIN)