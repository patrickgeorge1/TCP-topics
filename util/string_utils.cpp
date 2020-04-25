//
// Created by patrickgeorge1 on 4/22/20.
//

#include "string.h"
#include "string_utils.h"

bool isSubscribe(const char *message) {
    char *s2 = strdup(message);
    char *iterator = strtok(s2, " |");
    return strncmp(iterator, "subscribe", 9) == 0;
}

bool isUnsubscribe(const char *message) {
    char *s2 = strdup(message);
    char *iterator = strtok(s2, " |");
    return strncmp(iterator, "unsubscribe", 11) == 0;
}

char* getTopic(const char *message) {
    char *s2 = strdup(message);
    char *iterator = strtok(s2, " ");
    iterator = strtok(NULL, " |");
    return iterator;
}

char * getSF(const char *message) {
    char *s2 = strdup(message);
    char *iterator = strtok(s2, " ");
    iterator = strtok(NULL, " |");
    iterator = strtok(NULL, " |");
    return iterator;
}