﻿#include<iostream>
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
#include<limits>
#include<float.h>
#include<unordered_set>
#include<set>
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
//string driverLoc = "A1";
int T_pickup = 10;
int T_dropoff = 7;
// assuming the rider starts at 10 AM for delivery
// time is in minutes after midnight
int currentTime = (10 * 60);
double avgSpeed = 40.0; // speed in km/h

unordered_map<string, string> riderHomeDepot;
unordered_map<string, string> riderCurrentLoc;

void loadRiderDepots(const string& path = "riderDepots.txt") {
	ifstream in(path);
	if (!in.is_open()) {
		cout << "Warning: Could not open " << path
			<< " (using default depot for everyone).\n";
		return;
	}
	string user, depot;
	while (in >> user >> depot) {
		riderHomeDepot[user] = depot;
	}
	in.close();
}

string getHomeDepotFor(const string& riderUsername) {
	auto it = riderHomeDepot.find(riderUsername);
	if (it == riderHomeDepot.end()) {
		return "A1";
	}
	return it->second;
}

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
void color(int color);
vector<Node*> computeSingleRiderRoute(const string& startCode, const vector<int>& placedIndices, double capacity);

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

// Helper function to get console width and height
void getConsoleSize(int& width, int& height) {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
		width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	}
	else {
		width = 80;
		height = 25;
	}
}

// Center text horizontally
void printCentered(const string& text, int y, int consoleWidth, int colorCode = 7) {
	int x = (consoleWidth - (int)text.length()) / 2;
	gotoxy(x, y);
	color(colorCode);
	cout << text;
}

