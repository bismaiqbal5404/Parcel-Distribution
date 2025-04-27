#include "Rider.h"

Rider::Rider(string uname, string pass, string r)
    : User(uname, pass, r) { // call parent (User) constructor
}

void Rider::riderMenu() {
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
            startDelivery(); 
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

void Rider::startDelivery() {
    int option;
    do {
        cout << "\n--- Start Delivery ---\n";
        cout << "1. Choose Vehicle\n";
        cout << "2. View Shortest Route\n";
        cout << "3. Back to Rider Menu\n";
        cout << "Enter your option: ";
        cin >> option;
        cin.ignore();

        switch (option) {
        case 1:
            // Implement chooseVehicle();
            cout << "Vehicle chosen.\n";
            break;
        case 2:
            // Implement viewShortestRoute();
            cout << "Displaying shortest route.\n";
            break;
        case 3:
            cout << "Returning to Rider Menu...\n";
            break;
        default:
            cout << "Invalid option. Try again.\n";
        }
    } while (option != 3);
}
