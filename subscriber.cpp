//
// Created by patrickgeorge1 on 4/23/20.
//

#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "utils/message.h"
#include "utils/utils.h"
#include "utils/string_utils.h"



using  namespace std;

int parse_input(char *);

int main(int argc, char *argv[]) {
    DIE(argc < 4, "usage: ./subscriber <ID_Client> <IP_Server> <Port_Server>");

    char buffer[BUFLEN];
    struct sockaddr_in serv_addr;


    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    DIE(tcp_socket < 0, "cannot create socket");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[3]));
    int ret = inet_aton(argv[2], &serv_addr.sin_addr);
    DIE(ret == 0, "inet_aton conversion problem");

    ret = connect(tcp_socket, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    DIE(ret < 0, "cannot connect to TCP socket");

    fd_set descriptors;
    fd_set tmp_descriptors;
    int descriptors_max;
    FD_ZERO(&descriptors);
    FD_ZERO(&tmp_descriptors);

    FD_SET(tcp_socket, &descriptors);
    FD_SET(0, &descriptors);
    descriptors_max = tcp_socket;


    send_connect_message(tcp_socket, argv[1]);

    do {
        tmp_descriptors = descriptors;
        ret = select(descriptors_max + 1, &tmp_descriptors, NULL, NULL, NULL);
        DIE(ret < 0, "cannot select");

        // read from keyboard
        if (__FD_ISSET(0, &tmp_descriptors)) {
            memset(buffer, 0, BUFLEN);
            fgets(buffer, BUFLEN - 1, stdin);

            // TODO subscribe, unsubscribe

            if (strncmp(buffer, "exit", 4) == 0) {
                send_disconnect_message(tcp_socket);
                __FD_CLR(tcp_socket, &descriptors);

//                ret = close(tcp_socket);
//                DIE(ret < 0, "cannot close socket");
                ret = shutdown(tcp_socket, SHUT_RDWR);
                DIE(ret < 0, "cannot shutdown socket");
                break;
            }

            int command = parse_input(buffer);
            if (command < 0) {
                cout << "Invalid command, maybe you want to try <subscribe topic SF> or <unsubcribe topic>" << endl;
                continue;
            } else {

            }

        }

        // received tcp message
        if (__FD_ISSET(tcp_socket, &tmp_descriptors)) {
            message message = {};
            int received_bytes = recv(tcp_socket, &message, sizeof(message), 0);
            DIE(received_bytes < 0, "tcp message received got problems");

            if ((int) message.type == TYPE_DISCONNECT) {
                cout << "Server closed connection !" << endl;
                break;
            } else {
                // TODO format and print topic message

            }

        }

    } FOREVER;
}

int parse_input(char * message) {
    if (isSubscribe(message)) return TYPE_SUBSCRIBE;
    if (isUnsubscribe(message)) return TYPE_UNSUBSCRIBE;
    return -1;
}