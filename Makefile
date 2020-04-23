build:	clean
	g++ server.cpp ./utils/*.cpp -o server
	g++ subscriber.cpp ./utils/*.cpp -o subscriber

clean:
	rm -f server
	rm -f subscriber
