//
// Created by patrickgeorge1 on 4/22/20.
//

#include <iostream>
#include <cstring>
#include <math.h>
#include <arpa/inet.h>
#include "message.h"
#include "utils.h"


using  namespace std;

void DIE(bool ok, string message) {
    if (ok) {
        cout << message << endl;
        exit(EXIT_FAILURE);
    }
}


void send_connect_message(int socket, char* id) {
    message message = {};
    strcpy(message.id, id);
    message.type = TYPE_REQUEST_CONNECTION;
    int n = send(socket, &message, sizeof(message), 0);
    DIE(n < 0, "cannot send registration message");
}

void send_disconnect_message(int socket, char * id) {
    cout << "Sent shutdownn" << endl;
    message message = {};
    message.type = TYPE_DISCONNECT;
    strcpy(message.id, id);
    int n = send(socket, &message, sizeof(message), 0);
    DIE(n < 0, "cannot send disconnection message");
}

bool FD_IS_EMPTY(fd_set const *fdset)
{
    static fd_set empty;     // initialized to 0 -> empty
    return memcmp(fdset, &empty, sizeof(fd_set)) == 0;
}
void translate_tcp_message(message message) {
//    printf("%s:%d - %s - %d - %s \n", inet_ntoa(message.source.sin_addr), ntohs(message.source.sin_port), message.topic, message.type, message.message);
    int value_int;
    double value_double;
    float  value_float;
    char * value_string;

    switch ((int) message.type) {
        case DATATYPE_INT:
            printf("int case \n");
            value_int = ntohl(*(unsigned int *) (message.message + 1));
            printf("before sign %d \n", value_int);
            if (message.message[0]) value_int = -value_int;
            printf("after sign %d \n", value_int);

            printf("%s:%d - %s - %s - %d \n", inet_ntoa(message.source.sin_addr), ntohs(message.source.sin_port), message.topic, DATATYPE_INT_STRING, value_int);
            break;

        case DATATYPE_SHORTREAL:
            value_double = ntohs(*(unsigned short int *) (message.message));
            printf("%s:%d - %s - %s - %.2f \n", inet_ntoa(message.source.sin_addr), ntohs(message.source.sin_port), message.topic, DATATYPE_SHORTREAL_STRING, value_double);
            break;

        case DATATYPE_FLOAT:
            value_float = ntohl(*(unsigned int *)(message.message + 1));
            value_float = value_float / pow(10, message.message[5]);
            if (message.message[0] == 1) value_float = -value_float;

            printf("%s:%d - %s - %s - %lf \n", inet_ntoa(message.source.sin_addr), ntohs(message.source.sin_port), message.topic, DATATYPE_FLOAT_STRING, value_float);
            break;

        case DATATYPE_STRING:
            memcpy(value_string, message.message, strlen(message.message) + 1);
            printf("%s:%d - %s - %s - %s \n", inet_ntoa(message.source.sin_addr), ntohs(message.source.sin_port), message.topic, DATATYPE_STRING_STRING, value_string);
            break;
    }
}
