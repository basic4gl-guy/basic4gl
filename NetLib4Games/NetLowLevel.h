/*	NetLowLevel.h
	
	Created 4-Jan-2005: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

	NetLib4Games abstract low level networking classes.
*/

#ifndef _NetLowLevel_h
#define _NetLowLevel_h

#include <assert.h>
#include <list>
#include "NetHasErrorState.h"
#include "NetThreadUtils.h"

namespace NetLib4Games {

///////////////////////////////////////////////////////////////////////////////
//	NetHasErrorState
//
///	Implementation of HasErrorState with automatic logging

class NetHasErrorState : public NetLib4Games::HasErrorState {
protected:

	/// Set network error state.
	/// Also automatically logs the error (if logging is enabled.)
    virtual void SetError (std::string text);
};

class NetHasErrorStateThreadsafe : public HasErrorStateThreadsafe {
protected:

	/// Set network error state.
	/// Also automatically logs the error (if logging is enabled.)
    virtual void SetError (std::string text);

};

///////////////////////////////////////////////////////////////////////////////
//	NetSimplePacket
//
/// Very simple network packet.
/// This is just a helper object. The network implementation may choose to 
/// use it or ignore it use their own format internally.

struct NetSimplePacket {
	char *data;
	unsigned int size;

	NetSimplePacket (unsigned int _size) {
		size = _size;
		if (size > 0)
			data = new char [size];
		else
			data = NULL;
	}
	NetSimplePacket (char *_data, unsigned int _size) {
		assert (_data == NULL && _size == 0 || _data != NULL);
		size = _size;
		if (size > 0) {
			data = new char [size];
			memcpy (data, _data, size);
		}
		else 
			data = NULL;
	}	
	~NetSimplePacket () {
		if (data != NULL)
			delete[] data;
	}
};

///////////////////////////////////////////////////////////////////////////////
//	NetRefreshed
//
///	Any network object that needs to be refreshed periodically.

class NetRefreshed {
public:
	/// Refresh internal state
	virtual void Refresh () = 0;	
};

///////////////////////////////////////////////////////////////////////////////
//	NetConLow
//
///	Low level network connection.
/// All protocol specific logic is bundled into an object that descends from
/// this class. For example, the NetConLowUDP class contains all the UDP/IP
/// specific code.
///
/// The NetConLow object is passed to the constructor of the NetConL2 (i.e.
/// the main connection object of NetLib4Games).
/// So a UDP/IP NetLib4Games connection would be created like so:\code
/// NetConL2 *connection = new NetConL2(new NetConLowUDP());
/// \endcode
/// Other NetConLow descendants can support other low level network protocols.
/// (At the time of writing, only UDP/IP is supported.)
///
/// NetConLow objects are also created by NetListenLow::AcceptConnection(), 
/// when a connection request is accepted by the server.

/*
Note: Typically applications don't use the NetConLow (descendant) directly.
Instead they pass it to a NetConLx network connection, which adds extra
functionality (like connection lifetime, reliability, ordering, multi-packet
messages, timing etc).
The NetConLow simply supports unreliable packet sending and receiving to/from
a set target address.

For multithreaded support, the must object allow that the ::Refresh() method
(and anything that ::Refresh() itself calls) will likely be called from a 
different thread than the other methods, particularly ::Send() and ::Receive().
In some low level implementations, server connections may be driven by the
NetListenLow object (such as with the UDP implementation), which would also 
be in a different thread if multithreading is used.

  A recommended approach for threadsafeness is to either:
	* Lock the entire object around all method calls,
  OR
	* Lock access to:
		1. The send queue
		2. The receive queue
		3. The underlying networking object(s) (eg. the socket for UDP implementation)
*/

class NetConLow : public HasErrorStateThreadsafe/*, public NetRefreshed*/ {
public:
	virtual ~NetConLow ();

	/// Return true if connection is a client connection, or false if is a server connection.	
	virtual bool Client () = 0;

	/// Connect to address.
	/// \param address address to connect to. Meaning depends on underlying
	/// communication protocol. Eg. for UDP (internet) this would be a DNS 
	/// or IP address.
	/// Note: Not all connections are connected this way. They are also created
	/// in response to external connection requests.
	virtual bool Connect (std::string address) = 0;	

