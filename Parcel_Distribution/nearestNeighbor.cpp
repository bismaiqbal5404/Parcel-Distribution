#include<iostream>
#include<string>
#include<Windows.h> // to have better control of the console window for eg to change color or move to a location on the console window
#include<conio.h> // to take key input like arrow keys 
#include<sstream> // for string to number conversion 
#include <fstream>
#include<unordered_map>
#include<vector>
#include <iomanip> 
#include<cctype>
#include<algorithm>
#include<float.h>
using namespace std;

struct Node {
	string code;
	string areaName;
	vector<pair<Node*, double>> neighbors;
};

struct Zone {
	string id;
	string name;
	vector<Node*> nodes;
};

struct Graph {
	unordered_map<string, Node*> nodesByCode;
	unordered_map<string, Zone*> zonesById;
};
Graph G;
unordered_map<string, pair<unordered_map<Node*, double>, unordered_map<Node*, Node*>>> allShortestPaths;

struct users {
	string username;
	string password;
	string role;
} userInfo;

struct Order {
	string orderID;
	string customerName;
	string phoneNumber;
	string pickupZoneCode;
	string pickupNodeCode;
	string dropoffZoneCode;
	string dropoffNodeCode;
	float parcelWeight;
	string parcelType;
	string status;
	bool hasHardDeadline;
	double deadlineTime;
};
vector<Order> orders;

string username, fileName, password;
string driverLoc = "A1";
int T_pickup = 10;
int T_dropoff = 7;
// assuming the rider starts at 10 AM for delivery
// time is in minutes after midnight
int currentTime = (10 * 60);
double avgSpeed = 40.0; // speed in km/h

// function declarations
void gotoxy(int x, int y);
void riderMenu();
void customerMenu();
bool checkUsername(const string& username);
bool startsWithLetters(const string& str);
bool registerUser(); // Sign UP 
bool loginUser(); // Login 
int welcome();
bool menu1(int choose);
void startDelivery(string loc, double capacity);
void viewAssignedOrders();
void loadOrdersFromFile();

class MinHeap {
	pair<double, Node*>* harr;
	int capacity;
public:
	int heap_size;
	unordered_map<Node*, int> nodePos; // maps Node* to its index in harr
	//Constructor
	MinHeap(int capacity);

	//to heapify a subtree with the root at given index
	void MinHeapify(int i);

	int parent(int i) {
		return (i - 1) / 2;
	}

	int left(int i) {
		return (2 * i + 1);
	}

	int right(int i) {
		return (2 * i + 2);
	}

	pair<double, Node*> extractMin();

	void decreaseKey(int i, pair<double, Node*> new_val);

	pair<double, Node*> getMin() {
		return harr[0];
	}

	void deleteKey(int i);

	void insertKey(pair<double, Node*> k);

	bool isEmpty() const {
		return heap_size == 0;
	}

private:
	void heapSwap(int i, int j); // helper for swapping with position tracking
};

MinHeap::MinHeap(int cap) {
	heap_size = 0;
	capacity = cap;
	harr = new pair<double, Node*>[cap];
}

// swapping node positions with swapping in heap as well
void MinHeap::heapSwap(int i, int j) {
	swap(harr[i], harr[j]);
	nodePos[harr[i].second] = i;
	nodePos[harr[j].second] = j;
}

// inserts a new key k
void MinHeap::insertKey(pair<double, Node*> k) {
	if (heap_size == capacity) {
		cout << "\nOverflow: Could not insertKey\n";
		return;
	}

	//first insert the new key at the end
	int i = heap_size++;
	harr[i] = k;
	nodePos[k.second] = i;

	// Fix the min heap property if is is violated
	while (i != 0 && harr[parent(i)].first > harr[i].first) {
		heapSwap(i, parent(i));
		i = parent(i);
	}
}

//Decreases value of key at index 'i' to new_val. It is assumed that
// new_val is smaller harr[i]
void MinHeap::decreaseKey(int i, pair<double, Node*> new_val) {
	harr[i] = new_val;
	nodePos[new_val.second] = i;
	while (i != 0 && harr[parent(i)].first > harr[i].first) {
		heapSwap(i, parent(i));
		i = parent(i);
	}
}

