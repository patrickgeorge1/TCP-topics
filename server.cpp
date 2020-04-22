#include <iostream>
#include <cstring>
#include <cassert>
#include <map>
#include <vector>
#include <netinet/in.h>
#include "utils/utils.h"
#include "utils/string_utils.h"

using namespace std;

int main(int argc, char *argv[]) {
    DIE(argc < 2, "usage: ./server <PORT>");
    map<int, int> client_sockets;
    map<char *, vector <int>> client_topics;

    int sockfd;
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

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(sockfd < 0, "cannot create socket to listen");

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;


    return 0;
}
