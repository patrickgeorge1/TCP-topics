//
// Created by patrickgeorge1 on 4/22/20.
//

#include <iostream>
#include <cstring>
#include <math.h>
#include <arpa/inet.h>
#include "message.h"


using  namespace std;

void DIE(bool ok, string message) {
    // defensive programming
    if (ok) {
        cout << message << endl;
        exit(EXIT_FAILURE);
    }
}


void send_connect_message(int socket, char* id) {
    // subscriber send it's id to server to register
    message message = {};
    strcpy(message.id, id);
    message.type = TYPE_REQUEST_CONNECTION;
    int n = send(socket, &message, sizeof(message), 0);
    DIE(n < 0, "cannot send registration message");
}

void send_disconnect_message(int socket, char * id) {
    // subscriber annouce server about his exit
    message message = {};
    message.type = TYPE_DISCONNECT;
    strcpy(message.id, id);
    int n = send(socket, &message, sizeof(message), 0);
    DIE(n < 0, "cannot send disconnection message");
}

bool FD_IS_EMPTY(fd_set const *fdset) {
    // check if fd_set is empty
    static fd_set empty;     // initialized to 0 -> empty
    return memcmp(fdset, &empty, sizeof(fd_set)) == 0;
}

void translate_tcp_message(message message) {
    // format and prind message from udp
    char value_of_message[1500];

    switch (message.data_type) {
        case DATATYPE_INT:
            int value_int;
            value_int = ntohl(*(unsigned int *) (message.message + 1));
            if (message.message[0]) value_int = -value_int;

            sprintf(value_of_message, "%d", value_int);
            printf("%s:%d - %s - %s - %s \n", inet_ntoa(message.source.sin_addr), ntohs(message.source.sin_port), message.topic, DATATYPE_INT_STRING, value_of_message);
            break;

        case DATATYPE_SHORTREAL:
            double value_double;
            value_double = ntohs(*(unsigned short int *) (message.message));

            sprintf(value_of_message, "%.2f", value_double / 100);
            printf("%s:%d - %s - %s - %s \n", inet_ntoa(message.source.sin_addr), ntohs(message.source.sin_port), message.topic, DATATYPE_SHORTREAL_STRING, value_of_message);
            break;

        case DATATYPE_FLOAT:
            float  value_float;

            value_float = ntohl(*(unsigned int *)(message.message + 1));
            value_float = value_float / pow(10, message.message[5]);
            if (message.message[0] == 1) value_float = -value_float;

            sprintf(value_of_message, "%lf", value_float);
            printf("%s:%d - %s - %s - %s \n", inet_ntoa(message.source.sin_addr), ntohs(message.source.sin_port), message.topic, DATATYPE_FLOAT_STRING, value_of_message);
            break;

        case DATATYPE_STRING:
            memcpy(value_of_message, message.message, strlen(message.message) + 1);

            printf("%s:%d - %s - %s - %s \n", inet_ntoa(message.source.sin_addr), ntohs(message.source.sin_port), message.topic, DATATYPE_STRING_STRING, value_of_message);
            break;
    }
}
