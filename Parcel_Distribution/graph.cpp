//#include<iostream>
//#include<string>
//#include<fstream>
//#include<sstream>
//#include<vector>
//#include<queue>
//#include<unordered_map>
//using namespace std;
//
//struct Node {
//	string code;
//	string areaName;
//	vector<pair<Node*, double>> neighbors;
//};
//
//struct Zone {
//	string id;
//	string name;
//	vector<Node*> nodes;
//};
//
//struct Graph {
//	unordered_map<string, Node*> nodesByCode;
//	unordered_map<string, Zone*> zonesById;
//};
//
//static string trim(const string& s) {
//	int start = 0;
//	int end = static_cast<int>(s.size()) - 1;
//	while (start <= end && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r' || s[start] == '\n')){
//		start++;
//	}
//	while (end >= start && (s[end] == ' ' || s[end] == '\t' || s[end] == '\r' || s[end] == '\n')) {
//		end--;
//	}
//	return (start > end ? string() : s.substr(start, end - start + 1));
//}
//
//static string unquote(const string& s) {
//	if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
//		return s.substr(1, s.size() - 2);
//	return s;
//}
//
//void loadGraphFromFile(const string& path, Graph& G) {
//	ifstream in(path);
//	string line, currentZoneId;
//
//	while (getline(in, line)) {
//		line = trim(line);
//		if (line.empty() || line[0] == '#') continue;
//		istringstream ss(line);
//		string kw;
//		ss >> kw;
//		if (kw == "ZONE") {
//			ss >> currentZoneId;
//			string rest;
//			getline(ss, rest);
//			rest = trim(rest);
//			Zone* z = new Zone{ currentZoneId, unquote(rest), {} };
//			G.zonesById[currentZoneId] = z;
//		}
//		else if (kw == "NODE") {
//			string code;
//			ss >> code;
//			string rest;
//			getline(ss, rest);
//			rest = trim(rest);
//			Node* n = new Node{ code, unquote(rest), {} };
//			G.nodesByCode[code] = n;
//			auto zit = G.zonesById.find(currentZoneId);
//			if (zit == G.zonesById.end()) {
//				cerr << "Warning: NODE '" << code << "' declared before any ZONE\n";
//			}
//			else {
//				zit->second->nodes.push_back(n);
//			}
//		}
//		else if (kw == "EDGE") {
//			std::string a, b;
//			double d;
//			ss >> a >> b >> d;
//			auto it1 = G.nodesByCode.find(a);
//			auto it2 = G.nodesByCode.find(b);
//			if (it1 == G.nodesByCode.end() || it2 == G.nodesByCode.end()) {
//				std::cerr << "Warning: EDGE references undefined node '" << a << "' or '" << b << "'\n";
//				continue;
//			}
//			Node* n1 = it1->second;
//			Node* n2 = it2->second;
//			n1->neighbors.emplace_back(n2, d);
//			n2->neighbors.emplace_back(n1, d);
//		}
//
//	}
//}
//
//static unordered_map<Node*, double> dijkstra(int V ,string sourceCode, Graph G) {
//	priority_queue<pair<double, Node*>, vector<pair<double, Node*>>, greater<pair<double, Node*>>> pq;
//
//	unordered_map<Node*, double> dist;
//
//	Node* src = G.nodesByCode[sourceCode];
//	pq.push({ 0.0, src });
//	dist[src] = 0.0;
//
//	while (!pq.empty()) {
//		Node* u = pq.top().second;
//		double currentDist = pq.top().first;
//		pq.pop();
//
//		for (auto& neighbor : u->neighbors) {
//			Node* nextNode = neighbor.first;
//			double edgeCost = neighbor.second;
//			double newDist = currentDist + edgeCost;
//
//			if (!dist.count(nextNode) || dist[nextNode] > newDist) {
//				dist[nextNode] = newDist;
//				pq.push({ newDist, nextNode });
//			}
//		}
//	}
//	return dist;
//}
//
//void main() {
//	string fileName = "routes.txt";
//	Graph G;
//	loadGraphFromFile(fileName, G);
//
//	cout << "Nodes Loaded: " << G.nodesByCode.size() << endl;
//	cout << "Zones loaded: " << G.zonesById.size() << endl;
//
//	for (auto& pair : G.nodesByCode) {
//		Node* n = pair.second;
//		cout << n->code << " ( " << n->areaName << ") " << n->neighbors.size() << " neighbors\n";
//	}
//	unordered_map<Node*, double> result = dijkstra(G.nodesByCode.size(), "A1", G);
//	for (auto& dist : result)
//		cout << "The distance from A1 to " << dist.first->code << " is " << dist.second << " " << endl;
//
//}