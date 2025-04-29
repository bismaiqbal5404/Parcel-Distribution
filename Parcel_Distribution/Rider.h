#pragma once
#include "User.h"  // because Rider inherits User class
#include <iostream>
using namespace std;

class Rider : public User {
public:
    Rider(string uname, string pass, string r);
    void riderMenu();
    void startDelivery();
};
