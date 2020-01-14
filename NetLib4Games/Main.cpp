/*	NetLib4Games example. 

	Simple server or client network example.

	Demonstrates establishing a connection and sending and receiving data.

	Usage:	
	
		NetEngine

			Will run the demo in server mode.

		NetEngine address

			Will run the demo in client mode, e.g: NetEngine localhost:8000
*/

#include "UDP/NetLowLevelUDP.h"		// UDP/IP specific component
#include "NetLayer2.h"				// Main library header
#include <iostream>

using namespace NetLib4Games;
using namespace std;

#define PORT 8000

int main(int argc, char *argv[]) {

	// Network objects
	NetListenLow *listener = NULL;
	NetConL2 *connection = NULL;

	// No arguments = server mode.
	// 1 argument = client mode, where argument is address to connect to in format:
	//	address:port
	//
	if (argc == 1) {
		cout << "Server mode" << endl;

		// Create network listener
		listener = new NetListenLowUDP(8000);

		// Wait for connection
		cout << "Waiting for connection" << endl;
		while (!listener->ConnectionPending())
			Sleep(50);

		// Accept connection
		cout << "Accepting connection" << endl;
		connection = new NetConL2(listener->AcceptConnection());

		// Run a simple echo server
		while (connection->Connected()) {
			if (connection->DataPending()) {

				// Read in pending message
				char buffer[256];
				unsigned int size = 256;
				if (size > connection->PendingDataSize())
					size = connection->PendingDataSize();
				connection->Receive(buffer, size);

				cout << "Received message: " << buffer << endl;
			
				// Echo it back
				connection->Send(buffer, size, 0, true, false);
				cout << "Echoed back" << endl;
			}
			Sleep(10);
		}

		cout << "Server done" << endl;
	}
	else {
		cout << "Client mode" << endl;

		// Create network connection
		connection = new NetConL2(new NetConLowUDP());

		// Connect to server
		cout << "Connecting to " << argv[1] << endl;
		connection->Connect(argv[1]);

		// Wait for acceptance
		while (connection->Connected() && connection->HandShaking())
			Sleep(50);

		if (connection->Connected()) {
			cout << "Connection established" << endl;

			cout << "Enter a string to bounce back, or \"quit\" to quit" << endl;

			bool done = false;
			while (!done) {
				char buffer[256];
				cout << ">";
				cin.getline(buffer, 255);
				if (strcmp(buffer, "quit") == 0)
					done = true;
				else if (strlen(buffer) > 0) {

					// Send data to server
					connection->Send(buffer, strlen(buffer) + 1, 0, true, false);

					// Wait for reply
					bool received = false;
					for (int i = 0; i < 50 && !received; i++) {
						Sleep(100);
						if (connection->DataPending()) {

							// Receive reply
							char echoBuffer[256];
							unsigned int size = 255;
							if (size > connection->PendingDataSize())
								size = connection->PendingDataSize();
							connection->Receive(echoBuffer, size);

							cout << "Received: " << echoBuffer << endl;
							received = true;
						}
					}
				}
			}		
		}
		else
			cout << "Connection failed.";
	}

	// Delete network objects
	delete connection;
	if (listener != NULL)
		delete listener;

	return 0;
}