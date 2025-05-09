#include<iostream>
#include<string>
#include<fstream>
#include<sstream>
#include<vector>
#include<queue>
#include<unordered_map>
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
	decreaseKey(i, {DBL_MIN, harr[i].second});
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
	while (start <= end && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r' || s[start] == '\n')){
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

static pair<unordered_map<Node*, double>, unordered_map<Node*, Node*>> dijkstra(int V ,string sourceCode, Graph G) {
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

//void main() {
//	string fileName = "routes.txt";
//	Graph G;
//	loadGraphFromFile(fileName, G);
//	unordered_map<string, pair<unordered_map<Node*, double>, unordered_map<Node*, Node*>>> allShortestPaths;
//
//
//	cout << "Nodes Loaded: " << G.nodesByCode.size() << endl;
//	cout << "Zones loaded: " << G.zonesById.size() << endl;
//
//	for (auto& pair : G.nodesByCode) {
//		Node* n = pair.second;
//		//cout << n->code << " ( " << n->areaName << ") " << n->neighbors.size() << " neighbors\n";
//		auto result = dijkstra(G.nodesByCode.size(), n->code, G);
//		allShortestPaths[n->code] = result;
//	}
//	//unordered_map<Node*, double> result = dijkstra(G.nodesByCode.size(), "A1", G);
//	int count = 0;
//	/*for (auto& dist : result) {
//		cout << "The distance from A1 to " << dist.first->code << " is " << dist.second << " " << endl;
//		count++;
//	}*/
//	//cout << "The count is : " << count << endl;
//	bool cont = true;
//	while (cont) {
//		cout << "1. find shortest path between two nodes" << endl;
//		cout << "2. Exit"<< endl;
//		int choice;
//		cin >> choice;
//		cin.ignore();
//		string n1, n2;
//		bool exists;
//		Node* dest;
//		vector<Node*> path;
//		switch (choice) {
//		case 1: 
//			cout << "Enter the code of the first Node: " << endl;
//			getline(cin, n1);
//			cout << "Enter the code of the second Node: " << endl;
//			getline(cin, n2);
//			if (G.nodesByCode.find(n1) == G.nodesByCode.end() || G.nodesByCode.find(n2) == G.nodesByCode.end()) {
//				cout << "One or both node codes do not exist." << endl;
//				break;
//			}
//			dest = G.nodesByCode[n2];
//			exists = false;
//			if (allShortestPaths.find(n1) != allShortestPaths.end()) {
//				pair<unordered_map<Node*, double>, unordered_map<Node*, Node*>> path1 = allShortestPaths[n1];
//				if (path1.first.find(dest) != path1.first.end()) {                                                                                                                                                                                                            
//					exists = true;
//					cout << "The minimum distance from " << n1 << " to " << n2 << " is: " << path1.first[dest] << endl;
//					path = reconstructPath(dest, path1.second);
//
//					cout << "Path: ";
//					for (Node* node : path) {
//						cout << node->code << " ";
//					}
//					cout << endl;
//				}
//			}
//
//			if (!exists) {
//				cout << "There doesn't exist a path from " << n1 << " to " << n2 << endl;
//			}
//			break;
//		case 2:
//			cont = false;
//			break;
//		default:
//			break;
//		}
//	}
//	
//
//}
