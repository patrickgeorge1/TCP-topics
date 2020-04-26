//
// Created by patrickgeorge1 on 4/22/20.
//

#ifndef TCP_TOPICS_MESSAGE_H
#define TCP_TOPICS_MESSAGE_H

#include <netinet/in.h>

struct message {
    char topic[50];               // topic of client
    u_int8_t data_type;           // data type sent
    char message[1500];           // message sent

    unsigned char type;           // classify message type
    sockaddr_in source;           // source address and port
    char id[10];                  // id of client
} __attribute__((__packed__));

#define TYPE_REQUEST_CONNECTION 0
#define TYPE_SUBSCRIBE 1
#define TYPE_UNSUBSCRIBE 2
#define TYPE_DISCONNECT 4

#define DATATYPE_INT 0
#define DATATYPE_SHORTREAL 1
#define DATATYPE_FLOAT 2
#define DATATYPE_STRING 3

#define DATATYPE_INT_STRING "INT"
#define DATATYPE_SHORTREAL_STRING "SHORTREAL"
#define DATATYPE_FLOAT_STRING "FLOAT"
#define DATATYPE_STRING_STRING "STRING"

#endif //TCP_TOPICS_MESSAGE_H