// Method to remove minimum element (or root) from min heap
pair<double, Node*> MinHeap::extractMin() {
	if (heap_size <= 0)
		return { DBL_MAX, nullptr };
	if (heap_size == 1) {
		heap_size--;
		nodePos.erase(harr[0].second);
		return harr[0];
	}

	// store the minimum value, and remove it from heap
	pair<double, Node*> root = harr[0];
	harr[0] = harr[heap_size - 1];
	nodePos[harr[0].second] = 0;
	heap_size--;
	MinHeapify(0);
	nodePos.erase(root.second);

	return root;
}

// This function deletes key at index i. It first reduced value to minus
// infinity then calls extract min()
void MinHeap::deleteKey(int i) {
	decreaseKey(i, { DBL_MIN, harr[i].second });
	extractMin();
}

// A recursive method to heapify a subtree with the root at given index
// This method assumes that the subtrees are already heapified
void MinHeap::MinHeapify(int i) {
	int l = left(i);
	int r = right(i);
	int smallest = i;
	if (l < heap_size && harr[l].first < harr[i].first)
		smallest = l;
	if (r < heap_size && harr[r].first < harr[smallest].first)
		smallest = r;
	if (smallest != i) {
		heapSwap(i, smallest);
		MinHeapify(smallest);
	}
}

static string unquote(const string& s) {
	if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
		return s.substr(1, s.size() - 2);
	return s;
}

static string trim(const string& s) {
	int start = 0;
	int end = static_cast<int>(s.size()) - 1;
	while (start <= end && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r' || s[start] == '\n')) {
		start++;
	}
	while (end >= start && (s[end] == ' ' || s[end] == '\t' || s[end] == '\r' || s[end] == '\n')) {
		end--;
	}
	return (start > end ? string() : s.substr(start, end - start + 1));
}

void loadGraphFromFile(const string& path, Graph& G) {
	ifstream in(path);
	if (!in.is_open()) {
		cout << "Error: Could not open file " << path << endl;
		return;
	}

	string line, currentZoneId;

	while (getline(in, line)) {
		line = trim(line);
		if (line.empty() || line[0] == '#') continue;
		istringstream ss(line);
		string kw;
		ss >> kw;
		if (kw == "ZONE") {
			ss >> currentZoneId;
			string rest;
			getline(ss, rest);
			rest = trim(rest);
			Zone* z = new Zone{ currentZoneId, unquote(rest), {} };
			G.zonesById[currentZoneId] = z;
		}
		else if (kw == "NODE") {
			string code;
			ss >> code;
			string rest;
			getline(ss, rest);
			rest = trim(rest);
			Node* n = new Node{ code, unquote(rest), {} };
			G.nodesByCode[code] = n;
			auto zit = G.zonesById.find(currentZoneId);
			if (zit == G.zonesById.end()) {
				cerr << "Warning: NODE '" << code << "' declared before any ZONE\n";
			}
			else {
				zit->second->nodes.push_back(n);
			}
		}
		else if (kw == "EDGE") {
			std::string a, b;
			double d;
			ss >> a >> b >> d;
			auto it1 = G.nodesByCode.find(a);
			auto it2 = G.nodesByCode.find(b);
			if (it1 == G.nodesByCode.end() || it2 == G.nodesByCode.end()) {
				std::cerr << "Warning: EDGE references undefined node '" << a << "' or '" << b << "'\n";
				continue;
			}
			Node* n1 = it1->second;
			Node* n2 = it2->second;
			n1->neighbors.emplace_back(n2, d);
			n2->neighbors.emplace_back(n1, d);
		}
	}
	in.close();
}