	/// Disconnect
	virtual void Disconnect () = 0;		

	/// True if still connected.
	virtual bool Connected () = 0;

	/// Maximum packet size.
	/// Can only be called on connected connections
	virtual unsigned int MaxPacketSize () = 0;

	/// Send a packet to the destination address.
	/// \param flags not used in layer 1. Reserved for higher layers.
	/// If not connected, should simply do nothing.
	virtual void Send (char *data, unsigned int size) = 0;

	/// True if a data is waiting to be received.
	/// If not connected, should simply return false.
	virtual bool DataPending () = 0;

	/// Size of pending data
	virtual unsigned int PendingDataSize () = 0;

	/// Receive pending data.
	/// Can only be called on connected connections when DataPending() = true
	/// \param data buffer to receive data
	/// \param size In = Amount of room in data, Out = # of bytes read.
	/// Data will be truncated if it doesn't fit in buffer.	
	void Receive (char *data, unsigned int &size) {
		ReceivePart (data, 0, size);
		DonePendingData ();
	}

	/// Receive part of a pending packet.
	/// Can only be called on connected connections when PacketPending() = true
	/// \param data buffer to receive data
	/// \param size In = Amount of room in data, Out = # of bytes read.
	/// The packet remains in the receive queue until ::NextPacket is called
	virtual void ReceivePart (char *data, unsigned int offset, unsigned int& size) = 0;

	/// Discard the top-of-queue pending packet
	virtual void DonePendingData () = 0;	

	/// Event object.
	/// Event is signalled whenever data is received, or a significant state
	/// change occurs, such as a network error or disconnect.
	virtual ThreadEvent& Event () = 0;

    /// Expose the destination address of a connection to the application.
    virtual std::string Address() = 0;
};

class NetConReqValidator;

///////////////////////////////////////////////////////////////////////////////
//	NetListenLow
//
///	Abstract low level network listener.
/// Listens on the network for connection requests (ConnectionPending() = true) 
/// and can accept (AcceptConnection()) them - to create a NetConL2 network 
/// connection - or reject them.
/// 
/// Use the appropriate descendant class for the underlying network protocol
/// you wish to use.
/// E.g. the UDP/IP implementation is NetListenLowUDP.
/// 
/// IMPORTANT:
/// For some NetListenLow implementations, the NetListenLow object itself handles
/// maintaining the underlying network state.
/// Therefore you should NOT destroy the NetListenLow object while you have 
/// NetConL2 connections that have been accepted by it.

class NetListenLow : public HasErrorStateThreadsafe {
public:
	virtual ~NetListenLow ();

	/// True if a client connection is pending.
	/// That is, if a connection request has been received in response to a
	/// client NetConL2::Connect call.
	virtual bool ConnectionPending () = 0;

	/// Read the request string for the current pending connection.
	/// (i.e. the second parameter of the client's NetConL2::Connect call.)
	/// Call this ONLY if ConnectionPending() returns true.
	virtual std::string RequestString () = 0;		

	/// Accept and return pending connection (see ::ConnectionPending).
	/// Call this ONLY if ConnectionPending() returns true.
	/// Returns a low level network protocol connection that can be passed to 
	/// the constructor of a NetConL2 connection. E.g.:\code
	/// if (listener.ConnectionPending()) {
	///		NetConL2 *connection = new NetConL2(listener.AcceptConnection());
	/// 	...
	///	}
	/// \endcode
	virtual NetConLow *AcceptConnection () = 0;

	/// Reject the pending connection (see ConnectionPending())
	/// Call this ONLY if ConnectionPending() returns true.
	virtual void RejectConnection () = 0;

	/// [Used internally]
	bool IsConnectionRequest (NetSimplePacket *packet, std::string& requestString);
};

///////////////////////////////////////////////////////////////////////////////
//	NetConReqValidator
//
///	Abstract base class for an object that validates connection requests.
///	Because we don't interpret the contents of packets at this layer, we leave
/// a hook open to call a higher layer object to tell us whether we have
/// received a valid connection request (rather than just a stray packet).
class NetConReqValidator {
public:
	NetConReqValidator ();
	virtual ~NetConReqValidator ();
	virtual bool IsConnectionRequest (NetSimplePacket *packet, std::string& requestString) = 0;
};

}

#endif