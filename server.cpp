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

int main(int argc, char *argv[]) {
    DIE(argc < 2, "usage: ./server <PORT>");
    map<int, int> client_sockets;
    map<char *, vector <char *>> client_topics;

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
            clilen = sizeof(cli_addr);
            int new_tcp_client = accept(tcp_socket, (struct sockaddr *) &cli_addr, &clilen);
            DIE(new_tcp_client < 0, "cannot accept new tcp client");

            __FD_SET(new_tcp_client, &descriptors);
            if (new_tcp_client > descriptors_max) descriptors_max = new_tcp_client;

            // TODO move print to client registration
            printf("New client (CLIENT_ID) connected from %s:%d. \n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
        }

        // tcp message from client arrived
        for (int i = 5; i <= descriptors_max; ++i) {
            if (__FD_ISSET(i, &tmp_descriptors)) {
                // TODO check message type and process it

                message message = {};
                int received_bytes = recv(i, &message, sizeof(message), 0);
                DIE(received_bytes < 0, "tcp message received got problems");

                switch ((int) message.type) {
                    case TYPE_REQUEST_CONNECTION:
                        printf("New client %s connected from %s:%d. \n", message.id, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
                        // TODO log him in hashtables
                        break;
                    case TYPE_DISCONNECT:
                        __FD_CLR(i, &descriptors);
                        // TODO
                        cout << "Disconnected" << endl;
                        break;
                }
                cout << "[SERVER]: message received with type "  << (int) message.type << endl;
            }
        }

        // udp message arrived
        if (__FD_ISSET(udp_socket, &tmp_descriptors)) {
            // TODO send and/or store message
        }

        // read from keyboard
        if (__FD_ISSET(0, &tmp_descriptors)) {
            memset(buffer, 0, BUFLEN);
            fgets(buffer, BUFLEN - 1, stdin);
            if (strncmp(buffer, "exit", 4) == 0) {
                for (int i = 3; i <= descriptors_max; ++i) {
                    if (__FD_ISSET(i, &descriptors)) {
                        ret = shutdown(i, SHUT_RDWR);
                        __FD_CLR(i, &descriptors);
                        DIE(ret < 0, "Can t close a socket connection");
                    }
                }
                exit(EXIT_SUCCESS);
            }
        }


    } FOREVER;

    return 0;
}
