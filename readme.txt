
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  SOLVE  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

SERVER:
        (tools)
        map<string, int> client_sockets                  ---> each client: his socket
        map<string, bool> client_active                  ---> each client: is connected or not
        map<string, vector <message>> client_messages    ---> each client: his sf missed messages
        map<string, vector <string>> client_topics       ---> each topic: his subscribers
        map<pair<string, string>, bool> client_sf        ---> each <topic, client>: is sf or not

        (logic)
        1.) subscriber tries to connect, accept and wait his registration message

        2.) subscriber sent registration message, take his id, mark him as active
            in client_active, register his sockfd in client_sockets, and sent him
            all the messages from client_messages (sf missed messages)

        3.) subscriber sent subscription message, take topic and sf and assign
            him in client_topics and client_sf

        4.) subscriber send unsubscription message, take topic and remove him from
            client_topics and if case, client_sf

        5.) subscriber sent disconnect message, mark him as offline in
            client_active and clear his sockfd from client_sockets and from set

        6.) udp client sent a message, take topic, check for all the subscribers
            from that topic in client_topics and it they are active :
                    6.1) client = active                ==> send message
                    6.2) client = inactive, but is sf   ==> enqueue message
            using client_active and client_sf

        7.) user wrote "exit", send a message to each clients to annouce them that
            server is going to shutdown, close all connection and program



TCP_CLIENT:
        (tools)
        - when program start, connect and send a registration type message to
          server to annouce him about your identity (id), to be supervised

        (logic)
        1.) server sent message, check if message is type disconnect or udp_news:
                    1.1) message = type_disconnect     ==> close connection, exit
                    1.2) message = udp_news            ==> format and print news

        2.) user type "subscribe", send a subsbcribe type message to server

        3.) user type "unsubscribe", send a unsubsbcribe type message to server

        4.) user type "exit", annouce server about disconnection and close


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  PROBLEMS  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    1.) fgets mess up the null terminator  <==> put a '|' on the last character
        and parsed with strtok(char *, " |")

    2.) user "char *" as key for maps, sometimes map["topic"] = value1,
        sometimes map["topic"] = value2, even if key is the same <==> used
        "std::string" instead of "char *"

    3.) even if maps were objects, sending them as parameter to function did
        not reflect any change <==> used &

