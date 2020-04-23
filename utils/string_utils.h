//
// Created by patrickgeorge1 on 4/22/20.
//

#ifndef TCP_TOPICS_STRING_UTILS_H
#define TCP_TOPICS_STRING_UTILS_H


bool isSubscribe(const char *message);
bool isUnsubscribe(const char *message);
char * getSF(const char *);
char* getTopic(const char *);

#endif //TCP_TOPICS_STRING_UTILS_H
