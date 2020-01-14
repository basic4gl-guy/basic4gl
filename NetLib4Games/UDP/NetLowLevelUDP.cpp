/*	NetLowLevelUDP.cpp

	Created 4-Jan-2004: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

	UDP low level networking implementation
*/

#include "Udp/NetLowLevelUDP.h"
#include "NetMiscRoutines.h"
#include "NetLog.h"

namespace NetLib4Games {

#define TIMEOUTSECS  0
#define TIMEOUTUSECS 500000

// Winsock initialisation
ThreadLock winsockLock;
int winsockRefCount = 0;

bool WinsockAddRef () {
	WSADATA wsa;
	winsockLock.Lock ();

		// Reference counted approach to initialising and shutting down
		// winsock. Once the count drops to 0, winsock library is closed again.
		NetLog ("Winsock++");
		if (winsockRefCount == 0) {
			NetLog ("Start winsock");
			if (WSAStartup (0x0202, &wsa) == SOCKET_ERROR) {
				NetLog ("Startup failed!");
				WSACleanup ();
				winsockLock.Unlock ();
				return false;
			}			
		}
		winsockRefCount++;
	winsockLock.Unlock ();
	return true;
}

void WinsockRelease () {
	winsockLock.Lock ();
		winsockRefCount--;
		NetLog ("Winsock--");		
		if (winsockRefCount == 0) {
			NetLog ("Stop winsock");
			WSACleanup ();
		}
	winsockLock.Unlock ();
}

void FlushSocket (SOCKET sock) {

    // Try to flush through any buffered messages waiting on the winsock socket.

    while (true) {

        // Look for incoming packet
        fd_set readSet;
        FD_ZERO (&readSet);
        FD_SET (sock, &readSet);
        TIMEVAL timeout;
        timeout.tv_sec	= 0;
        timeout.tv_usec = 0;
        select (0, &readSet, NULL, NULL, &timeout);

        // Exit if none
		if (!FD_ISSET (sock, &readSet))
            return;

        // Read and discard packet 
        char buffer;
		recvfrom (sock, &buffer, 1, 0, NULL, NULL);
    }
}

///////////////////////////////////////////////////////////////////////////////
//	WinsockUser
bool WinsockUser::CheckWinsock () {	
	if (!m_winsockRef) {
		m_winsockRef = WinsockAddRef ();
		return m_winsockRef;
	}
	else
		return true;
}

WinsockUser::~WinsockUser () {
	if (m_winsockRef) {
		WinsockRelease ();
		m_winsockRef = false;
	}
}

///////////////////////////////////////////////////////////////////////////////
//	NetConLowUDP
NetConLowUDP::NetConLowUDP () : 
	m_connected (false),
	m_listen (NULL),
	m_ownSocket (true),
	m_connectedEvent (true, false),
	m_buffer (NULL) {
	
	NetLog ("Create UDP client connection");
	
	// Start the socket service thread
	m_socketServiceThread.Start (this);
    if (m_socketServiceThread.Running())                         // Thread can fail to start if OS refuses to create it
        m_socketServiceThread.RaisePriority ();
    else
        SetError((std::string)"Unable to create thread. Error: " + IntToString(GetLastError()));
}

NetConLowUDP::NetConLowUDP (SOCKET sharedSocket, sockaddr_in addr, unsigned int maxPacketSize, NetListenLowUDP *listen) : 
	m_socket (sharedSocket),
	m_addr (addr),
	m_maxPacketSize (maxPacketSize),
	m_listen (listen),
	m_connected (true), 
	m_ownSocket (false),
	m_connectedEvent (true, true),
	m_buffer (NULL) {

	NetLog ("Create UDP server connection");
		
	assert (m_listen != NULL);

	// Note: Don't start the socket service thread.
	// The socket is owned and serviced by the m_listen object, which 
	// shares it between multiple connections.
}

NetConLowUDP::~NetConLowUDP () {

	// Disconnect
	Disconnect ();

	// Terminate thread
	m_socketServiceThread.Terminate ();

	// Unhook from listener
	UnhookFromListen ();

	// Clear queue
	ClearQueue ();

	// Delete temp buffer
	if (m_buffer != NULL)
		delete[] m_buffer;		

	NetLog ("Delete UDP connection");
}

void NetConLowUDP::UnhookFromListen () {
	if (m_listen != NULL) {
		m_listen->FreeNotify (this);
		m_listen = NULL;
	}
	NetLog ("Unhook from UDP listen socket");
}

void NetConLowUDP::ClearQueue () {

	// Recycle any allocated pending packets
	for (	std::list<NetSimplePacket *>::iterator i = m_pendingPackets.begin ();
			i != m_pendingPackets.end ();
			i++) 
		delete *i;
	m_pendingPackets.clear ();
}

void NetConLowUDP::Disconnect () {

	NetLog ("Disconnect UDP connection");

	m_serviceLock.Lock ();
		
	if (m_connected) {
	
		// If we own the socket, then close it down
		if (m_ownSocket) {

			NetLog ("Close socket");
			shutdown (m_socket, SD_SEND);
			closesocket (m_socket);
			m_socket = 0;
		}

		// Unhook from listener.		
		UnhookFromListen ();

		// Clear packet queue
		ClearQueue ();

		// Disconnect from listen
		m_connected = false;
		m_connectedEvent.Reset ();
		m_dataEvent.Set ();	
	}

	m_serviceLock.Unlock ();
}

bool NetConLowUDP::Connect (std::string address) {

	m_serviceLock.Lock ();
		if (Client () && !Connected ()) {

			NetLog ("Connect to: " + address);

			// Initialise winsock if necessary
			if (!CheckWinsock ()) {
				SetError ("Error initialising winsock");
				m_serviceLock.Unlock ();
				return false;
			}

			// Address format:
			// IP:port, e.g. "192.168.0.3:8000" or "somedomain.com:9999"

			// Separate address from port
			std::string addressStr, portStr;

			int pos = address.find (':');			// Look for colon
			if (pos != std::string::npos) {
				addressStr	= address.substr (0, pos);
				portStr		= address.substr (pos + 1, address.length ());
			}
			else {
				addressStr	= address;
				portStr		= "8000";
			}

			// "localhost" is special
			if (LowerCase (addressStr) == "localhost") 
				addressStr = "127.0.0.1";

			// Resolve target address
			m_addr.sin_family = AF_INET;
			m_addr.sin_port	= htons (StringToInt (portStr));

			// Try as numeric IP first
			m_addr.sin_addr.s_addr = inet_addr (addressStr.c_str ());

			// If that fails, try DNS lookup
			if (m_addr.sin_addr.s_addr == INADDR_NONE) {
				hostent *he = gethostbyname (addressStr.c_str ());
				if (he == NULL) {
					SetError ("Unable to resolve host: " + addressStr);
					m_serviceLock.Unlock ();
					return false;
				}
				m_addr.sin_addr.s_addr = *((u_long*)he->h_addr_list[0]);
			}

			// Create socket
			m_socket	= socket (AF_INET, SOCK_DGRAM, 0);
			if (m_socket == INVALID_SOCKET) {
				SetError ("Unable to create UDP socket");
				m_serviceLock.Unlock ();
				return false;
			}

			// Get the maximum packet size
			int len = sizeof (m_maxPacketSize);
			getsockopt (
				m_socket, 
				SOL_SOCKET, 
				SO_MAX_MSG_SIZE, 
				(char *) &m_maxPacketSize, 
				&len);

			NetLog ("Maximum UDP packet size: " + IntToString (m_maxPacketSize));

			// Note: At this point there in no real concept of connection,
			// handshaking, keepalives, timeouts etc.
			// Thus if the address resolved, we treat it as a successful connection.
			m_connected = true;
			m_connectedEvent.Set ();
			m_dataEvent.Set ();
		}
		bool result = Connected ();
	m_serviceLock.Unlock ();

	return result;
}

bool NetConLowUDP::Connected () {
	m_stateLock.Lock ();
		bool result = m_connected; 
	m_stateLock.Unlock ();
	return result;
}

unsigned int NetConLowUDP::MaxPacketSize () {
	m_stateLock.Lock ();
		unsigned int result = m_maxPacketSize;
	m_stateLock.Unlock ();
	return result;
}

void NetConLowUDP::Send (char *data, unsigned int size) {
	if (!m_connected)
		return;

	NetLog ("Send UDP packet, " + IntToString (size) + " bytes");

	assert (data != NULL);
	assert (size <= m_maxPacketSize);

	sendto (m_socket, data, size, 0, (sockaddr *) &m_addr, sizeof (m_addr));
}

bool NetConLowUDP::Client () {

	// Client connections own their own socket.
	// Server connections share the socket of the listener.

	// m_ownSocket set by constructor and does not change.
	// Thus thread locking is unnecessary.
	m_stateLock.Lock ();	
		bool result = m_ownSocket;
	m_stateLock.Unlock ();
	return result;
}

bool NetConLowUDP::DataPending () {

	m_inQueueLock.Lock ();
		if (!m_connected) {
			m_inQueueLock.Unlock ();
			return false;
		}

		// Note: Rather than performing the select in here, 
		// we perform the select in ::Refresh, add any received packets to 
		// the incoming queue and then return true if any packets are queued here.
		bool result = !m_pendingPackets.empty ();
	m_inQueueLock.Unlock ();
	return result;
}

unsigned int NetConLowUDP::PendingDataSize () {
	m_inQueueLock.Lock ();
		unsigned int result = DataPending () ? m_pendingPackets.front ()->size : 0;
	m_inQueueLock.Unlock ();
	return result;
}

void NetConLowUDP::ReceivePart (char *data, unsigned int offset, unsigned int &size) {
	assert (data != NULL);

	m_inQueueLock.Lock ();
		if (DataPending ()) {
			
			// Find topmost queued packet
			NetSimplePacket *packet = m_pendingPackets.front ();

			// Number of bytes to copy
			unsigned int remainingBytes = offset >= packet->size ? 0 : packet->size - offset;
			if (remainingBytes < size) 
				size = remainingBytes;

			// Copy data
			if (size > 0) 
				memcpy (data, packet->data + offset, size);
		}
		else
			size = 0;
	m_inQueueLock.Unlock ();
}

void NetConLowUDP::DonePendingData () {
	m_inQueueLock.Lock ();
		if (DataPending ()) {

			// Pop the topmost queued packet
			NetSimplePacket *packet = m_pendingPackets.front ();
			m_pendingPackets.pop_front ();

			// Delete it
			delete packet;
		}
	m_inQueueLock.Unlock ();
}

void NetConLowUDP::ThreadExecute () {

	// Always lock around all service thread activity
	// (except when waiting for events or for data on the socket.)
	// Once service is locked, we can abide by the following rule.
	//	1. State can be read without locking (must lock to read with m_stateLock)
	//	2. Access to in packet queue must be locked (with m_inQueueLock)
	//
	// Note: 1 is true because UI thread always locks state for reading with m_stateLock
	// and for writing with m_serviceLock.
	while (!m_socketServiceThread.Terminating ()) {
		m_serviceLock.Lock ();

		// If not connected, wait until we are
		if (!m_connected) {
			m_serviceLock.Unlock ();

			ThreadEvent *events [2] = { &m_connectedEvent, &m_socketServiceThread.TerminateEvent () };
			WaitForEvents (events, 2);
		}
		else {
			m_serviceLock.Unlock ();

			// Select and wait for data.
			// Note: We can't block forever, as we need to periodically check 
			// whether the thread has been terminated.
			TIMEVAL timeout;
			timeout.tv_sec	= TIMEOUTSECS;
			timeout.tv_usec = TIMEOUTUSECS;
			fd_set readSet;
			FD_ZERO (&readSet);
			FD_SET (m_socket, &readSet);
			select (0, &readSet, NULL, NULL, &timeout);

			// Found one?
			m_serviceLock.Lock ();
				if (FD_ISSET (m_socket, &readSet)) {

					// Get temp buffer for data
					if (m_buffer == NULL)
						m_buffer = new char [m_maxPacketSize];

					// Read in packet
					sockaddr_in	addr;
					int addrLen	= sizeof (addr);
					unsigned int size = recvfrom (m_socket, m_buffer, m_maxPacketSize, 0, (sockaddr *) &addr, &addrLen);
                    if (size == SOCKET_ERROR) {
                        NetLog ("Error reading UDP packet: " + IntToString (WSAGetLastError ()));
                    }
					else {

						NetLog ("Read and queue UDP packet, " + IntToString (size) + " bytes");

						// Add to end of queue
						m_inQueueLock.Lock ();
							m_pendingPackets.push_back (new NetSimplePacket (m_buffer, size));
							m_dataEvent.Set ();
						m_inQueueLock.Unlock ();
					}
				}
			m_serviceLock.Unlock ();
		}
	}
}

ThreadEvent& NetConLowUDP::Event () {
	return m_dataEvent;
}

std::string NetConLowUDP::Address() {
    return
        IntToString(m_addr.sin_addr.S_un.S_un_b.s_b1) + "." +
        IntToString(m_addr.sin_addr.S_un.S_un_b.s_b2) + "." +
        IntToString(m_addr.sin_addr.S_un.S_un_b.s_b3) + "." +
        IntToString(m_addr.sin_addr.S_un.S_un_b.s_b4);
}

///////////////////////////////////////////////////////////////////////////////
//	NetListenLowUDP
NetListenLowUDP::NetListenLowUDP (unsigned int port) : m_port (port), m_buffer (NULL), m_socket (0) {
	NetLog ("Create UDP listener, port " + IntToString (port));

    // Create socket
    OpenSocket ();

    // Start service thread
	m_socketServiceThread.Start (this);
    m_socketServiceThread.RaisePriority ();
}

NetListenLowUDP::~NetListenLowUDP () {

	NetLog ("Delete UDP listener");

	// Close down the thread
	m_socketServiceThread.Terminate ();

    // Close the socket
    CloseSocket ();

	// Delete temp buffer
	if (m_buffer != NULL)
		delete[] m_buffer;
}

void NetListenLowUDP::OpenSocket () {
    assert (m_socket == 0);

	// Ensure winsock is running
	if (!CheckWinsock ()) {
		SetError ("Error initialising winsock");
        m_socket = 0;
		return;
	}

	// Create socket
	m_socket = socket (AF_INET, SOCK_DGRAM, 0);
	if (m_socket == INVALID_SOCKET) {
		SetError ("Unable to create UDP socket");
		m_socket = 0;
		return;
	}

	// Bind to address
	m_addr.sin_family		= AF_INET;
	m_addr.sin_addr.s_addr	= INADDR_ANY;
	m_addr.sin_port			= htons (m_port);
    if (bind (m_socket, (sockaddr *) &m_addr, sizeof (m_addr)) == SOCKET_ERROR) {
		SetError ("Unable to bind to port " + IntToString (m_port));
		closesocket (m_socket);
		m_socket = 0;
		return;
	}

	// Get the maximum packet size
	int len = sizeof (m_maxPacketSize);
	getsockopt (
		m_socket,
		SOL_SOCKET,
		SO_MAX_MSG_SIZE,
		(char *) &m_maxPacketSize,
		&len);

	NetLog ("Maximum UDP packet size: " + IntToString (m_maxPacketSize));
}

void NetListenLowUDP::CloseSocket () {

    // Close the socket
	if (m_socket != 0) {
		shutdown (m_socket, SD_SEND);
		closesocket (m_socket);
        m_socket = 0;
	}

    // Close all connections using the socket
	for (	std::list<NetConLowUDP *>::iterator i = m_connections.begin ();
			i != m_connections.end ();
			i++)
		(*i)->FreeNotify (this);
	m_connections.clear ();
}

NetConLowUDP *NetListenLowUDP::FindConnection (sockaddr_in& addr) {

	// Find connection whose address matches addr
	for (	std::list<NetConLowUDP *>::iterator i = m_connections.begin ();
			i != m_connections.end ();
			i++)
		if (	(*i)->m_addr.sin_port				== addr.sin_port 
			&&	(*i)->m_addr.sin_addr.S_un.S_addr	== addr.sin_addr.S_un.S_addr)
		return *i;

	// None found
	return NULL;	
}

bool NetListenLowUDP::IsPending (sockaddr_in& addr) {

	// Find a pending connection whose address matches addr
	for (	std::list<NetPendConLowUDP *>::iterator i = m_pending.begin ();
			i != m_pending.end ();
			i++) 
		if (	(*i)->addr.sin_port				== addr.sin_port 
			&&	(*i)->addr.sin_addr.S_un.S_addr	== addr.sin_addr.S_un.S_addr) 
			return true;

	// None found
	return false;
}

bool NetListenLowUDP::ConnectionPending () {
	m_connectionLock.Lock ();
		bool result = !m_pending.empty ();
	m_connectionLock.Unlock ();
	return result;
}

std::string NetListenLowUDP::RequestString () {
	m_connectionLock.Lock ();
		assert (ConnectionPending ());
		std::string result = m_pending.front ()->requestString;
	m_connectionLock.Unlock ();
	return result;
}

NetConLow *NetListenLowUDP::AcceptConnection () {
	assert (!m_pending.empty ());

	NetLog ("Accept UDP connection");

	m_connectionLock.Lock ();

		// Extract connection request
		NetPendConLowUDP *pendConnection = m_pending.front ();
		m_pending.pop_front ();

		// Create a connection, and add to list
		NetConLowUDP *connection = new NetConLowUDP (
			m_socket, 
			pendConnection->addr, 
			m_maxPacketSize,
			this);
		m_connections.push_back (connection);

		// Queue first packet
		connection->QueuePendingPacket (new NetSimplePacket (pendConnection->packet->data, pendConnection->packet->size));
	
	m_connectionLock.Unlock ();

	// Finished with connection request
	delete pendConnection;

	return connection;
}

void NetListenLowUDP::RejectConnection () {
	assert (!m_pending.empty ());

	NetLog ("Reject UDP connection");

	m_connectionLock.Lock ();

		// Extract connection request
		NetPendConLowUDP *pendConnection = m_pending.front ();
		m_pending.pop_front ();

	m_connectionLock.Unlock ();

	// Delete request
	delete pendConnection;
}

void NetListenLowUDP::ThreadExecute () {

	while (!m_socketServiceThread.Terminating ()) {

		fd_set readSet;
		FD_ZERO (&readSet);
		FD_SET (m_socket, &readSet);
		TIMEVAL timeout;
		timeout.tv_sec	= TIMEOUTSECS;
		timeout.tv_usec = TIMEOUTUSECS;
		select (0, &readSet, NULL, NULL, &timeout);

		// Found one?
		if (FD_ISSET (m_socket, &readSet)) {

			// Get temp buffer for data
			if (m_buffer == NULL)
				m_buffer = new char [m_maxPacketSize];

			// Read in packet
			sockaddr_in	addr;
			int addrLen	= sizeof (addr);
			unsigned int size = recvfrom (m_socket, m_buffer, m_maxPacketSize, 0, (sockaddr *) &addr, &addrLen);
            if (size == SOCKET_ERROR) {
                NetLog ("Error reading UDP packet: " + IntToString (WSAGetLastError ()));

                m_connectionLock.Lock ();
                    NetConLowUDP *connection = FindConnection (addr);
                    if (connection != NULL)
                        connection->Disconnect ();
                m_connectionLock.Unlock ();
            }
            else {
                NetLog ("Read UDP packet, " + IntToString (size) + " bytes");

                // Bundle packet
                NetSimplePacket *packet = new NetSimplePacket (m_buffer, size);
                std::string requestString;

                // Find target connection (by matching against packet address)
                m_connectionLock.Lock ();

                    NetConLowUDP *connection = FindConnection (addr);
                    if (connection != NULL) {

                        NetLog ("Queue packet in UDP net connection");

                        // Queue connection packet
                        connection->QueuePendingPacket (packet);
                    }
                    // Check whether packet is a connection request
                    else if (IsConnectionRequest (packet, requestString)			// Is a connection request
                    &&	!IsPending (addr)) {									// And connection is not already pending
                        NetLog ("Create pending UDP connection");

                        // If there is no existing pending connection then create one
                        m_pending.push_back (new NetPendConLowUDP (addr, requestString, packet));
                    }
                    else {

                        // Unknown packet. Ignore it
                        NetLog ("Discard stray UDP packet");
                        delete packet;
                    }

                m_connectionLock.Unlock ();
            }
		}
	}
}

void NetListenLowUDP::FreeNotify (NetConLowUDP *connection) {
	assert (connection != NULL);

	// Remove connection from list
	m_connectionLock.Lock ();
		m_connections.remove (connection);
	m_connectionLock.Unlock ();
}

}