static pair<unordered_map<Node*, double>, unordered_map<Node*, Node*>> dijkstra(int V, string sourceCode, Graph G) {
	MinHeap pq(V);

	unordered_map<Node*, double> dist;
	unordered_map<Node*, Node*> prev;

	auto srcIt = G.nodesByCode.find(sourceCode);
	if (srcIt == G.nodesByCode.end()) {
		return { dist, prev }; // Return empty maps if source not found
	}

	Node* src = srcIt->second;
	dist[src] = 0.0;
	pq.insertKey({ 0.0, src });

	while (!pq.isEmpty()) {
		pair<double, Node*> top = pq.extractMin();
		Node* u = top.second;
		double currentDist = top.first;

		for (auto& neighbor : u->neighbors) {
			Node* nextNode = neighbor.first;
			double edgeCost = neighbor.second;
			double newDist = currentDist + edgeCost;

			if (!dist.count(nextNode) || dist[nextNode] > newDist) {
				dist[nextNode] = newDist;
				prev[nextNode] = u;

				if (pq.nodePos.find(nextNode) != pq.nodePos.end()) {
					int idx = pq.nodePos[nextNode];
					pq.decreaseKey(idx, { newDist, nextNode });
				}
				else {
					pq.insertKey({ newDist, nextNode });
				}
			}
		}
	}
	return { dist, prev };
}

vector<Node*> reconstructPath(Node* destination, unordered_map<Node*, Node*>& prev) {
	vector<Node*> path;
	for (Node* at = destination; at != nullptr; at = prev[at]) {
		path.push_back(at);
	}
	reverse(path.begin(), path.end());
	return path;
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

	system("cls");
	cout << "\t\tParcel Distribution System\t\n\n\tAll your Parcels delivered to you safely and quickly\n\n";
	cout << "\tAlready have an account?\n\n\tLogin or register an account to continue.\n\n";

	for (;;) {
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

		if (key == 75 && (counter == 2 || counter == 4)) { //75 left key 
			counter--;
		}
		if (key == 77 && (counter == 1)) { //77 right key 
			counter++;
		}
		if (key == 72 && (counter == 3 || counter == 4)) { //72 up Arr key
			counter--;
			counter--;
		}
		if (key == 80 && (counter == 1)) { //80 down Arr key
			counter++;
			counter++;
		}
		if (key == '\r') { // carriage return OR Enter key 
			log = counter;
			color(7);
			return log;
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
	}
	case 1:
	{
		if (loginUser()) {
			return true;
		}
		else {
			cout << "You cannot Log in" << endl;
			return false;
		}
	}
	case 3:
	{
		cout << "\n\tThank you for using Parcel Distribution system\n";
		return false;
	}
	default:
		cout << "\tInvalid Input" << endl;
		return false;
	}
}

bool checkUsername(const string& username) {
	ifstream file("usernames.txt");
	if (!file.is_open()) {
		return false;  // Treat as "not found" if file can't open
	}

	string line;
	string trimmedUsername = trim(username);
	while (getline(file, line)) {
		if (trim(line) == trimmedUsername) {
			file.close();  // Username found
			return true;
		}
	}

	file.close();  // Username not found
	return false;
}

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

// Main registration function
bool registerUser() {
	while (true) {
		cout << "\nEnter username: ";
		getline(cin, userInfo.username);
		userInfo.username = trim(userInfo.username);

		if (userInfo.username.empty() || !startsWithLetters(userInfo.username)) {
			cout << "\tInvalid username! It must start with a letter.\n";
			continue;
		}

		if (checkUsername(userInfo.username)) {
			cout << "\tUsername already exists! Please choose a different one.\n";
			continue; // Changed from return false to continue
		}

		break; // Valid username and doesn't exist
	}

	// Get password
	while (true) {
		cout << "\tEnter password: ";
		getline(cin, userInfo.password);
		userInfo.password = trim(userInfo.password);
		if (userInfo.password.empty()) {
			cout << "\tPassword cannot be empty.\n";
			continue;
		}
		break;
	}

	// Get role
	while (true) {
		cout << "\n\tSelect Role:\n";
		cout << "\t1. Rider\n";
		cout << "\t2. Customer\n";
		cout << "\tEnter choice (1 or 2): ";
		string input;
		getline(cin, input);
		input = trim(input);

		if (input == "1") {
			userInfo.role = "rider";
			break;
		}
		else if (input == "2") {
			userInfo.role = "customer";
			break;
		}
		else {
			cout << "\tInvalid choice. Please enter 1 for Rider or 2 for Customer.\n";
		}
	}

	// Write username to usernames.txt
	ofstream unameFile("usernames.txt", ios::app);
	if (!unameFile) {
		cout << "ERROR! Unable to open usernames.txt\n";
		return false;
	}
	unameFile << userInfo.username << endl;
	unameFile.close();

	// Write full user data to users.txt
	ofstream userFile("users.txt", ios::app);
	if (!userFile) {
		cout << "ERROR! Unable to open users.txt\n";
		return false;
	}
	userFile << userInfo.username << " " << userInfo.password << " " << userInfo.role << "\n";
	userFile.close();

	cout << "Registration Successful!\n";
	return true;
}

