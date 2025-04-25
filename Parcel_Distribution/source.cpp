#include<iostream>
#include<string>
#include<Windows.h> // to have better control of the console window for eg to change color or move to a location on the console window
#include<conio.h> // to take key input like arrow keys 
#include<sstream> // for string to number conversion 
#include <fstream>
#include <iomanip> 
using namespace std;

string username, fileName, password;

void gotoxy(int x, int y);
bool checkUsername(const string& username);
bool startsWithLetters(const string& str);
bool registerUser(); // Sign UP 
bool loginUser(); // Login 

struct users {
	string username;
	string password;
	string role;
}userInfo;

// function to set color 
void color(int color) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}
// function to go to a location on the console 
void gotoxy(int x, int y) {
	COORD c;
	c.X = x;
	c.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

// welcome screen 
int welcome() {
	int Set[] = { 7,7,7,7 }; // color code for white 
	int counter = 1; // to keep track 
	char key; // for arrow key input 
	int log;
	cout << "\t\tParcel Distribution System\t\n\n\tAll your Parcels delivered to you safely and quickly\n\n";
	cout << "\tAlready have an account?\n\n\tLogin or register an account to continue.\n\n";
	for (int i = 0;;) {
		gotoxy(10, 8);
		color(Set[0]);
		cout << "1. Login";
		gotoxy(28, 8);
		color(Set[1]);
		cout << "\t 2. Register an account\n\t";
		gotoxy(10, 10);
		color(Set[2]);
		cout << "3. Exit";
		key = _getch();
		if (key == 75 && (counter == 2 || counter == 4)) { //72 up Arr key, 75 left key 
			counter--;
		}
		if (key == 77 && (counter == 1 )) { //80 down Arr key, 77 right key 
			counter++;
		}
		if (key == 72 && (counter == 3 || counter == 4)) { //72 up Arr key, 75 left key 
			counter--;
			counter--;
		}
		if (key == 80 && (counter == 1)) { //80 down Arr key, 77 right key 
			counter++;
			counter++;
		}
		if (key == '\r') { // '\z' carriage return OR Enter key 
			log = counter;
			color(7);
			return log;
			break;
		}
		// by default color white 
		Set[0] = 7;
		Set[1] = 7;
		Set[2] = 7;
		Set[3] = 7;
		if (counter == 1) {
			Set[0] = 12; // setting color to red 
		}
		if (counter == 2) {
			Set[1] = 12; // setting color to red 
		}
		if (counter == 3) {
			Set[2] = 12; // setting color to red 
		}
		if (counter == 4) {
			Set[3] = 12; // setting color to red 
		}
	}
}

// Menu 1 
bool menu1(int choose) {
	switch (choose) {
	case 2:
	{
		if (registerUser()) {
			if (loginUser()) {
				return true;
			}
		}
		return false;
		break;
	}
	case 1:
	{
		if (loginUser())
		{
			return true;
		}
		else {
			cout << "You cannot Log in" << endl;
			return false;
		}
		break;
	}
	case 3:
	{
		cout << "\n\tThank you for using Parcel Distribution system\n";
		return false;
		break;
	}
	default:
		cout << "\tInvalid Input" << endl;
		return false;
		break;
	}
}
bool registerUser() {
	bool check = false;
	bool validUsername = false;
	while (!validUsername) {
		cout << "\nEnter username: ";
		getline(cin, userInfo.username);
		if (startsWithLetters(userInfo.username)) {
			validUsername = true;
			if ("users.txt" != NULL) { // calls the check username function only if the file is not empty
				check = checkUsername(userInfo.username);
			}
			if (check) // if username already exists 
			{
				cout << "\tUsername already exists! register again with a new username." << endl;
				return false;
			}
			else // if username doesnt exist already 
			{
				fileName = userInfo.username + ".txt";
				cout << "\tEnter password: ";
				getline(cin, userInfo.password);
				cout << "\tEnter role (rider/customer): ";
				getline(cin, userInfo.role);
				while (userInfo.role != "rider" && userInfo.role != "customer") {
					cout << "\tInvalid role. Please enter 'rider' or 'customer': ";
					getline(cin, userInfo.role);
				}
			}
		}
	}
	fstream file("usernames.txt", ios::app); // this file is only storing the usernames not other info about users and is then used in check username function
	if (file.is_open())
	{
		file << userInfo.username << endl;
	}
	else {
		cout << "ERROR! Unable to open file (names)" << endl;
	}
	fstream outfile("users.txt", ios::app); // this file stores all the info about user 
	if (outfile.is_open())
	{
		outfile << userInfo.username << " " << userInfo.password << " " << userInfo.role << "\n";
		outfile.close();
		string fileName = userInfo.username + ".txt";
		cout << "Registration Successful" << endl;
		return true;
	}
	else {
		cout << "ERROR! Unable to open file" << endl;
		return false;
	}
}
bool checkUsername(const string& username) {
	fstream file("usernames.txt", ios::in | ios::out | ios::app);
	if (file.is_open()) {
		string line;
		while (getline(file, line)) {
			if (line == username) {
				file.close(); // Close the file before returning 
				return true;
			}
		}
		file.close(); // Close the file before returning 
		return false;
	}
	else {
		cout << "names File unable to open!" << endl;
		return false; // Return false indicating failure to open the file 
	}
}
// function to login 
bool loginUser()
{
	string u, p, name, password, f,role;
	double i;
	bool flag = 0;
	cout << "\n\tLogin Info:" << endl;
	cout << "\tEnter your username: ";
	getline(cin, name);
	cout << "\tEnter your password: ";
	cin >> password;
	fstream inFile("users.txt", ios::in); // Open file for reading 
	if (inFile.is_open()) {
		while (inFile >> u >> p >>role) {
			if (u == name && p == password) {
				inFile.close();
				fileName = name + ".txt";
				userInfo.username = u;
				userInfo.role = role;
				userInfo.password = p;
				cout << "\tLogin Successful." << endl;

				if (role == "customer") {
					cout << "\n\tWelcome, customer " << userInfo.username << "!" << endl;
					// add customer menu here
				}
				else if (role == "rider") {
					cout << "\n\tWelcome, rider " << userInfo.username << "!" << endl;
					// add rider menu here
				}

				return true;
			}
		}
		inFile.close();
		if (!flag) {
			cout << "\tLogin Unsuccessful, please provide correct credentials next time!" << endl;
			return false;
		}
	}
	else {
		cout << "\tError: Unable to open file for login." << endl;
		inFile.close();
		return false;
	}
}
// function implemented when user forgets password 

bool startsWithLetters(const string& str) {
	if (str.empty()) {
		cerr << "\tError: Empty string provided." << endl;
		return false;
	}
	// Check if the first character is an alphabet letter 
	if (isalpha(str[0])) {
		return true; // String starts with letters 
	}
	else {
		cerr << "\tError: String does not start with letters." << endl;
		return false;
	}
}


void main() {
	int log = welcome(); // user selects an option i.e Login / signup 
	menu1(log);
}