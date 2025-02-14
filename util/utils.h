//
// Created by patrickgeorge1 on 4/22/20.
//

#ifndef TCP_TOPICS_UTILS_H
#define TCP_TOPICS_UTILS_H

#define FOREVER while(1)
#define BUFLEN 256
#define MAX_CLIENTS 100
#define DEBUG_PRINT 0

void DIE(bool, std::string);
void send_connect_message(int, char *);
void send_disconnect_message(int, char *);
bool FD_IS_EMPTY(fd_set const *);
void translate_tcp_message(message);

#endif //TCP_TOPICS_UTILS_H
