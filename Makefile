build:	clean
	g++ server.cpp ./util/*.cpp -o server
	g++ subscriber.cpp ./util/*.cpp -o subscriber
	g++ udp_client.cpp ./util/*.cpp -o news
clean:
	rm -f server
	rm -f subscriber
	rm -f news

udp-three:
	python3 udp_client.py --source-port 1234 --input_file three_topics_payloads.json --mode random --delay 2000 127.0.0.1 8080

udp-simple:
	python3 udp_client.py --mode manual 127.0.0.1 8080


