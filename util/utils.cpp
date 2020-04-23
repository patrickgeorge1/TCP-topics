//
// Created by patrickgeorge1 on 4/22/20.
//

#include <iostream>
#include <cstring>
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

void send_disconnect_message(int socket) {
    cout << "Sent shutdownn" << endl;
    message message = {};
    message.type = TYPE_DISCONNECT;
    int n = send(socket, &message, sizeof(message), 0);
    DIE(n < 0, "cannot send disconnection message");
}

bool FD_IS_EMPTY(fd_set const *fdset)
{
    static fd_set empty;     // initialized to 0 -> empty
    return memcmp(fdset, &empty, sizeof(fd_set)) == 0;
}