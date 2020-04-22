//
// Created by patrickgeorge1 on 4/22/20.
//
#include "string.h"
#include "string_utils.h"

bool isSubscribe(char *message) {
    char *iterator = strtok(message, " ");
    return strcmp(iterator, "subscribe") == 0;
}

bool isUnsubscribe(char *message) {
    char *iterator = strtok(message, " ");
    return strcmp(iterator, "unsubscribe") == 0;
}

