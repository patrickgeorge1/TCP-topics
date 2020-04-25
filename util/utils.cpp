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
    char value_of_message[1500];

    switch (message.data_type) {
        case DATATYPE_INT:
            printf("int case \n");
            int value_int;

            value_int = ntohl(*(unsigned int *) (message.message + 1));
            if (message.message[0]) value_int = -value_int;

            sprintf(value_of_message, "%d", value_int);
            printf("%s:%d - %s - %s - %s \n", inet_ntoa(message.source.sin_addr), ntohs(message.source.sin_port), message.topic, DATATYPE_INT_STRING, value_of_message);
            break;

        case DATATYPE_SHORTREAL:
            printf("shortreal case \n");
            double value_double;

            value_double = ntohs(*(unsigned short int *) (message.message));

            sprintf(value_of_message, "%.2f", value_double / 100);
            printf("%s:%d - %s - %s - %s \n", inet_ntoa(message.source.sin_addr), ntohs(message.source.sin_port), message.topic, DATATYPE_SHORTREAL_STRING, value_of_message);
            break;

        case DATATYPE_FLOAT:
            printf("float case \n");
            float  value_float;

            value_float = ntohl(*(unsigned int *)(message.message + 1));
            value_float = value_float / pow(10, message.message[5]);
            if (message.message[0] == 1) value_float = -value_float;

            sprintf(value_of_message, "%lf", value_float);

            printf("%s:%d - %s - %s - %s \n", inet_ntoa(message.source.sin_addr), ntohs(message.source.sin_port), message.topic, DATATYPE_FLOAT_STRING, value_of_message);
            break;

        case DATATYPE_STRING:
            printf("string case \n");

            memcpy(value_of_message, message.message, strlen(message.message) + 1);
            printf("%s:%d - %s - %s - %s \n", inet_ntoa(message.source.sin_addr), ntohs(message.source.sin_port), message.topic, DATATYPE_STRING_STRING, value_of_message);
            break;
    }
}