// Draw a box border with characters
void drawBox(int left, int top, int width, int height, int colorCode = 7) {
	color(colorCode);
	// Top border
	gotoxy(left, top);
	cout << char(201); // ┌
	for (int i = 0; i < width - 2; i++) cout << char(205); // ─
	cout << char(187); // ┐

	// Sides
	for (int i = 1; i < height - 1; i++) {
		gotoxy(left, top + i);
		cout << char(186); // │
		gotoxy(left + width - 1, top + i);
		cout << char(186); // │
	}

	// Bottom border
	gotoxy(left, top + height - 1);
	cout << char(200); // └
	for (int i = 0; i < width - 2; i++) cout << char(205); // ─
	cout << char(188); // ┘
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
	int consoleWidth, consoleHeight;
	getConsoleSize(consoleWidth, consoleHeight);
	system("cls");

	int boxWidth = 60;
	int boxHeight = 15;
	int left = (consoleWidth - boxWidth) / 2;
	int top = (consoleHeight - boxHeight) / 2;

	drawBox(left, top, boxWidth, boxHeight, 11); // Light Cyan box

	// Title
	printCentered("Parcel Distribution System", top + 1, consoleWidth, 14); // Yellow

	// Subtitle / tagline
	printCentered("Delivering your parcels safely and swiftly!", top + 3, consoleWidth, 7);

	// Instructions
	printCentered("Please login or register to continue", top + 5, consoleWidth, 7);

	// Menu options
	int Set[] = { 7,7,7 };
	int counter = 1;
	char key;
	int log;

	while (true) {
		// Reset colors
		Set[0] = 7;
		Set[1] = 7;
		Set[2] = 7;

		// Highlight current choice in bright red
		Set[counter - 1] = 12;

		// Print options centered with spacing
		string opt1 = "1. Login";
		string opt2 = "2. Register an account";
		string opt3 = "3. Exit";

		// Position options vertically spaced
		printCentered(opt1, top + 7, consoleWidth, Set[0]);
		printCentered(opt2, top + 9, consoleWidth, Set[1]);
		printCentered(opt3, top + 11, consoleWidth, Set[2]);

		key = _getch();

		if ((key == 72 || key == 'w' || key == 'W') && counter > 1) { // Up arrow or w
			counter--;
		}
		else if ((key == 80 || key == 's' || key == 'S') && counter < 3) { // Down arrow or s
			counter++;
		}
		else if (key == '\r') { // Enter key
			log = counter;
			color(7);
			system("cls");
			return log;
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
	int consoleWidth, consoleHeight;
	getConsoleSize(consoleWidth, consoleHeight);
	int boxWidth = 60;
	int boxHeight = 13;
	int left = (consoleWidth - boxWidth) / 2;
	int top = (consoleHeight - boxHeight) / 2;

	while (true) {
		system("cls");
		drawBox(left, top, boxWidth, boxHeight, 11);
		printCentered("User Registration", top + 1, consoleWidth, 14);

		gotoxy(left + 4, top + 3);
		color(7);
		cout << "Enter username (must start with letter): ";
		getline(cin, userInfo.username);
		userInfo.username = trim(userInfo.username);

		if (userInfo.username.empty() || !startsWithLetters(userInfo.username)) {
			gotoxy(left + 4, top + 5);
			color(12);
			cout << "Invalid username! Must start with a letter.";
			_getch();
			continue;
		}

		if (checkUsername(userInfo.username)) {
			gotoxy(left + 4, top + 5);
			color(12);
			cout << "Username already exists! Choose a different one.";
			_getch();
			continue;
		}

		// Password input
		gotoxy(left + 4, top + 5);
		color(7);
		cout << "Enter password: ";
		getline(cin, userInfo.password);
		userInfo.password = trim(userInfo.password);

		if (userInfo.password.find(' ') != string::npos) {
			gotoxy(left + 4, top + 7);
			color(12);
			cout << "Password cannot contain spaces.";
			_getch();
			continue;
		}

		// Role selection
		string input;
		while (true) {
			gotoxy(left + 4, top + 7);
			color(7);
			cout << "Select Role: 1. Rider  2. Customer (Enter 1 or 2): ";
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
				gotoxy(left + 4, top + 9);
				color(12);
				cout << "Invalid choice. Please enter 1 or 2.";
				_getch();
				gotoxy(left + 4, top + 9);
				cout << string(50, ' '); // Clear previous error
			}
		}

		// Save data to files
		ofstream unameFile("usernames.txt", ios::app);
		if (!unameFile) {
			gotoxy(left + 4, top + 10);
			color(12);
			cout << "ERROR! Unable to open usernames.txt";
			_getch();
			return false;
		}
		unameFile << userInfo.username << endl;
		unameFile.close();

		ofstream userFile("users.txt", ios::app);
		if (!userFile) {
			gotoxy(left + 4, top + 10);
			color(12);
			cout << "ERROR! Unable to open users.txt";
			_getch();
			return false;
		}
		userFile << userInfo.username << "\n" << userInfo.password << "\n" << userInfo.role << "\n";
		userFile.close();

		gotoxy(left + 4, top + 11);
		color(10); // Green
		cout << "Registration Successful! Press any key to continue...";
		_getch();
		return true;
	}
}

// function to login 
bool loginUser() {
	int consoleWidth, consoleHeight;
	getConsoleSize(consoleWidth, consoleHeight);
	int boxWidth = 60;
	int boxHeight = 10;
	int left = (consoleWidth - boxWidth) / 2;
	int top = (consoleHeight - boxHeight) / 2;

	while (true) {
		system("cls");
		drawBox(left, top, boxWidth, boxHeight, 11);
		printCentered("User Login", top + 1, consoleWidth, 14);

		string u, p, name, password, role;
		bool flag = false;
		gotoxy(left + 4, top + 3);
		color(7);
		cout << "Username: ";
		getline(cin, name);

		gotoxy(left + 4, top + 5);
		cout << "Password: ";
		getline(cin, password);

		ifstream inFile("users.txt");
		if (inFile.is_open()) {
			while (inFile >> u >> p >> role) {
				if (u == name && p == password) {
					inFile.close();
					fileName = name + ".txt";
					userInfo.username = u;
					userInfo.role = role;
					userInfo.password = p;

					username = u;
					//fileName = u + ".txt";

					gotoxy(left + 4, top + 7);
					color(10); // Green
					cout << "Login Successful! Press any key to continue...";
					_getch();
					inFile.close();

					system("cls");
					if (role == "customer") {
						customerMenu();
					}
					else if (role == "rider") {
						riderMenu();
					}
					return true;
				}
			}
			inFile.close();

			gotoxy(left + 4, top + 7);
			color(12); // Red
			cout << "Login failed! Incorrect username or password.";
			_getch();
		}
		else {
			gotoxy(left + 4, top + 7);
			color(12);
			cout << "Error: Unable to open users.txt";
			_getch();
			return false;
		}
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
	if (order.deadlineTime >= 0 && order.deadlineTime < 1440)
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
		getline(ss, time, ',');

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
		cin >> hr >> mn;
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


vector<int> collectPlacedOrders() {
	loadOrdersFromFile(); // load orders from file
	vector<int> placed;
	for (int i = 0; i < (int)orders.size(); i++) {
		if (orders[i].status == "Placed") {
			placed.push_back(i);
		}
	}
	return placed;
}

// function to count no of riders
vector<string> getRiders() {
	string u, p, role;
	vector<string> riders;

	ifstream inFile("users.txt");
	if (inFile.is_open()) {
		while (inFile >> u >> p >> role) {
			if (role == "rider") {
				riders.push_back(u);
			}
		}
		inFile.close();
	}
	else {
		cout << "\tError: Unable to open file for login." << endl;
	}
	return riders;
}

// gets distance between any two nodes
double getDistanceBetweenNodes(Node* A, Node* B) {
	auto& spInfo = allShortestPaths[A->code].first;
	if (spInfo.find(B) == spInfo.end()) {
		return DBL_MAX;
	}
	return spInfo[B];
}

void riderDispatchAll() {
	vector<string> riders = getRiders();
	int K = (int)riders.size();
	if (K == 0) {
		cout << "No riders found! Please register at least one rider first.\n";
		return;
	}
	if (K > 5) {
		cout << "Warning: More than 5 riders found. Only using the first 5.\n";
		K = 5;
		riders.resize(K);
	}

	vector<double> capacities(K, 10.0);
	for (int r = 0; r < K; r++) {
		cout << "\nRider: " << riders[r] << "\n";
		cout << "  Select your vehicle type:\n"
			<< "   1) Bike (max 10 kg)\n"
			<< "   2) Car  (max 50 kg)\n"
			<< "   3) Truck(max 200 kg)\n"
			<< "  Enter 1/2/3: ";
		int ch;
		cin >> ch;
		cin.ignore(1000, '\n');
		if (ch == 2)      capacities[r] = 50.0;
		else if (ch == 3) capacities[r] = 200.0;
		else              capacities[r] = 10.0;
	}

	vector<int> placed = collectPlacedOrders();
	if (placed.empty()) {
		cout << "No placed orders to dispatch right now.\n";
		return;
	}

	string referenceDepot = "A1";
	Node* refNode = G.nodesByCode[referenceDepot];

	vector<pair<int, double>> keyed;
	keyed.reserve(placed.size());
	for (int idx : placed) {
		Node* pnode = G.nodesByCode[orders[idx].pickupNodeCode];
		double d = getDistanceBetweenNodes(refNode, pnode);
		keyed.push_back({ idx, d });
	}
	sort(keyed.begin(), keyed.end(),
		[&](const pair<int, double>& a, const pair<int, double>& b) {
			return a.second < b.second;
		});

	vector<int> sortedPlaced;
	sortedPlaced.reserve(keyed.size());

	for (auto& kv : keyed) {
		sortedPlaced.push_back(kv.first);
	}

	vector<vector<int>> riderBuckets(K);
	for (int i = 0; i < (int)sortedPlaced.size(); i++) {
		int riderIdx = i % K;
		riderBuckets[riderIdx].push_back(sortedPlaced[i]);
	}

	for (int r = 0; r < K; r++) {
		const string& username = riders[r];

		string startCode;
		auto itCurr = riderCurrentLoc.find(username);
		if (itCurr != riderCurrentLoc.end()) {
			startCode = itCurr->second;
		}
		else {
			startCode = getHomeDepotFor(username);
		}

		if (G.nodesByCode.find(startCode) == G.nodesByCode.end()) {
			startCode = "A1";
		}

		cout << "\n=== Dispatching orders for Rider: " << username
			<< " (start at " << startCode << ") ===\n";

		color(11);
		cout << "Optimizing route for " << username << "...\n";
		color(7);

		vector<Node*> route = computeSingleRiderRoute(startCode, riderBuckets[r], capacities[r]);
		saveAllOrdersToFile();
		color(11);
		cout << "✓ Optimization complete!\n";
		color(7);

		if (!route.empty()) {
			/*cout << "--- Full Node‐by‐Node Route for " << username << " ---\n";
			for (Node* hop : route) {
				cout << hop->code << " (" << hop->areaName << ")\n";
			}*/
			cout << "[Route Summary] " << startCode
				<< " - " << route.back()->code
				<< " (" << route.back()->areaName << ")\n";

			Node* lastNode = route.back();
			/*cout << "Final Location: "
				<< lastNode->code << " (" << lastNode->areaName << ")\n";*/
			// (E) Update riderCurrentLoc so next dispatch uses this as the start
			riderCurrentLoc[username] = lastNode->code;
		}
		else {
			cout << "No feasible orders assigned to you (all overweight or missed deadlines).\n";
		}
		cout << "==============================================\n";
	}
}

void riderMenu() {
	int choice;
	do {
		cout << "\n======= Rider Menu =======\n";
		cout << "1. View Available Orders\n";
		cout << "2. Start Delivery\n";
		cout << "3. Logout\n";
		cout << "4. Start New Day\n";
		cout << "Enter your choice: ";
		cin >> choice;
		cin.ignore();

		switch (choice) {
		case 1:
			viewAssignedOrders();
			break;
		case 2:
			riderDispatchAll();
			color(10);
			break;

		case 3:
		{
			system("cls");
			int log = welcome();
			menu1(log);
			return; // Exit the menu loop
		}
		case 4:
			currentTime = 600;
			riderCurrentLoc.clear();
			break;
		default:
			cout << "Invalid choice. Try again.\n";
		}
	} while (choice != 3);
}


double timeInMinutes(Node* A, Node* B) {
	auto& spInfo = allShortestPaths[A->code].first;
	if (spInfo.find(B) == spInfo.end()) {
		return DBL_MAX; // unreachable
	}
	double distKm = spInfo[B];
	double minutes = (distKm / avgSpeed) * 60.0;
	return minutes;
}

vector<Node*> twoOptOptimizeRoute(
	const string& startCode,
	const vector<Node*>& route,
	const unordered_map<Node*, int>& nodeToOrder,
	const unordered_map<Node*, bool>& nodeIsDrop
) {
	if (route.size() < 3)
		return route;

	auto totalDistance = [&](const vector<Node*>& seq) {
		double distSum = 0.0;
		Node* prev = G.nodesByCode[startCode];
		for (Node* nxt : seq) {
			double d = getDistanceBetweenNodes(prev, nxt);
			if (d == DBL_MAX) return DBL_MAX;
			distSum += d;
			prev = nxt;
		}
		return distSum;
		};

	vector<Node*> bestRoute = route;
	double bestDist = totalDistance(bestRoute);

	bool improved = true;
	size_t N = bestRoute.size();

	while (improved) {
		improved = false;
		for (size_t i = 0; i < N - 1 && !improved; ++i) {
			for (size_t k = i + 1; k < N && !improved; ++k) {
				bool validSwap = true;


				unordered_map<int, size_t> origPickIdx, origDropIdx;
				for (size_t x = i; x <= k; ++x) {
					Node* node = bestRoute[x];

					auto itOrd = nodeToOrder.find(node);
					if (itOrd == nodeToOrder.end()) {
						continue;
					}
					int ord = itOrd->second;
					bool isDrop = nodeIsDrop.at(node);
					if (isDrop) {
						origDropIdx[ord] = x;
					}
					else {
						origPickIdx[ord] = x;
					}
				}

				for (auto& kv : origPickIdx) {
					int ord = kv.first;
					size_t p = kv.second;
					if (origDropIdx.count(ord)) {
						size_t d = origDropIdx[ord];
						size_t newP = i + (k - p);
						size_t newD = i + (k - d);
						if (newP >= newD) {
							validSwap = false;
							break;
						}
					}
				}
				if (!validSwap) continue;

				for (auto& kv : origDropIdx) {
					int ord = kv.first;
					size_t d = kv.second;
					if (!origPickIdx.count(ord)) {
						size_t origP = N;
						for (size_t y = 0; y < N; ++y) {
							auto itO = nodeToOrder.find(bestRoute[y]);
							if (itO != nodeToOrder.end() && itO->second == ord
								&& !nodeIsDrop.at(bestRoute[y])) {
								origP = y;
								break;
							}
						}
						if (origP == N) {
							validSwap = false;
							break;
						}
						size_t newD = i + (k - d);
						if (origP >= newD) {
							validSwap = false;
							break;
						}
					}
				}
				if (!validSwap) continue;

				for (auto& kv : origPickIdx) {
					int ord = kv.first;
					size_t p = kv.second;   // original pickup idx
					if (!origDropIdx.count(ord)) {
						size_t origD = N;
						for (size_t y = 0; y < N; ++y) {
							auto itO = nodeToOrder.find(bestRoute[y]);
							if (itO != nodeToOrder.end() && itO->second == ord
								&& nodeIsDrop.at(bestRoute[y])) {
								origD = y;
								break;
							}
						}
						if (origD == N) {
							validSwap = false;
							break;
						}
						size_t newP = i + (k - p);
						if (newP >= origD) {
							validSwap = false;
							break;
						}
					}
				}
				if (!validSwap) continue;

				vector<Node*> newRoute = bestRoute;
				reverse(newRoute.begin() + i, newRoute.begin() + k + 1);
				double newDist = totalDistance(newRoute);
				if (newDist < bestDist) {
					bestDist = newDist;
					bestRoute = move(newRoute);
					improved = true;
				}
			}
		}
	}

	return bestRoute;
}

vector<Node*> computeSingleRiderRoute(const string& startCode, const vector<int>& placedIndices, double capacity) {
	struct PDU {
		int ordIdx;
		Node* pickupNode;
		Node* dropoffNode;
		bool pickedUp;
		bool delivered;
		float weight;
	};
	vector<PDU> tasks;
	for (int idx : placedIndices) {
		Order& o = orders[idx];
		tasks.push_back({ idx, G.nodesByCode[o.pickupNodeCode], G.nodesByCode[o.dropoffNodeCode], false , false, o.parcelWeight });
	}

	Node* currentNode = G.nodesByCode[startCode];
	double localTime = currentTime;
	float currentLoad = 0;
	vector<Node*> fullVisitSequence;

	while (true) {
		vector<int> missedPos, heavyPos, feasibleHardPos, feasibleSoftPos;
		for (int i = 0; i < (int)tasks.size(); ++i) {
			PDU& t = tasks[i];
			if (t.delivered) continue;

			if (!t.pickedUp) {
				if (t.weight + currentLoad > capacity) {
					heavyPos.push_back(i);
					continue;
				}

				Order& o = orders[t.ordIdx];
				if (o.hasHardDeadline) {
					Node* PU = t.pickupNode;
					Node* DO = t.dropoffNode;
					double d1 = getDistanceBetweenNodes(currentNode, PU);
					double d2 = getDistanceBetweenNodes(PU, DO);

					if (d1 == DBL_MAX || d2 == DBL_MAX) {
						missedPos.push_back(i);
						continue;
					}
					double t1 = (d1 / avgSpeed) * 60.0;
					double t2 = (d2 / avgSpeed) * 60.0;
					double finishTime = localTime + t1 + T_pickup + t2 + T_dropoff;
					if (finishTime > o.deadlineTime) {
						missedPos.push_back(i);
					}
					else {
						feasibleHardPos.push_back(i);
					}
				}

				else {
					Node* PU = t.pickupNode;
					double d1 = getDistanceBetweenNodes(currentNode, PU);
					if (d1 == DBL_MAX) {
						missedPos.push_back(i);
					}
					else {
						feasibleSoftPos.push_back(i);
					}
				}
			}

			else {
				Node* DO = t.dropoffNode;
				double d2 = getDistanceBetweenNodes(currentNode, DO);
				if (d2 == DBL_MAX) {
					missedPos.push_back(i);
				}
				else {
					feasibleHardPos.push_back(i);
				}
			}
		}

		vector<int> toRemove = missedPos;
		toRemove.insert(toRemove.end(), heavyPos.begin(), heavyPos.end());
		sort(toRemove.rbegin(), toRemove.rend());
		for (int pos : toRemove) {
			if (pos >= 0 && pos < (int)tasks.size()) {
				tasks[pos].delivered = true; // mark as “done/cancelled”
			}
		}

		int chosen = -1;
		if (!feasibleHardPos.empty()) {
			double bestD = DBL_MAX;
			for (int i : feasibleHardPos) {
				PDU& t = tasks[i];
				Node* target = t.pickedUp ? t.dropoffNode : t.pickupNode;
				double d = getDistanceBetweenNodes(currentNode, target);
				if (d < bestD) { bestD = d; chosen = i; }
			}
		}
		else if (!feasibleSoftPos.empty()) {
			double bestD = DBL_MAX;
			for (int i : feasibleSoftPos) {
				PDU& t = tasks[i];
				Node* target = t.pickupNode;
				double d = getDistanceBetweenNodes(currentNode, target);
				if (d < bestD) { bestD = d; chosen = i; }
			}
		}
		else {
			// No more deliverable tasks remain
			break;
		}

		PDU& t = tasks[chosen];
		if (!t.pickedUp) {
			auto& prevMap = allShortestPaths[currentNode->code].second;
			vector<Node*> pathToPU = reconstructPath(t.pickupNode, prevMap);
			if (!pathToPU.empty()) {
				if (!fullVisitSequence.empty() && fullVisitSequence.back() == pathToPU.front()) {
					for (size_t k = 1; k < pathToPU.size(); ++k)
						fullVisitSequence.push_back(pathToPU[k]);
				}
				else {
					for (auto* n : pathToPU)
						fullVisitSequence.push_back(n);
				}
			}
			double d1 = getDistanceBetweenNodes(currentNode, t.pickupNode);
			double t1 = (d1 / avgSpeed) * 60.0;
			localTime += t1 + T_pickup;
			currentLoad += t.weight;
			t.pickedUp = true;
			currentNode = t.pickupNode;
			cout << "Order #" << t.ordIdx
				<< " | Pickup: " << currentNode->code << " (" << currentNode->areaName << ")"
				<< " | Weight: " << fixed << setprecision(1) << t.weight << " kg\n";
		}
		else {
			auto& prevMap2 = allShortestPaths[currentNode->code].second;
			vector<Node*> pathToDO = reconstructPath(t.dropoffNode, prevMap2);
			if (!pathToDO.empty()) {
				if (!fullVisitSequence.empty() && fullVisitSequence.back() == pathToDO.front()) {
					for (size_t k = 1; k < pathToDO.size(); ++k)
						fullVisitSequence.push_back(pathToDO[k]);
				}
				else {
					for (auto* n : pathToDO)
						fullVisitSequence.push_back(n);
				}
			}
			double d2 = getDistanceBetweenNodes(currentNode, t.dropoffNode);
			double t2 = (d2 / avgSpeed) * 60.0;
			localTime += t2 + T_dropoff;
			currentLoad -= t.weight;
			t.delivered = true;
			orders[t.ordIdx].status = "Delivered";
			currentNode = t.dropoffNode;
			color(10);
			cout << "Order #" << t.ordIdx
				<< " | Drop: " << currentNode->code << " (" << currentNode->areaName << ")"
				<< " | Remaining Load: " << currentLoad
				<< " | Dist: " << fixed << setprecision(2) << d2 << " km\n";
			color(7);
		}

	}

	set<int> printMissed, printHeavy;
	for (auto& t : tasks) {
		if (!t.pickedUp && !t.delivered) {
			if (t.weight + 0 > capacity)
				printHeavy.insert(t.ordIdx);
			else printMissed.insert(t.ordIdx);
		}
		else if (t.pickedUp && !t.delivered) {
			printMissed.insert(t.ordIdx);
		}
	}

	if (!printMissed.empty()) {
		cout << "\n--- MISSED OR UNREACHABLE ORDERS ---\n";
		for (int idx : printMissed) {
			color(14);
			cout << "Order " << idx
				<< " missed/unreachable. Deadline = "
				<< orders[idx].deadlineTime << " min\n";
			color(7);
		}
	}
	if (!printHeavy.empty()) {
		cout << "\n--- TOO HEAVY FOR THIS VEHICLE ---\n";
		for (int idx : printHeavy) {
			color(12);
			cout << "Order " << idx
				<< " weight=" << fixed << setprecision(2)
				<< orders[idx].parcelWeight
				<< " kg > capacity=" << capacity << " kg\n";
			color(7);
		}
	}

	unordered_map<Node*, int> nodeToOrder;
	unordered_map<Node*, bool> nodeIsDrop;
	for (auto& t : tasks) {
		nodeToOrder[t.pickupNode] = t.ordIdx;
		nodeIsDrop[t.pickupNode] = false;
		nodeToOrder[t.dropoffNode] = t.ordIdx;
		nodeIsDrop[t.dropoffNode] = true;
	}

	if (fullVisitSequence.size() > 3) {
		int w, h;
		color(11);
		fullVisitSequence = twoOptOptimizeRoute(startCode, fullVisitSequence, nodeToOrder, nodeIsDrop);
		color(7);
	}

	return fullVisitSequence;
}

void viewAssignedOrders() {
	loadOrdersFromFile();
	bool found = false;

	cout << "\n======= Available Orders to Deliver =======\n";

	for (const auto& Order : orders) {
		if (Order.status == "Placed") {
			cout << "Order ID: " << Order.orderID << endl;
			cout << "Pickup Location: " << G.nodesByCode[Order.pickupNodeCode]->areaName << endl;
			cout << "Drop-off Location: " << G.nodesByCode[Order.dropoffNodeCode]->areaName << endl;
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
	loadRiderDepots("riderDepots.txt");
	int log = welcome(); // user selects an option i.e Login / signup 
	menu1(log);
	return 0;
}