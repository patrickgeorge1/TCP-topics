#include <iostream>
#include <cstring>
#include <map>
#include <vector>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "utils/message.h"
#include "utils/utils.h"
#include "utils/string_utils.h"

using namespace std;

void send_offline_messages(char *, int, map<char *, vector <message>> &, bool);
void subscribe_client(message , map<char *, vector <char *>> &);
void register_client(message , int , struct sockaddr_in, map<char *, int> &, map<char *, bool> &, map<char *, bool> &,  map<char *, vector <struct message>> &);
void disconnect_client(message , map<char *, int> &, map<char *, bool> &);
void unsubscribe_client(message, map<char *, vector <char *>> &);
void send_online_messages(message, map<char *, int> &, map<char *, vector <char *>> &, map<char *, bool> &, map<char *, bool> &, struct sockaddr_in, map<char *, vector <struct message>> &);


int main(int argc, char *argv[]) {
    DIE(argc < 2, "usage: ./server <PORT>");
    map<char *, int> client_sockets = {};               // identify socket by client id
    map<char *, vector <char *>> client_topics = {};    // identify topics by client id
    map<char *, vector <message>> client_messages = {};  // identify remaining messages by client id
    map<char *, bool> client_active = {};               // identify status active by client id
    map<char *, bool> client_sf = {};                   // identify status sf by client id

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
        }

        // tcp message from client arrived
        for (int i = 5; i <= descriptors_max; ++i) {
            if (__FD_ISSET(i, &tmp_descriptors)) {

                message message = {};
                int received_bytes = recv(i, &message, sizeof(message), 0);
                DIE(received_bytes < 0, "tcp message received got problems");

                switch ((int) message.type) {
                    case TYPE_REQUEST_CONNECTION:
                        // register client
                        register_client(message, i, cli_addr, client_sockets, client_active,client_sf, client_messages);
                        // sent client's missed messages
                        send_offline_messages(message.id, client_sockets[message.id], client_messages, client_sf[message.id]);
                        break;

                    case TYPE_DISCONNECT:
                        __FD_CLR(i, &descriptors);       // remove client descriptor
                        // disconnect client
                        disconnect_client(message, client_sockets, client_active);
                    break;

                    case TYPE_SUBSCRIBE:
                        subscribe_client(message, client_topics);
                        break;

                    case TYPE_UNSUBSCRIBE:
                        unsubscribe_client(message, client_topics);
                        break;
                }

            }
        }


        // udp message arrived
        if (__FD_ISSET(udp_socket, &tmp_descriptors)) {
            message message = {};
            int received_bytes = recvfrom(udp_socket, &message, sizeof(message), 0, (struct sockaddr*) &cli_addr, &clilen);
            DIE(received_bytes < 0, "udp message received got problems");

            send_online_messages(message, client_sockets, client_topics, client_active, client_sf, cli_addr, client_messages);
        }

        // read from keyboard
        if (__FD_ISSET(0, &tmp_descriptors)) {
            memset(buffer, 0, BUFLEN);
            fgets(buffer, BUFLEN - 1, stdin);
            if (strncmp(buffer, "exit", 4) == 0) {
                break;
            }
        }


    } FOREVER;

    return 0;
}

void register_client(message message, int socket, struct sockaddr_in cli_addr, map<char *, int> &client_sockets, map<char *, bool> &client_active, map<char *, bool> &client_sf,  map<char *, vector <struct message>> &client_messages) {
    if (client_active.find(message.id) == client_active.end()) printf("NEW client %s connected from %s:%d. \n", message.id, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
    else printf("client %s reconnected from %s:%d. \n", message.id, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
    client_sockets[message.id] = socket;                             // register client socket
    client_active[message.id] = true;                                // make client active
    client_sf[message.id] = strncmp(message.message, "sf", 2) == 0;  // register client sf status
}

void disconnect_client(message message, map<char *, int> &client_sockets, map<char *, bool> &client_active) {
    printf("client %s disconnected\n", message.id);
    client_active[message.id] = false;   // make client offline
    client_sockets.erase(message.id);    // remove client socket
}

void send_offline_messages(char *id, int tcp_socket, map<char *, vector <message>> &client_messages, bool sf) {
    if (sf) {
        vector<message> messages = client_messages[id];
        printf("client %s reconnected and checks for unreached messages\n", id);
        // send messages
        for (message message : messages) {
            int ret = send(tcp_socket, &message, sizeof(message), 0);
            DIE(ret < 0, "cannot send topic message sf to user that reconnected");
        }
        // empty vector
        vector<message> empty_messages;
        client_messages[id] = empty_messages;
    }
}

void subscribe_client(message message, map<char *, vector <char *>> &client_topics) {
    printf("client %s subscribed to %s\n", message.id, message.topic);
    vector<char *> subscribers;
    if (client_topics.find(message.topic) != client_topics.end()) {
        // topic exists so I take it
        subscribers = client_topics[message.topic];
    }
    subscribers.push_back(message.id);
    client_topics[message.topic] = subscribers;
}

void unsubscribe_client(message message, map<char *, vector <char *>> &client_topics) {
    printf("client %s unsubscribed to %s\n", message.id, message.topic);
    vector<char *> subscribers;
    subscribers = client_topics[message.topic];

    for (int i = 0; i < subscribers.size(); ++i) {
        if (strcmp(subscribers[i], message.id) == 0) {
            subscribers.erase (subscribers.begin()+i);
            break;
        }
    }
    client_topics[message.topic] = subscribers;
}

void send_online_messages(message message, map<char *, int> &client_sockets, map<char *, vector <char *>> &client_topics, map<char *, bool> &client_active, map<char *, bool> &client_sf, struct sockaddr_in cli_addr, map<char *, vector <struct message>> &client_messages) {
    message.source = cli_addr;
    char * topic = message.topic;
    vector<char *> subscribers = client_topics[topic];
    for (char * subscriber : subscribers) {
        int tcp_socket = client_sockets[subscriber];
        if (client_active[subscriber]) {
            // subscriber is active
            int ret = send(tcp_socket, &message, sizeof(message), 0);
            DIE(ret < 0, "cannot send topic message to subscriber ");
        } else {
            if (client_sf[subscriber]) {
                // subscriber is SF
                vector<struct message> offline_messages;
                if (client_messages.find(subscriber) != client_messages.end()) {
                    offline_messages = client_messages[subscriber];
                }
                offline_messages.push_back(message);
                client_messages[subscriber] = offline_messages;
            }
        }
    }
}