#include "User.h"

User::User(string uname, string pass, string r) {
    username = uname;
    password = pass;
    role = r;
}

void User::displayInfo() {
    cout << "Username: " << username << " Role: " << role << endl;
}
