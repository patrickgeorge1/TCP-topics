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


int main(int argc, char *argv[]) {
    char buffer[BUFLEN];
    struct sockaddr_in serv_addr;

    int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    DIE(udp_socket < 0, "cannot create socket");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[1]));
    int ret = inet_aton("127.0.0.1", &serv_addr.sin_addr);
    DIE(ret < 0, "inet_aton conversion problem");

    message message = {};
    strcpy(message.topic, "me");
    message.type = DATATYPE_STRING;
    strcpy(message.message, "helllou");

    int bytes_sent = sendto(udp_socket, &message, sizeof(message), 0, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    DIE(bytes_sent < 0, " udp client can t sent");

    return 0;
}