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
}userInfo;

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
};
vector<Order> orders;

string username, fileName, password;

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
void startDelivery(string loc);
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

static string unquote(const string& s) {
	if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
		return s.substr(1, s.size() - 2);
	return s;
}

void loadGraphFromFile(const string& path, Graph& G) {
	ifstream in(path);
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
}

static pair<unordered_map<Node*, double>, unordered_map<Node*, Node*>> dijkstra(int V, string sourceCode, Graph G) {
	MinHeap pq(V);

	unordered_map<Node*, double> dist;
	unordered_map<Node*, Node*> prev;

	Node* src = G.nodesByCode[sourceCode];
	dist[src] = 0.0;
	pq.insertKey({ 0.0, src });
	pq.nodePos[src] = 0;

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
					pq.nodePos[nextNode] = pq.heap_size - 1;
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

string selectZone() {
	int choice;
	int idx = 1;
	for (auto a : G.zonesById) {
		cout << idx++ << ". " << a.first << " " << a.second->name << endl;
	}
	cout << "Select a Zone: ";
	cin >> choice;
	idx = 1;
	string zId;
	for (auto a : G.zonesById) {
		if (idx == choice) {
			zId = a.first;
		}
		idx++;
	}

	cin.ignore();

	if (choice < 1 || choice > G.zonesById.size()) {
		cout << "Invalid Zone selected!\n";
		return "";
	}
	return zId;
}

string selectLocation(const string zoneCode) {
	Zone* zone1 = G.zonesById[zoneCode];
	int idx = 1;
	for (auto a : zone1->nodes) {
		cout << idx++ << ". " << a->code << " " << a->areaName << endl;
	}
	int choice;
	cout << "Select a Location: ";
	cin >> choice;
	idx = 1;
	cin.ignore();

	if (choice < 1 || choice > zone1->nodes.size()) {
		cout << "Invalid Location selected!\n";
		return "";
	}
	string nodeCode;
	for (auto a : zone1->nodes) {
		if (idx == choice) {
			nodeCode = a->code;
		}
		idx++;
	}
	cout << nodeCode << endl;
	return nodeCode;
}

void generateInvoice(const Order& order) {
	Node* pickupNode = G.nodesByCode[order.pickupNodeCode];
	Node* dropoffNode = G.nodesByCode[order.dropoffNodeCode];
	cout << "\n--------------------------------------\n";
	cout << "         Parcel Delivery Invoice      \n";
	cout << "--------------------------------------\n";
	cout << "Order ID: " << order.orderID << endl;
	cout << "Customer Name: " << order.customerName << endl;
	cout << "Phone Number: " << order.phoneNumber << endl;
	cout << "Pickup: " << pickupNode->areaName << "\n";
	cout << "Drop-off: " << dropoffNode->areaName << "\n";
	cout << "Parcel Weight: " << order.parcelWeight << " kg\n";
	cout << "Parcel Type: " << order.parcelType << endl;
	cout << "Order Status: " << order.status << endl;
	cout << "--------------------------------------\n";
}

void loadOrdersFromFile() {
	ifstream inFile("orders.txt");
	string line;
	while (getline(inFile, line)) {
		stringstream ss(line);
		Order o;
		string weight;

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

		orders.push_back(o);
	}
}

void saveOrderToFile(const Order& order) {
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
				<< order.pickupZoneCode << ","
				<< order.pickupNodeCode << ","
				<< order.dropoffZoneCode << ","
				<< order.dropoffNodeCode << ","
				<< order.parcelWeight << ","
				<< order.parcelType << ","
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
		string orderID, customerName, phoneNumber, pickupZone, pickupLocation, dropoffZone, dropoffLocation, parcelWeight, parcelType, status;
		getline(ss, orderID, ',');
		getline(ss, customerName, ',');
		getline(ss, phoneNumber, ',');
		getline(ss, pickupZone, ',');
		getline(ss, pickupLocation, ',');
		getline(ss, dropoffZone, ',');
		getline(ss, dropoffLocation, ',');
		getline(ss, parcelWeight, ',');
		getline(ss, parcelType, ',');
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
void riderMenu() {
	int choice;
	do {
		cout << "\n======= Rider Menu =======\n";
		cout << "1. View Assigned Orders\n";
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
			string loc = "A1";
			startDelivery(loc);
			break;
		}
		default:
			cout << "Invalid choice. Try again.\n";
		}
	} while (choice != 3);
}
void startDelivery(string riderLocation) {
	loadOrdersFromFile();
	string selectedOrderId;
	bool found = false;

	cout << "\nEnter the Order ID you want to deliver: ";
	cin >> selectedOrderId;

	for (auto& Order : orders) {
		if (Order.orderID == selectedOrderId && Order.status == "Placed") {
			found = true;

			cout << "\nStarting delivery Order ID: " << selectedOrderId << endl;
			cout << "Rider  Location: " << riderLocation << endl;
			if (G.nodesByCode.find(Order.pickupNodeCode) == G.nodesByCode.end() || G.nodesByCode.find(Order.dropoffNodeCode) == G.nodesByCode.end()) {
				cout << "One or both node codes do not exist." << endl;
				break;
			}
			if (riderLocation != Order.pickupNodeCode) {
				vector<Node*> path;
				Node* dest = G.nodesByCode[Order.pickupNodeCode];
				bool exists = false;
				if (allShortestPaths.find(riderLocation) != allShortestPaths.end()) {
					pair<unordered_map<Node*, double>, unordered_map<Node*, Node*>> path1 = allShortestPaths[riderLocation];
					if (path1.first.find(dest) != path1.first.end()) {                                                                                                                                                                                                            
						exists = true;
						cout << "The minimum distance from " << riderLocation << " to " << Order.pickupNodeCode << " is: " << path1.first[dest] << endl;
						path = reconstructPath(dest, path1.second);
					
						cout << " Going to Pickup Location: " << Order.pickupNodeCode << endl;
						cout << "Path: ";
						for (Node* node : path) {
							cout << node->code << " " << node->areaName << " " << path1.first[node] << endl;
							system("pause");
						}
						cout << endl;
					}
				}
					
			}

			riderLocation = Order.pickupNodeCode;
			cout << "Arrived at Pickup Location: " << riderLocation << endl;

			if (riderLocation != Order.dropoffNodeCode) {
				vector<Node*> path;
				Node* dest = G.nodesByCode[Order.dropoffNodeCode];
				bool exists = false;
				if (allShortestPaths.find(riderLocation) != allShortestPaths.end()) {
					pair<unordered_map<Node*, double>, unordered_map<Node*, Node*>> path1 = allShortestPaths[riderLocation];
					if (path1.first.find(dest) != path1.first.end()) {
						exists = true;
						cout << "The minimum distance from " << riderLocation << " to " << Order.dropoffNodeCode << " is: " << path1.first[dest] << endl;
						path = reconstructPath(dest, path1.second);

						cout << " Going to Dropoff Location: " << Order.dropoffNodeCode << endl;
						cout << "Path: ";
						for (Node* node : path) {
							cout << node->code<< " " << node->areaName << " " << path1.first[node] << endl;
							system("pause");
						}
						cout << endl;
					}
				}

			}

			riderLocation = Order.dropoffNodeCode;
			cout << "Arrived at Drop-off Location: " << riderLocation << endl;

			Order.status = "Delivered";
			saveAllOrdersToFile();
			cout << "Order " << Order.orderID << " marked as DELIVERED.\n";
			cout << "Rider's new current location: " << riderLocation << endl;

			break;
		}
	}
	

	if (!found) {
		cout << "\n// Order not found or already delivered.\n";
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
