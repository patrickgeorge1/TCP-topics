//
// Created by patrickgeorge1 on 4/22/20.
//

#include <iostream>
#include <string>
#include "utils.h"


using  namespace std;

void DIE(bool ok, string message) {
    if (ok) {
        cout << message << endl;
        exit(EXIT_FAILURE);
    }
}