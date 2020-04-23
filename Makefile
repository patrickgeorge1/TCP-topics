build:	clean
	g++ server.cpp ./utils/*.cpp -o server
	g++ subscriber.cpp ./utils/*.cpp -o subscriber
	g++ udp_client.cpp ./utils/*.cpp -o news
clean:
	rm -f server
	rm -f subscriber
	rm -f news
