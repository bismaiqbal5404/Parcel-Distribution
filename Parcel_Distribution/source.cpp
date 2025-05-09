#include<iostream>
#include<string>
#include<Windows.h> // to have better control of the console window for eg to change color or move to a location on the console window
#include<conio.h> // to take key input like arrow keys 
#include<sstream> // for string to number conversion 
#include <fstream>
#include<unordered_map>
#include<vector>
#include <iomanip> 
using namespace std;

string username, fileName, password;

void gotoxy(int x, int y);
void riderMenu();
void customerMenu();
bool checkUsername(const string& username);
bool startsWithLetters(const string& str);
bool registerUser(); // Sign UP 
bool loginUser(); // Login 
int welcome();
bool menu1(int choose);


struct users {
	string username;
	string password;
	string role;
}userInfo;


int main() {
	int log = welcome(); // user selects an option i.e Login / signup 
	menu1(log);
	return 0;
}

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
		cout << "1. Login ";
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
		if (key == 77 && (counter == 1)) { //80 down Arr key, 77 right key 
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
	string u, p, name, password, f, role;
	double i;
	bool flag = 0;
	cout << "\n\tLogin Info:" << endl;
	cout << "\tEnter your username: ";
	getline(cin, name);
	cout << "\tEnter your password: ";
	cin >> password;
	fstream inFile("users.txt", ios::in); // Open file for reading 
	if (inFile.is_open()) {
		while (inFile >> u >> p >> role) {
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
					customerMenu();
				}
				else if (role == "rider") {
					cout << "\n\tWelcome, rider " << userInfo.username << "!" << endl;
					riderMenu();
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
void riderMenu() {
	int choice;
	do {
		cout << "\n======= Rider Menu =======\n";
		cout << "1. View Assigned Orders\n";
		cout << "2. Start Delivery\n";
		cout << "3. Mark Order as Delivered\n";
		cout << "4. Logout\n";
		cout << "Enter your choice: ";
		cin >> choice;
		cin.ignore();

		switch (choice) {
		case 1:
			// viewAssignedOrders();
			break;
		case 2:
			//startDelivery();
			break;
		case 3:
			// markOrderDelivered();
			break;
		case 4:
			cout << "Logging out Rider...\n";
			break;
		default:
			cout << "Invalid choice. Try again.\n";
		}
	} while (choice != 4);
}

struct Order {
	string orderID;
	string customerName;
	string phoneNumber;
	string pickupZone;
	string pickupLocation;
	string dropoffZone;
	string dropoffLocation;
	double parcelWeight;
	string parcelType;
	int requiredMinutes;
	string status;
};

vector<Order> orders;
unordered_map<string, vector<string>> zones = {
	{"Gulshan e Hadeed", {"Gulshan Mor", "Bata Mor", "Phase 1 Market", "Sidhu Chowk", "Monday Bazar"}},
	{"Saddar", {"Empress Market", "Passport Office", "Zainab Market", "Governor House", "Regal Chowk"}},
	{"Federal B Area", {"Usman Memorial Hospital", "Karimabad", "Hussainabad", "Aisha Manzil", "Water pump chowrangi"}},
	{"Light House", {"Paper Market", "Karachi Metropolitan", "Judia Bazaar", "Boltan Market", "I.I Chundrigarh road"}},
	{"Gulshan-e-Iqbal", {"LuckyOne", "Imtiaz Supermarket", "Continental Bakery", "Practical Centre", "Sir Syed University"}}
};

unordered_map<string, unordered_map<string, double>> distances = {
	{"Gulshan Mor", {{"Bata Mor", 0.2}, {"Sidhu Chowk", 1.3}}},
	{"Bata Mor", {{"Gulshan Mor", 0.2}, {"Phase 1 Market", 0.5}}},
	{"Phase 1 Market", {{"Bata Mor", 0.5}, {"Sidhu Chowk", 0.3}}},
	{"Sidhu Chowk", {{"Gulshan Mor", 1.3}, {"Phase 1 Market", 0.3}}}
	// Add more distances as needed
};

void displayZones() {
	int idx = 1;
	for (auto& zone : zones) {
		cout << idx++ << ". " << zone.first << endl;
	}
}

string selectZone() {
	int choice;
	displayZones();
	cout << "Select a Zone: ";
	cin >> choice;
	cin.ignore();

	if (choice < 1 || choice > zones.size()) {
		cout << "Invalid Zone selected!\n";
		return "";
	}

	auto it = zones.begin();
	advance(it, choice - 1);
	return it->first;
}

string selectLocation(const string& zone) {
	if (zones.find(zone) == zones.end()) {
		cout << "Zone not found!\n";
		return "";
	}

	const vector<string>& locs = zones[zone];
	int idx = 1;
	for (auto& loc : locs) {
		cout << idx++ << ". " << loc << endl;
	}

	int choice;
	cout << "Select a Location: ";
	cin >> choice;
	cin.ignore();

	if (choice < 1 || choice > locs.size()) {
		cout << "Invalid Location selected!\n";
		return "";
	}

	return locs[choice - 1];
}

void generateInvoice(const Order& order) {
	cout << "\n--------------------------------------\n";
	cout << "         Parcel Delivery Invoice      \n";
	cout << "--------------------------------------\n";
	cout << "Order ID: " << order.orderID << endl;
	cout << "Customer Name: " << order.customerName << endl;
	cout << "Phone Number: " << order.phoneNumber << endl;
	cout << "Pickup: " << order.pickupLocation << " (" << order.pickupZone << ")\n";
	cout << "Drop-off: " << order.dropoffLocation << " (" << order.dropoffZone << ")\n";
	cout << "Parcel Weight: " << order.parcelWeight << " kg\n";
	cout << "Parcel Type: " << order.parcelType << endl;
	cout << "Required Delivery Time: " << order.requiredMinutes << " minutes\n";
	cout << "Order Status: " << order.status << endl;
	cout << "--------------------------------------\n";
}

void loadOrdersFromFile() {
	ifstream inFile("orders.txt");
	string line;
	while (getline(inFile, line)) {
		stringstream ss(line);
		Order o;
		string weight, minutes;

		getline(ss, o.orderID, ',');
		getline(ss, o.customerName, ',');
		getline(ss, o.phoneNumber, ',');
		getline(ss, o.pickupZone, ',');
		getline(ss, o.pickupLocation, ',');
		getline(ss, o.dropoffZone, ',');
		getline(ss, o.dropoffLocation, ',');
		getline(ss, weight, ',');
		getline(ss, o.parcelType, ',');
		getline(ss, minutes, ',');
		getline(ss, o.status, ',');

		o.parcelWeight = stod(weight);
		o.requiredMinutes = stoi(minutes);

		orders.push_back(o);
	}
}

void saveOrderToFile(const Order& order) {
	ofstream outFile("orders.txt", ios::app);
	if (outFile.is_open()) {
		outFile << order.orderID << ","
			<< order.customerName << ","
			<< order.phoneNumber << ","
			<< order.pickupZone << ","
			<< order.pickupLocation << ","
			<< order.dropoffZone << ","
			<< order.dropoffLocation << ","
			<< order.parcelWeight << ","
			<< order.parcelType << ","
			<< order.requiredMinutes << ","
			<< order.status << "\n";
		outFile.close();
	}
	else {
		cout << "Unable to open file to save the order.\n";
	}
}

void saveAllOrdersToFile() {
	ofstream outFile("orders.txt");
	if (outFile.is_open()) {
		for (const auto& order : orders) {
			outFile << order.orderID << ","
				<< order.customerName << ","
				<< order.phoneNumber << ","
				<< order.pickupZone << ","
				<< order.pickupLocation << ","
				<< order.dropoffZone << ","
				<< order.dropoffLocation << ","
				<< order.parcelWeight << ","
				<< order.parcelType << ","
				<< order.requiredMinutes << ","
				<< order.status << "\n";
		}
		outFile.close();
	}
	else {
		cout << "Unable to open file to save orders.\n";
	}
}

void placeNewOrder() {
	Order newOrder;
	newOrder.status = "Placed";

	cout << "Enter Your Name: ";

	getline(cin, newOrder.customerName);

	cout << "Enter Your Phone Number: ";
	getline(cin, newOrder.phoneNumber);

	cout << "\nSelect Pickup Zone:\n";
	newOrder.pickupZone = selectZone();
	if (newOrder.pickupZone.empty()) return;

	cout << "\nSelect Pickup Location:\n";
	newOrder.pickupLocation = selectLocation(newOrder.pickupZone);
	if (newOrder.pickupLocation.empty()) return;

	cout << "\nSelect Drop-off Zone:\n";
	newOrder.dropoffZone = selectZone();
	if (newOrder.dropoffZone.empty()) return;

	cout << "\nSelect Drop-off Location:\n";
	newOrder.dropoffLocation = selectLocation(newOrder.dropoffZone);
	if (newOrder.dropoffLocation.empty()) return;

	cout << "Enter Parcel Weight (kg): ";
	cin >> newOrder.parcelWeight;
	cin.ignore();

	cout << "Enter Parcel Type (Document, Electronics, Clothes): ";
	getline(cin, newOrder.parcelType);

	cout << "Enter your Required Delivery Time (in minutes): ";
	cin >> newOrder.requiredMinutes;
	cin.ignore();


	stringstream ss;
	ss << setw(2) << setfill('0') << orders.size() + 1;
	newOrder.orderID = ss.str();

	orders.push_back(newOrder);
	saveOrderToFile(newOrder);

	cout << "\nOrder placed successfully! Your Order ID is " << newOrder.orderID << "\n";
	generateInvoice(newOrder);
}

void trackOrder() {
	string id;
	cout << "Enter your Order ID: ";
	getline(cin, id);

	ifstream inFile("orders.txt");
	string line;
	bool orderFound = false;

	while (getline(inFile, line)) {
		stringstream ss(line);
		string orderID, customerName, phoneNumber, pickupZone, pickupLocation, dropoffZone, dropoffLocation, parcelWeight, parcelType, requiredMinutes, status;
		getline(ss, orderID, ',');
		getline(ss, customerName, ',');
		getline(ss, phoneNumber, ',');
		getline(ss, pickupZone, ',');
		getline(ss, pickupLocation, ',');
		getline(ss, dropoffZone, ',');
		getline(ss, dropoffLocation, ',');
		getline(ss, parcelWeight, ',');
		getline(ss, parcelType, ',');
		getline(ss, requiredMinutes, ',');
		getline(ss, status, ',');

		if (orderID == id) {
			cout << "\nOrder Status: " << status << endl;
			orderFound = true;
			break;
		}
	}

	if (!orderFound) {
		cout << "Order not found!\n";
	}
}

void cancelOrder() {
	string id;
	cout << "Enter your Order ID to Cancel: ";
	getline(cin, id);

	for (auto& order : orders) {
		if (order.orderID == id) {
			if (order.status == "Cancelled") {
				cout << "Order already cancelled!\n";
				return;
			}
			order.status = "Cancelled";
			cout << "Order cancelled successfully!\n";
			saveAllOrdersToFile();
			return;
		}
	}
	cout << "Order not found!\n";
}

void viewOrderHistory() {
	string name, phone;
	cout << "Enter Customer Name: ";
	getline(cin, name);
	cout << "Enter Phone Number: ";
	getline(cin, phone);

	bool found = false;
	for (const auto& order : orders) {
		if (order.customerName == name && order.phoneNumber == phone) {
			generateInvoice(order);
			found = true;
		}
	}

	if (!found) {
		cout << "No orders found for this customer.\n";
	}
}

void customerMenu() {
	int choice;

	loadOrdersFromFile();

	do {
		cout << "\nParcel Delivery System\n";
		cout << "1. Place New Order\n";
		cout << "2. Track Order\n";
		cout << "3. Cancel Order\n";
		cout << "4. View Order History\n";
		cout << "5. Exit\n";
		cout << "Enter your choice: ";
		cin >> choice;
		cin.ignore();

		switch (choice) {
		case 1: placeNewOrder(); break;
		case 2: trackOrder(); break;
		case 3: cancelOrder(); break;
		case 4: viewOrderHistory(); break;
		case 5: break;
		default: cout << "Invalid choice. Please try again.\n"; break;
		}
	} while (choice != 5);

}