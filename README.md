# Parcel Distribution System

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Technologies Used](#technologies-used)
- [Setup Instructions](#setup-instructions)
- [Usage](#usage)
- [Core Algorithms & Data Structures](#core-algorithms--data-structures)
- [Complexity](#complexity)
- [Contributing](#contributing)

## Overview
The **Parcel Distribution System** is a C++ CLI application simulating a small‐scale courier service for multiple riders. It features:
- Role‐based menus (Customer & Rider)
- Capacity and time‐window constraints
- Nearest‐neighbor assignment with 2-Opt route optimization
- Persistent state via text files
- Colored, centered UI with input validation

Ideal as an academic prototype or a starting point for small courier operations.

## Features
- **User Roles**  
  - **Customers**:  
    - Register/Login  
    - Place orders (pickup/drop-off zones & nodes, weight, optional hard deadline)  
    - Track or cancel orders by ID  
    - View order history by name & phone  
  - **Riders**:  
    - Register/Login  
    - Choose vehicle (Bike 10 kg, Car 50 kg, Truck 200 kg)  
    - View available “Placed” orders  
    - Dispatch assigned orders (up to 5 riders) using:
      - Nearest-neighbor selection respecting capacity & deadlines  
      - Dijkstra’s shortest paths for pickup→dropoff  
      - 2-Opt local search to shorten the route  
    - Persist last drop-off location across runs  
    - “Start New Day” resets clock to 10 AM  

- **Routing & Optimization**  
  - Graph loaded from `routes.txt` (153 nodes, 273 undirected edges)  
  - Precompute all-pairs shortest paths (Dijkstra + custom MinHeap)  
  - O(1) distance lookups during routing  
  - 2-Opt post-processing while preserving pickup-before-dropoff constraints  

- **UI & Input Validation**  
  - Centered, colored “boxed” menus  
  - Arrow/W-S navigation + Enter to confirm  
  - Color‐coded logs:  
    - Pickup “[+]” in white  
    - Delivery “[✓]” in green  

- **Persistence & Logging**  
  - `orders.txt`, `users.txt`, `usernames.txt`, `riderDepots.txt`, and `riderLocs.txt` 
  - Auto-create missing files

## Technologies Used
- **C++17** – Core implementation  
- **Standard Library** – `<iostream>`, `<fstream>`, `<unordered_map>`, `<vector>`, etc.  
- **Data Structures** – Graph, MinHeap, vectors, structs, etc.  

## Setup Instructions

1. **Clone** this repository:  
   ```sh
   git clone https://github.com/bismaiqbal5404/Parcel-Distribution.git
   cd ParcelDistributionSystem

## Usage
- **Login/Register**: Secure authentication for customers and riders.  
- **Place Order**: Customers select pickup/drop-off locations, weight, type, and optional deadline.  
- **Track Order**: View status and details by Order ID.  
- **Cancel Order**: Cancel a placed order before delivery.  
- **View Order History**: Customers browse past orders by name and phone.  
- **View Available Orders**: Riders see all “Placed” orders awaiting assignment.  
- **Start Delivery**: Riders choose vehicle, execute nearest‐neighbor pickups/deliveries, and run 2-Opt.  
- **Start New Day**: Reset delivery clock to 10 AM for all riders.  
- **Logout**: Exit to the main menu. 

## Core Algorithms & Data Structures

- **Graph Representation**  
  - `Node` struct:  
    - `code`, `areaName`  
    - `vector<pair<Node*,double>> neighbors`  
  - `Zone` struct:  
    - `id`, `name`  
    - `vector<Node*> nodes`  
  - `Graph` holds `unordered_map<string,Node*> nodesByCode` and `unordered_map<string,Zone*> zonesById`.

- **MinHeap (Custom Priority Queue)**  
  - Backed by an array of `(distance, Node*)` pairs.  
  - `unordered_map<Node*,int> nodePos` tracks each node’s index in the heap.  
  - Supports `insertKey`, `extractMin`, `decreaseKey`, and `deleteKey` in O(log N).

- **Dijkstra’s Algorithm**  
  - Uses the MinHeap to compute shortest paths from a single source to all nodes.  
  - Stores results as two `unordered_map<Node*,...>` for `dist` and `prev`.

- **All-Pairs Shortest Paths**  
  - Run Dijkstra once per node (N≈153).  
  - Cache `(dist, prev)` in `unordered_map<string,pair<...>> allShortestPaths`.

- **PDU Task Struct & Single‐Rider Loop**  
  ```cpp
  struct PDU {
    int ordIdx;
    Node* pickupNode;
    Node* dropoffNode;
    bool pickedUp;
    bool delivered;
    float weight;
  };
  ```
  - Build a vector of PDUs from assigned orders
  - Loop: classify each PDU (too heavy, missed deadline, feasible hard, feasible soft).
  - Select nearest feasible, run Dijkstra to target, update load/time, mark pickedUp/delivered, append to route.

- **2-Opt Route Optimization**
  - Input: vector<Node*> route (combined pickup→dropoff hops).
  - For each pair (i, k), check feasibility (pickup index < dropoff index), reverse segment if it shortens total distance.
  - Repeat until no improvement.


## Complexity
Let N = number of graph nodes (≈153), E = number of edges (≈273), m = number of orders (≤30), K = number of riders (≤5).

- **Graph Loading:**
  - O(N + E) to parse zones, nodes, and edges.

- **All-Pairs Dijkstra:**
  - N runs × O((N + E) log N) each → O(N·(N + E) log N).

- **Single-Rider Dispatch (m orders):**
  - Each order requires two Dijkstra calls (pickup + dropoff) → O(m · ((N + E) log N)).

- **PDU Loop (m tasks):**
  - Classification + selection per iteration: O(m + N).
  - Total ≈ O(m · N + m² log m) ≈ O(m²) for m≤30.

- **2-Opt Optimization (route length r ≈ 2m):**
  - Each pass: O(r³) ≈ O(m³).
  - Converges in a few passes → practical O(m³), worst O(m⁴).

- **Multi-Rider Dispatch (K riders):**
  - Sort m orders by distance: O(m log m).
  - Assign round-robin: O(m).
  - Each rider does PDU loop + 2-Opt → O(m² + m³).
  - Total across K ≈ O(K·(m² + m³)) ≈ O(m³) since K≤5.

Note: With fixed N, E, and m≤30, each “Start Delivery” completes in under a second.

## Contributors
- Mahnoor Zahra ([GitHub Profile](https://github.com/mahra110))
- Bisma Iqbal ([GitHub Profile](https://github.com/bismaiqbal5404))
- Abbas Fakhruddin ([GitHub Profile](https://github.com/Abbas-coding))