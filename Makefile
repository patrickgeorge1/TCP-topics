build:	clean
	g++ server.cpp ./utils/*.cpp -o server

clean:
	rm -f server