// function to login 
bool loginUser() {
	string u, p, name, password, role;
	bool flag = false;
	cout << "\n\tLogin Info:" << endl;
	cout << "\tEnter your username: ";
	getline(cin, name);
	cout << "\tEnter your password: ";
	getline(cin, password); // Changed from cin >> password to getline

	ifstream inFile("users.txt");
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
		cout << "\tLogin Unsuccessful, please provide correct credentials next time!" << endl;
		return false;
	}
	else {
		cout << "\tError: Unable to open file for login." << endl;
		return false;
	}
}

string selectZone() {
	if (G.zonesById.empty()) {
		cout << "No zones available!\n";
		return "";
	}

	int choice;
	int idx = 1;
	for (auto& a : G.zonesById) {
		cout << idx++ << ". " << a.first << " " << a.second->name << endl;
	}
	cout << "Select a Zone: ";
	cin >> choice;
	cin.ignore();

	if (choice < 1 || choice > G.zonesById.size()) {
		cout << "Invalid Zone selected!\n";
		return "";
	}

	idx = 1;
	for (auto& a : G.zonesById) {
		if (idx == choice) {
			return a.first;
		}
		idx++;
	}
	return "";
}

string selectLocation(const string& zoneCode) {
	auto zoneIt = G.zonesById.find(zoneCode);
	if (zoneIt == G.zonesById.end()) {
		cout << "Zone not found!\n";
		return "";
	}

	Zone* zone1 = zoneIt->second;
	if (zone1->nodes.empty()) {
		cout << "No locations available in this zone!\n";
		return "";
	}

	int idx = 1;
	for (auto& a : zone1->nodes) {
		cout << idx++ << ". " << a->code << " " << a->areaName << endl;
	}

	int choice;
	cout << "Select a Location: ";
	cin >> choice;
	cin.ignore();

	if (choice < 1 || choice > zone1->nodes.size()) {
		cout << "Invalid Location selected!\n";
		return "";
	}

	idx = 1;
	for (auto& a : zone1->nodes) {
		if (idx == choice) {
			return a->code;
		}
		idx++;
	}
	return "";
}

void generateInvoice(const Order& order) {
	auto pickupIt = G.nodesByCode.find(order.pickupNodeCode);
	auto dropoffIt = G.nodesByCode.find(order.dropoffNodeCode);

	cout << "\n--------------------------------------\n";
	cout << "         Parcel Delivery Invoice      \n";
	cout << "--------------------------------------\n";
	cout << "Order ID: " << order.orderID << endl;
	cout << "Customer Name: " << order.customerName << endl;
	cout << "Phone Number: " << order.phoneNumber << endl;

	if (pickupIt != G.nodesByCode.end()) {
		cout << "Pickup: " << pickupIt->second->areaName << "\n";
	}
	else {
		cout << "Pickup: " << order.pickupNodeCode << " (Node not found)\n";
	}

	if (dropoffIt != G.nodesByCode.end()) {
		cout << "Drop-off: " << dropoffIt->second->areaName << "\n";
	}
	else {
		cout << "Drop-off: " << order.dropoffNodeCode << " (Node not found)\n";
	}

	cout << "Parcel Weight: " << order.parcelWeight << " kg\n";
	cout << "Parcel Type: " << order.parcelType << endl;
	cout << "Order Status: " << order.status << endl;
	cout << "Urgent : " << order.hasHardDeadline ? "Yes\n" : "No\n";
	if(order.deadlineTime >= 0 && order.deadlineTime <1440)
		cout << "Deadline Time: " << order.deadlineTime << endl;
	cout << "--------------------------------------\n";
}

