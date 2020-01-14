/*	NetLowLevelUDP.h

	Created 4-Jan-2004: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

	NetLib4Games UDP low level networking implementation
*/

#ifndef _NetLowLevelUDP_h
#define _NetLowLevelUDP_h

#include <winsock2.h>
#include "NetLowLevel.h"

namespace NetLib4Games {

class NetListenLowUDP;

///////////////////////////////////////////////////////////////////////////////
//	WinsockUser
//
///	Simple base class for any object that uses Winsock.
/// CheckWinsock() ensures that winsock has been loaded before any winsock calls
/// are made. Destructor ensures that winsock is unloaded after ALL objects that
/// use it have been destroyed.

class WinsockUser {
private:
	bool m_winsockRef;
protected:
	virtual bool CheckWinsock ();
public:
	WinsockUser () : m_winsockRef (false) { ; }
	virtual ~WinsockUser ();
};

///////////////////////////////////////////////////////////////////////////////
//	NetConLowUDP
//
///	UDP/IP (ie internet) implementation of NetConLow

class NetConLowUDP : public NetConLow, public Threaded, public WinsockUser {

	// The socket operates in either of two modes.
	// It either owns the socket or shares the socket with a number of other connections.
	// If the socket is owned:
	//		m_socket is set to the socket owned by the connection
	//		m_listen is NULL
	//		::Refresh() automatically polls the socket and queues any incoming packets
	// If the socket is shared:
	//		m_socket is set to the shared socket stored in a NetListenLowUDP object
	//		m_listen points to that net listener object
	//		::Refresh() does nothing. Instead NetListenLowUDP::Refresh() will poll the 
	//		socket and pass any packets to this connection to be queued.
	
	SOCKET			m_socket;
	sockaddr_in		m_addr;				
	bool			m_connected, m_ownSocket;
	NetListenLowUDP	*m_listen;
	unsigned int	m_maxPacketSize;
	std::list<NetSimplePacket *> m_pendingPackets;
	char			*m_buffer;

	//	Thread handling
	Thread			m_socketServiceThread;
	ThreadEvent		m_connectedEvent,		// Signalled when connection is connected
					m_dataEvent;			// Signalled when data arrives
	ThreadLock		m_serviceLock, 
					m_inQueueLock,
					m_stateLock;

	/// Alternative constructor for incoming connections.
	/// Note: These connections don't store their own socket. Instead they
	/// share the socket of the NetListenLowUDP.
	NetConLowUDP (
		SOCKET sharedSocket, 
		sockaddr_in addr, 
		unsigned int maxPacketSize,
		NetListenLowUDP *listen);

	void QueuePendingPacket (NetSimplePacket *packet) {
		m_inQueueLock.Lock ();
			m_pendingPackets.push_back (packet);
			m_dataEvent.Set ();
		m_inQueueLock.Unlock ();
	}

	/// Used by NetListenLowUDP to register that it has been freed
	void FreeNotify (NetListenLowUDP *connection) {
		m_serviceLock.Lock ();
			if (m_listen == connection) {

				// We deliberately clear the listen pointer BEFORE disconnecting
				// This prevents Disconnect() from trying to notify the listener (which is unneccesary and would cause problems)
				m_listen = NULL;
				Disconnect ();
			}
		m_serviceLock.Unlock ();
	}
	void UnhookFromListen ();
	void ClearQueue (); 

protected:
	virtual void ThreadExecute ();

public:

	/// Default constructor. Connection must then be connected with ::Connect
	NetConLowUDP ();
	virtual ~NetConLowUDP ();

	/// Connect to internet address.
	/// address format:
	///		ip:port
	/// eg:
	///		192.168.0.1:8000
	/// or:
	///		somedomain.com:9999
	virtual bool Connect (std::string address);
	virtual void Disconnect ();
	virtual bool Connected ();
	virtual unsigned int MaxPacketSize ();
	virtual void Send (char *data, unsigned int size);
	virtual bool Client ();
	virtual bool DataPending ();
	virtual unsigned int PendingDataSize ();
	virtual void ReceivePart (char *data, unsigned int offset, unsigned int& size);
	virtual void DonePendingData ();	
	virtual ThreadEvent& Event ();
    virtual std::string Address();

	friend class NetListenLowUDP;
};

///////////////////////////////////////////////////////////////////////////////
//	NetPendConLowUDP
//
///	A pending NetConLowUDP connection.
struct NetPendConLowUDP {

	/// Sender address
	sockaddr_in		addr;

	/// Connection request string
	std::string		requestString;

	/// Initial packet
	NetSimplePacket *packet;

	NetPendConLowUDP (sockaddr_in _addr, std::string _requestString, NetSimplePacket *_packet) : addr (_addr), requestString (_requestString), packet (_packet) { 
		assert (packet != NULL);
	}
	~NetPendConLowUDP () { 
		delete packet;
	}
};

///////////////////////////////////////////////////////////////////////////////
//	NetListenLowUDP
//
///	UDP/IP (ie internet) implementation of NetListenLow

class NetListenLowUDP : public NetListenLow, public Threaded, public WinsockUser {

	// Listen socket
	SOCKET			m_socket;
	sockaddr_in		m_addr;
    unsigned int    m_port;
	unsigned int	m_maxPacketSize;
	char			*m_buffer;

	// Incoming connection requests
	std::list<NetPendConLowUDP *> m_pending;

	// All connections sharing m_socket
	std::list<NetConLowUDP *>	m_connections;

	// Thread handling
	Thread			m_socketServiceThread;
	ThreadLock		m_connectionLock;

	NetConLowUDP		*FindConnection (sockaddr_in& addr);
	bool				IsPending (sockaddr_in& addr);

    void                OpenSocket ();
    void                CloseSocket ();
	
protected:
	virtual void ThreadExecute ();

public:
	/// Construct listener.
	/// Note: If HasErrorState::Error () returns true, the object should
	/// be deleted immediately and not used.
	NetListenLowUDP (unsigned int port);
	virtual ~NetListenLowUDP ();

	/// Used by NetConLowUDP to notify listener of a deleted connection.
	void FreeNotify (NetConLowUDP *connection);

	virtual bool ConnectionPending ();
	virtual std::string RequestString ();		
	virtual NetConLow *AcceptConnection ();
	virtual void RejectConnection ();
};

}

#endif