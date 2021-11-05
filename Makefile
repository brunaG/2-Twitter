FLAGS = -pthread -Wall -g -std=c++11
SRC := $(wildcard Server/*.cpp main_server.cpp main_client.cpp Client/*.cpp Utils/*.cpp)
CLIENT := $(wildcard main_client.cpp Client/*.cpp Utils/*.cpp)
SERVER := $(wildcard main_server.cpp Server/*.cpp Server/*.hpp Utils/*.cpp)
all: $(SRC)
	g++ $(FLAGS) $(SERVER) -o server
	g++ $(FLAGS) $(CLIENT) -o client

server: $(SERVER)
	g++ $(FLAGS) main_server.cpp Server/*.cpp -o server

client: $(CLIENT)
	g++ $(FLAGS) main_client.cpp Client/*.cpp Utils/*.cpp -o client

run_replicas: $(SERVER)
	./server 1 127.0.0.1 primario
	sleep 1.5

	./server 2 127.0.0.2 replica
	sleep 1.5

	./server 3 127.0.0.3 replica
	sleep 1.5

	./server 4 127.0.0.4 replica
	sleep 1.5