void loadOrdersFromFile() {
	orders.clear(); // Clear existing orders
	ifstream inFile("orders.txt");
	if (!inFile.is_open()) {
		return; // File doesn't exist yet, that's okay
	}

	string line;
	while (getline(inFile, line)) {
		if (line.empty()) continue;

		stringstream ss(line);
		Order o;
		string weight, time;
		string hasDeadline;


		getline(ss, o.orderID, ',');
		getline(ss, o.customerName, ',');
		getline(ss, o.phoneNumber, ',');
		getline(ss, o.pickupZoneCode, ',');
		getline(ss, o.pickupNodeCode, ',');
		getline(ss, o.dropoffZoneCode, ',');
		getline(ss, o.dropoffNodeCode, ',');
		getline(ss, weight, ',');
		getline(ss, o.parcelType, ',');
		getline(ss, o.status, ',');
		getline(ss, hasDeadline, ',');
		getline(ss, time , ',');

		o.hasHardDeadline = hasDeadline == "true" ? true : false;
		o.deadlineTime = stoi(time);
		o.parcelWeight = stof(weight);
		orders.push_back(o);
	}
	inFile.close();
}

void saveOrderToFile(const Order& order) {
	string d = order.hasHardDeadline ? "true" : "false";
	ofstream outFile("orders.txt", ios::app);
	if (outFile.is_open()) {
		outFile << order.orderID << ","
			<< order.customerName << ","
			<< order.phoneNumber << ","
			<< order.pickupZoneCode << ","
			<< order.pickupNodeCode << ","
			<< order.dropoffZoneCode << ","
			<< order.dropoffNodeCode << ","
			<< order.parcelWeight << ","
			<< order.parcelType << ","
			<< order.status << ","
			<< d << "," 
			<< order.deadlineTime << "\n";
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
			string d = order.hasHardDeadline ? "true" : "false";
			outFile << order.orderID << ","
				<< order.customerName << ","
				<< order.phoneNumber << ","
				<< order.pickupZoneCode << ","
				<< order.pickupNodeCode << ","
				<< order.dropoffZoneCode << ","
				<< order.dropoffNodeCode << ","
				<< order.parcelWeight << ","
				<< order.parcelType << ","
				<< order.status << ","
				<< d << ","
				<< order.deadlineTime << "\n";
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
	newOrder.pickupZoneCode = selectZone();
	if (newOrder.pickupZoneCode.empty()) return;

	cout << "\nSelect Pickup Location:\n";
	newOrder.pickupNodeCode = selectLocation(newOrder.pickupZoneCode);
	if (newOrder.pickupNodeCode.empty()) return;

	cout << "\nSelect Drop-off Zone:\n";
	newOrder.dropoffZoneCode = selectZone();
	if (newOrder.dropoffZoneCode.empty()) return;

	cout << "\nSelect Drop-off Location:\n";
	newOrder.dropoffNodeCode = selectLocation(newOrder.dropoffZoneCode);
	if (newOrder.dropoffNodeCode.empty()) return;

	cout << "Enter Parcel Weight (kg): ";
	cin >> newOrder.parcelWeight;
	cin.ignore();

	cout << "Enter Parcel Type (Document, Electronics, Clothes): ";
	getline(cin, newOrder.parcelType);

	cout << "Is this an urgent (hard‐deadline) delivery? (Y/N): ";
	char urgent; cin >> urgent;
	if (urgent == 'Y' || urgent == 'y') {
		newOrder.hasHardDeadline = true;
		cout << "Enter deadline time in hours (0–23) and minutes (0–59): ";
		int hr, mn;
		scanf("%d %d", &hr, &mn);
		newOrder.deadlineTime = hr * 60 + mn;  // store as minutes since midnight
	}
	else {
		newOrder.hasHardDeadline = false;
		newOrder.deadlineTime = 1e9; // a very large number so it’s effectively soft/none
	}

	// Generate order ID
	stringstream ss;
	ss << setw(3) << setfill('0') << (orders.size() + 1);
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

	bool orderFound = false;
	for (const auto& order : orders) {
		if (order.orderID == id) {
			cout << "\nOrder Status: " << order.status << endl;
			generateInvoice(order);
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
			if (order.status == "Delivered") {
				cout << "Cannot cancel a delivered order!\n";
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
		cout << "\n======= Customer Menu =======\n";
		cout << "1. Place New Order\n";
		cout << "2. Track Order\n";
		cout << "3. Cancel Order\n";
		cout << "4. View Order History\n";
		cout << "5. Logout\n";
		cout << "Enter your choice: ";
		cin >> choice;
		cin.ignore();

		switch (choice) {
		case 1: placeNewOrder(); break;
		case 2: trackOrder(); break;
		case 3: cancelOrder(); break;
		case 4: viewOrderHistory(); break;
		case 5: {
			system("cls");
			int log = welcome();
			menu1(log);
			return; // Exit the menu loop
		}
		default:
			cout << "Invalid choice. Please try again.\n";
		}
	} while (choice != 5);
}

void riderMenu() {
	int choice;
	do {
		cout << "\n======= Rider Menu =======\n";
		cout << "1. View Available Orders\n";
		cout << "2. Start Delivery\n";
		cout << "3. Logout\n";
		cout << "Enter your choice: ";
		cin >> choice;
		cin.ignore();

		switch (choice) {
		case 1:
			viewAssignedOrders();
			break;
		case 2:
		{
			cout << "Select your vehicle type:\n"
				<< "1. Bike (max 10 kg)\n"
				<< "2. Car (max 50 kg)\n"
				<< "3. Truck (max 200 kg)\n";
			int choice; cin >> choice;
			double capacity = (choice == 1 ? 10.0 : choice == 2 ? 50.0 : 200.0);
			/*string loc = "A1";*/
			startDelivery(driverLoc, capacity);
			break;
		}
		case 3:
		{
			system("cls");
			int log = welcome();
			menu1(log);
			return; // Exit the menu loop
		}
		default:
			cout << "Invalid choice. Try again.\n";
		}
	} while (choice != 3);
}

// gets distance between any two nodes
double getDistanceBetweenNodes(Node* A, Node* B) {
	auto& spInfo = allShortestPaths[A->code].first;
	if (spInfo.find(B) == spInfo.end()) {
		return DBL_MAX;
	}
	return spInfo[B];
}

double timeInMinutes(Node* A, Node* B) {
	// 1) Look up the precomputed distance (in km) from A → B
	auto& spInfo = allShortestPaths[A->code].first;
	if (spInfo.find(B) == spInfo.end()) {
		return DBL_MAX; // unreachable
	}
	double distKm = spInfo[B];
	// 2) Convert km → minutes (time = distance / speed * 60)
	double minutes = (distKm / avgSpeed) * 60.0;
	return minutes;
}

// given a starting location, generates a path for the rider to pick orders and deliver them in one go, currently no constraints are handled except
// that the rider cannot go to a destination without going to its pickup location first ( cannot deliver a order you don't have )
// so keeping pickup location and dest location adjacent such that once the rider picks up a parcel it delivers it first
// not optimal bt simpler to code, will later change if time allows.
vector<Node*> computeSingleRiderRoute(const string& startCode, const vector<int>& placedIndices, double capacity) {

	vector<int > remaining = placedIndices;
	vector<int> missedDeadlines;
	vector<int> tooHeavyOrders;
	vector<Node*> fullVisitSequence; // our route

	Node* currentNode = G.nodesByCode[startCode];
	
	while (!remaining.empty()) {

		double bestDist = DBL_MAX;
		int bestOrderIdx = -1;
		bool foundHardOrder = false;

		for (int idxPos = 0; idxPos < (int)remaining.size();idxPos++) {
			int orderIndex = remaining[idxPos];
			Order& o = orders[orderIndex];

			if (o.parcelWeight > capacity) {
				continue;
			}

			if (!o.hasHardDeadline) {
				continue;
			}
			Node* pickupNode = G.nodesByCode[o.pickupNodeCode];
			Node* dropoffNode = G.nodesByCode[o.dropoffNodeCode];

			double distToPickup = getDistanceBetweenNodes(currentNode, pickupNode);
			if (distToPickup == DBL_MAX) continue;

			double timeToPickup = (distToPickup / avgSpeed) * 60.0;
			
			double distToDropoff = getDistanceBetweenNodes(pickupNode, dropoffNode);
			if (distToDropoff == DBL_MAX) continue;
			double timeToDropoff = (distToDropoff / avgSpeed) * 60.0;

			double finishTime = currentTime + timeToPickup + T_pickup + timeToDropoff + T_dropoff;
			if (finishTime > o.deadlineTime) {
				missedDeadlines.push_back(orderIndex);
				continue;
			}

			if (distToPickup < bestDist) {
				bestDist = distToPickup;
				bestOrderIdx = idxPos;
				foundHardOrder = true;
			}
		}
		
		if (!foundHardOrder) {
			bestDist = DBL_MAX;
			bestOrderIdx = -1;
			
			for (int pos = 0; pos < (int)remaining.size(); pos++) {
				int    ordIdx = remaining[pos];
				Order& O = orders[ordIdx];

				// skip any order with a hard deadline (we either delivered it or recorded it as missed)
				if (O.hasHardDeadline) {
					continue;
				}
				// Skip overweight orders (record them)
				if (O.parcelWeight > capacity) {
					tooHeavyOrders.push_back(ordIdx);
					continue;
				}

				// distance to pickup
				Node* pickupNode = G.nodesByCode[O.pickupNodeCode];
				double distToPickup = getDistanceBetweenNodes(currentNode, pickupNode);
				if (distToPickup == DBL_MAX) {
					// unreachable, skip
					continue;
				}

				// Among no‐deadline, pick the nearest pickup
				if (distToPickup < bestDist) {
					bestDist = distToPickup;
					bestOrderIdx = pos;
				}
			}
		}
		
		if (bestOrderIdx < 0) break;
		
		int chosenOrderIdx = remaining[bestOrderIdx];
		Order& o = orders[chosenOrderIdx];
		Node* pickupNode = G.nodesByCode[o.pickupNodeCode];
		Node* dropoffNode = G.nodesByCode[o.dropoffNodeCode];

		{
			auto& prevMap = allShortestPaths[currentNode->code].second;
			vector<Node*> pathToPickup = reconstructPath(pickupNode, prevMap);
			if (!pathToPickup.empty()) {
				if (!fullVisitSequence.empty() && fullVisitSequence.back() == pathToPickup.front()) {
					for (int i = 1; i < (int)pathToPickup.size(); i++)
						fullVisitSequence.push_back(pathToPickup[i]);
				}
				else {
					for (Node* n : pathToPickup)
						fullVisitSequence.push_back(n);
				}
			}
		}

		{
			auto& prevMap2 = allShortestPaths[pickupNode->code].second;
			vector<Node*> pathToDropoff = reconstructPath(dropoffNode, prevMap2);
			if (!pathToDropoff.empty()) {
				if (!fullVisitSequence.empty()
					&& fullVisitSequence.back() == pathToDropoff.front()) {
					for (int i = 1; i < (int)pathToDropoff.size(); i++)
						fullVisitSequence.push_back(pathToDropoff[i]);
				}
				else {
					for (Node* n : pathToDropoff)
						fullVisitSequence.push_back(n);
				}
			}
		}

		{
			double dist1 = getDistanceBetweenNodes(currentNode, pickupNode);
			double time1 = (dist1 / avgSpeed) * 60.0;
			double dist2 = getDistanceBetweenNodes(pickupNode, dropoffNode);
			double time2 = (dist2 / avgSpeed) * 60.0;
			currentTime += time1 +T_pickup + time2 + T_dropoff;
		}

		o.status = "Delivered";
		{
			double dist1 = getDistanceBetweenNodes(currentNode, pickupNode);
			double dist2 = getDistanceBetweenNodes(pickupNode, dropoffNode);
			cout << "Order " << o.orderID << " delivered. "
				<< "From “" << currentNode->areaName << "” → “"
				<< pickupNode->areaName << "” → “"
				<< dropoffNode->areaName << "”. "
				<< "Leg distance: " << (dist1 + dist2) << " km.\n";
		}

		currentNode = dropoffNode;
		remaining.erase(remaining.begin() + bestOrderIdx);
		driverLoc = currentNode->code;
	}
	if (!missedDeadlines.empty()) {
		cout << "\n--- MISSED HARD DEADLINES (cannot be served on time) ---\n";
		for (int idx : missedDeadlines) {
			cout << "Order " << orders[idx].orderID
				<< " missed its hard deadline (deadline = "
				<< orders[idx].deadlineTime << " min).\n";
		}
	}
	if (!tooHeavyOrders.empty()) {
		cout << "\n--- TOO HEAVY FOR THIS VEHICLE ---\n";
		for (int idx : tooHeavyOrders) {
			cout << "Order " << orders[idx].orderID
				<< " weight = " << orders[idx].parcelWeight
				<< " kg > capacity = " << capacity << " kg.\n";
		}
	}
	return fullVisitSequence;
}


void startDelivery(string riderLocation, double capacity) {
	loadOrdersFromFile();

	vector<int> placedIndices;
	for (int i = 0; i < (int)orders.size();i++) {
		if (orders[i].status == "Placed") {
			placedIndices.push_back(i);
		}
	}

	if (placedIndices.empty()) {
		cout << "No new orders to deliver right now.\n";
		return;
	}

	auto fullRoute = computeSingleRiderRoute(riderLocation, placedIndices, capacity);
	saveAllOrdersToFile();

	cout << "\n--- Full Delivery Route ---\n";
	Node* prev = nullptr;
	for (Node* hop : fullRoute) {
		if (prev) {
			double d = getDistanceBetweenNodes(prev, hop);
			cout << hop->code << " (" << hop->areaName << ")"
				<< " [dist from prev: " << d << "]\n";
		}
		else {
			cout << hop->code << " (" << hop->areaName << ")"
				<< "   [start here]\n";
		}
		prev = hop;
	}

	if (!fullRoute.empty()) {
		Node* lastNode = fullRoute.back();
		cout << "\nRider's final location: " 
		    << lastNode->code << " (" << lastNode->areaName << ")\n";
	}

}
void viewAssignedOrders() {
	loadOrdersFromFile();
	bool found = false;

	cout << "\n======= Available Orders to Deliver =======\n";

	for (const auto& Order : orders) {
		if (Order.status == "Placed") {
			cout << "Order ID: " << Order.orderID << endl;
			cout << "Pickup Location: " << Order.pickupNodeCode << endl;
			cout << "Drop-off Location: " << Order.dropoffNodeCode << endl;
			cout << "-----------------------------\n";
			found = true;
		}
	}

	if (!found) {
		cout << "No new orders currently available for delivery.\n";
	}
}

int main() {
	string fileName = "routes.txt";
	loadGraphFromFile(fileName, G);

	for (auto& pair : G.nodesByCode) {
		Node* n = pair.second;
		auto result = dijkstra(G.nodesByCode.size(), n->code, G);
		allShortestPaths[n->code] = result;
	}

	int log = welcome(); // user selects an option i.e Login / signup 
	menu1(log);
	return 0;
}

//some orders saved in file to use for future testing
//001, abbas, 12345, H, H8, F, F6, 2, gift, Placed
//002, abbas, 12345, C, C4, L, L1, 3, docs, Placed
//003, abbas, 12345, E, E5, I, I3, 1, clothes, Placed
//004, abbas, 12345, B, B10, N, N6, 5, electronics, Placed