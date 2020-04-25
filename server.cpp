#include <iostream>
#include <cstring>
#include <string>
#include <utility>
#include <map>
#include <vector>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "util/message.h"
#include "util/utils.h"
#include "util/string_utils.h"

using namespace std;

void send_offline_messages(string, int, map<string, vector <message>> &);
void subscribe_client(message &, map<string, vector <string>> &, map<pair<string, string>, bool> &);
void register_client(message &, int , struct sockaddr_in, map<string, int> &, map<string, bool> &, map<string, vector <struct message>> &);
void disconnect_client(message &, map<string, int> &, map<string, bool> &);
void unsubscribe_client(message &, map<string, vector <string>> &, map<pair<string, string>, bool> &);
void send_online_messages(message &, map<string, int> &, map<string, vector <string>>, map<string, bool> &, map<pair<string, string>, bool> &, struct sockaddr_in, map<string, vector <struct message>> &);


int main(int argc, char *argv[]) {
    DIE(argc < 2, "usage: ./server <PORT>");
    map<string, int> client_sockets = {};               // identify socket by client id
    map<string, vector <string>> client_topics = {};    // identify topics by client id
    map<string, vector <message>> client_messages = {};  // identify remaining messages by client id
    map<string, bool> client_active = {};               // identify status active by client id
    map<pair<string, string>, bool> client_sf = {};     // identify status sf by client id and topic

    int tcp_socket, udp_socket;
    char buffer[BUFLEN];
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    int port = atoi(argv[1]);
    DIE(port == 0, "port: atoi conversion failed or given port is 0");

    fd_set descriptors;
    fd_set tmp_descriptors;
    int descriptors_max;
    FD_ZERO(&descriptors);
    FD_ZERO(&tmp_descriptors);

    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    DIE(tcp_socket < 0, "cannot create tcp socket to listen");

    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    DIE(udp_socket < 0, "cannot create udp socket to listen");

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;



    int ret = bind(tcp_socket, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
    DIE(ret < 0, "cannot bind socket");

    ret = bind(udp_socket, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
    DIE(ret < 0, "cannot bind socket");



    ret = listen(tcp_socket, MAX_CLIENTS);
    DIE(ret < 0, "cannot listen at socket");

    FD_SET(0, &descriptors);
    FD_SET(udp_socket, &descriptors);
    descriptors_max = udp_socket;
    FD_SET(tcp_socket, &descriptors);
    descriptors_max = tcp_socket;


    do {
        tmp_descriptors = descriptors;
        ret = select(descriptors_max + 1, &tmp_descriptors, NULL, NULL, NULL);
        DIE(ret < 0, "cannot select at socket");

        // tcp connection request
        if (__FD_ISSET(tcp_socket, &tmp_descriptors)) {
            // accept
            clilen = sizeof(cli_addr);
            int new_tcp_client = accept(tcp_socket, (struct sockaddr *) &cli_addr, &clilen);
            DIE(new_tcp_client < 0, "cannot accept new tcp client");

            // add file descriptor
            __FD_SET(new_tcp_client, &descriptors);
            if (new_tcp_client > descriptors_max) descriptors_max = new_tcp_client;
            if(DEBUG_PRINT) cout << "accepted tcp request" << endl;
        }

        // tcp message from client arrived
        for (int i = 5; i <= descriptors_max; ++i) {
            if (__FD_ISSET(i, &tmp_descriptors)) {

                message message = {};
                int received_bytes = recv(i, &message, sizeof(message), 0);
                DIE(received_bytes < 0, "tcp message received got problems");
                string message_topic (message.topic);
                string message_id (message.id);

                switch ((int) message.type) {
                    case TYPE_REQUEST_CONNECTION:
                        // register client
                        register_client(message, i, cli_addr, client_sockets, client_active, client_messages);
                        // sent client's missed messages
                        send_offline_messages(message_id, client_sockets[message_id], client_messages);
                        break;

                    case TYPE_DISCONNECT:
                        __FD_CLR(i, &descriptors);       // remove client descriptor
                        // disconnect client
                        disconnect_client(message, client_sockets, client_active);
                        if(DEBUG_PRINT) cout << "client " << message.id << " status connection " << client_active[message_id] << endl;
                    break;

                    case TYPE_SUBSCRIBE:
                        subscribe_client(message, client_topics, client_sf);
                        break;

                    case TYPE_UNSUBSCRIBE:
                        unsubscribe_client(message, client_topics, client_sf);
                        break;
                }

            }
        }


        // udp message arrived
        if (__FD_ISSET(udp_socket, &tmp_descriptors)) {
            message message = {};
            int received_bytes = recvfrom(udp_socket, &message, sizeof(message), 0, (struct sockaddr*) &cli_addr, &clilen);
            DIE(received_bytes < 0, "udp message received got problems");
            string message_topic (message.topic);
            send_online_messages(message, client_sockets, client_topics, client_active, client_sf, cli_addr, client_messages);

            }

        // read from keyboard
        if (__FD_ISSET(0, &tmp_descriptors)) {
            memset(buffer, 0, BUFLEN);
            fgets(buffer, BUFLEN - 1, stdin);
            if (strncmp(buffer, "exit", 4) == 0) {
                message message = {};
                message.type = TYPE_DISCONNECT;
                for (int i = 5; i <= descriptors_max; ++i) {
                    ret = send(i, &message, sizeof(message), 0);
                    DIE(ret < 0, "cannot send shutdown to servers");
                }
                ret = shutdown(tcp_socket, SHUT_RDWR);
                DIE(ret < 0, "cannot shutdown tcp socket");
                break;
            }
        }


    } FOREVER;

    return 0;
}

void register_client(message &message, int socket, struct sockaddr_in cli_addr, map<string, int> &client_sockets, map<string, bool> &client_active, map<string, vector <struct message>> &client_messages) {
    string message_id(message.id);

    if (client_active.find(message_id) == client_active.end()) {
        printf("New client %s connected from %s:%d. \n", message_id.c_str(), inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

    } else {
        printf("client %s reconnected from %s:%d. \n", message_id.c_str(), inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
    }
    client_sockets[message_id] = socket;                             // register client socket
    client_active[message_id] = true;                                // make client active
}

void disconnect_client(message &message, map<string, int> &client_sockets, map<string, bool> &client_active) {
    string message_id(message.id);
    printf("Client %s disconnected .\n", message_id.c_str());

    if(DEBUG_PRINT) printf("Client %s disconnected %s\n", message_id.c_str(), message.id);
    client_active[message_id] = false;   // make client offline
    client_sockets.erase(message_id);    // remove client socket
}

void send_offline_messages(string id, int tcp_socket, map<string, vector <message>> &client_messages) {
        vector<message> messages = client_messages[id];
        if(DEBUG_PRINT) printf("client is here %s checks for unreached messages\n", id.c_str());
        // send messages
        for (message message : messages) {
            int ret = send(tcp_socket, &message, sizeof(message), 0);
            DIE(ret < 0, "cannot send topic message sf to user that reconnected");
            if(DEBUG_PRINT) printf("sent restant\n");
        }
        // empty vector
        vector<message> empty_messages;
        client_messages[id] = empty_messages;
}

void subscribe_client(message &message, map<string, vector <string>> &client_topics, map<pair<string, string>, bool> &client_sf) {
    string message_id(message.id);
    string message_topic(message.topic);

    if(DEBUG_PRINT) printf("client %s subscribed to %s\n", message_id.c_str(), message_topic.c_str());
    vector<string> subscribers;

    if (client_topics.find(message_topic) != client_topics.end()) {
        // topic exists so I take it
        subscribers = client_topics[message_topic];
    }
    client_sf[make_pair(message_topic, message_id)] = strncmp(message.message, "sf", 2) == 0;

    subscribers.push_back(message_id);
    client_topics[message_topic] = subscribers;
}

void unsubscribe_client(message &message, map<string, vector <string>> &client_topics, map<pair<string, string>, bool> &client_sf) {
    string message_id(message.id);
    string message_topic(message.topic);

    if(DEBUG_PRINT) printf("client %s unsubscribed to %s\n", message_id.c_str(), message_topic.c_str());

    vector<string> subscribers  = client_topics[message_topic];
    vector<string> remainig_subscribers;


    for (string subscriber : subscribers) {
        if (message_id != subscriber) {
            remainig_subscribers.push_back(subscriber);
        }
    }

    client_sf[make_pair(message_topic, message_id)] = false;
    client_topics[message_topic] = remainig_subscribers;
}

void send_online_messages(message &message, map<string, int> &client_sockets, map<string, vector <string>> client_topics, map<string, bool> &client_active, map<pair<string, string>, bool> &client_sf, struct sockaddr_in cli_addr, map<string, vector <struct message>> &client_messages){
    message.source = cli_addr;
    string message_topic(message.topic);
    if(DEBUG_PRINT) cout << "udp message arrived   topic :  " << message_topic.c_str() <<  " --> ready to deliver message " << message.message << endl;


    vector<string> subscribers = client_topics[message_topic];
    for (string subscriber : subscribers) {
        int tcp_socket = client_sockets[subscriber];
        if (client_active[subscriber] == true) {
            // subscriber is active
            int ret = send(tcp_socket, &message, sizeof(message), 0);
            if(DEBUG_PRINT) cout << "-- deliver to client" << subscriber.c_str() << " [active] " << subscriber << " <--> map " << client_active[subscriber] << " map keys " << client_active.size() <<  endl;
            DIE(ret < 0, "cannot send topic message to subscriber ");
        } else {

            if (client_sf[make_pair(message_topic, subscriber)]) {
                // subscriber is SF
                vector<struct message> offline_messages;
                if (client_messages.find(subscriber) != client_messages.end()) {
                    offline_messages = client_messages[subscriber];
                }
                offline_messages.push_back(message);
                client_messages[subscriber] = offline_messages;
                if(DEBUG_PRINT) cout << "-- deliver to client" << subscriber.c_str() << " [inactive] " << subscriber << " <--> map " << client_active[subscriber] << endl;
            }
        }
    }
}