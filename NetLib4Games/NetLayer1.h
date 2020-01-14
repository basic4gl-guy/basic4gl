/*	NetLayer1.h

	Created 7-Jan-2004: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

	NetLib4Games layer 1.

	Introduces basic connection concepts, plus reliable packets. Specifically:
		* Handshaking
		* Clean disconnects
		* Keep alives
		* Timeouts
		* Packet confirmation
		* General timing
*/

#ifndef _NetLayer1_h
#define _NetLayer1_h

#include "NetLowLevel.h"
#include "NetUtils.h"
#include "NetMiscRoutines.h"

// NetSettingsL1 defaults
#define NET_HANDSHAKETIMEOUT		10000		// Handshake timeout after 10 seconds inactivity
#define NET_TIMEOUT					60000		// Timeout after 30 seconds of inactivity
#define NET_KEEPALIVE				10000		// Keepalives sent every 10 seconds
#define NET_RELIABLERESEND			200			// Reliable packets resent every .2 seconds (until confirmed)
#define NET_DUP						1			// # of duplicate packets to send

// NetSettingsStaticL1 defaults
#define NET_PREFMINPACKETSIZE		64
#define NET_PREFMAXPACKETSIZE		512
#define NET_MAXSENDBUFSIZE			256			// Size of receive buffer in packets
#define NET_MAXRECVBUFSIZE			256			// Size of send buffer in packets
#define NET_RELIABLEBITBUFSIZE		4096
#define NET_UNRELIABLEBITBUFSIZE	1024

namespace NetLib4Games {

///////////////////////////////////////////////////////////////////////////////
//	NetSettingsL1
//
///	Network settings for layer 1. Can be changed once the connection is active.
struct NetSettingsL1 {

	//	All times in milliseconds

	/// Handshake timeout (ms).
	/// Allows a different (generally shorter) timeout perioud to be specified
	/// during the handshake sequence
	unsigned int handshakeTimeout;

	///	General connection timeout (ms).
	/// If no packets received within stated time, connection is dropped.
	unsigned int timeout;

	/// Keepalive period (ms).
	/// If no packets sent within stated time, a keep-alive is automatically sent.
	unsigned int keepAlive;

	/// Period after which an unconfirmed reliable packet is resent (ms)
	unsigned int reliableResend;

	/// # of duplicate packets to send
	unsigned int dup;

	NetSettingsL1 () {

		// Use default settings
		handshakeTimeout	= NET_HANDSHAKETIMEOUT;
		timeout				= NET_TIMEOUT;
		keepAlive			= NET_KEEPALIVE;
		reliableResend		= NET_RELIABLERESEND;
		dup					= NET_DUP;
	}

	NetSettingsL1 (const NetSettingsL1& s) {
		handshakeTimeout	= s.handshakeTimeout;
		timeout				= s.timeout;
		keepAlive			= s.keepAlive;
		reliableResend		= s.reliableResend;
		dup					= s.dup;
	}
};

///////////////////////////////////////////////////////////////////////////////
//	NetSettingsStaticL1
//
///	Static network settings for layer 1. Must be set before the connection is
/// created.

struct NetSettingsStaticL1 {

	/// Prefered minimum packet size.
	/// Used by packet merging code (when its implemented).
	unsigned int prefMinPacketSize;

	/// Prefered maximum packet size.
	/// Used to restrict packet sizes.
	unsigned int prefMaxPacketSize;

	/// Size of send buffer in packets. Used to queue reliable packets until 
	/// delivery is confirmed.
	unsigned int maxSendBufSize;

	/// Size of receive buffer in packets. 
	unsigned int maxRecvBufSize;

	/// Size of reliable bit buffer. (Used to track reliable packets).
	unsigned int reliableBitBufSize;

	/// Size of unreliable bit buffer. (Used to track unreliable packets).
	unsigned int unreliableBitBufSize;

	NetSettingsStaticL1 () {

		// Use default settings
		prefMinPacketSize		= NET_PREFMINPACKETSIZE;
		prefMaxPacketSize		= NET_PREFMAXPACKETSIZE;
		maxSendBufSize			= NET_MAXSENDBUFSIZE;
		maxRecvBufSize			= NET_MAXRECVBUFSIZE;
		reliableBitBufSize		= NET_RELIABLEBITBUFSIZE;
		unreliableBitBufSize	= NET_UNRELIABLEBITBUFSIZE;
	}

	NetSettingsStaticL1 (const NetSettingsStaticL1& s) {
		prefMinPacketSize		= s.prefMinPacketSize;
		prefMaxPacketSize		= s.prefMaxPacketSize;
		maxSendBufSize			= s.maxSendBufSize;
		maxRecvBufSize			= s.maxRecvBufSize;
		reliableBitBufSize		= s.reliableBitBufSize;
		unreliableBitBufSize	= s.unreliableBitBufSize;
	}
};

///////////////////////////////////////////////////////////////////////////////
//	Constants/flags/bitmasks

#define NETL1_RELIABLE		0x80
#define NETL1_RESENT        0x40        // Set if this packet is a resend of a previous packet
#define NETL1_TYPEMASK		0x3f
#define NETL1_GETTYPE(x)	((x)&NETL1_TYPEMASK)

/// Net layer 1 connection types
enum NetL1Type { 
	l1User			= 0,	// User data
	l1KeepAlive,			// Keepalive packet to prevent connection timing out
	l1Confirm,				// Confirm reliable packet received
	l1Connect,				// Request connection (client)
	l1Accept,				// Accept connection (server)
	l1Disconnect };			// Clean disconnect

///////////////////////////////////////////////////////////////////////////////
//	NetPacketHeaderL1
//
/// Layer 1 network packet header
#pragma pack(push, 1)
struct NetPacketHeaderL1 {
	byte flags;
	unsigned long id;
};
#pragma pack(pop)

///////////////////////////////////////////////////////////////////////////////
//  NetInPacketL1
//
/// Buffered incoming layer 1 network packet
struct NetInPacketL1 {
    bool            resent;     // True if packet was resent
    NetSimplePacket packet;     // Packet data
    NetInPacketL1(unsigned int size, bool _resent) : packet(size), resent(_resent) {}
};

///////////////////////////////////////////////////////////////////////////////
//	NetOutPacketL1
//
///	Outgoing layer 1 network packet
struct NetOutPacketL1 {
	unsigned int	due;		// Time at which packet is due to be (re)sent
	int				id;
	bool			reliable;
    bool            resent;
	NetSimplePacket	*packet;

	NetOutPacketL1 (unsigned int _due, NetSimplePacket *_packet) :
		due (_due),
		packet (_packet) {
		assert (packet != NULL);

		// Extract packet id
		assert (packet->size >= sizeof (NetPacketHeaderL1));
		id			= ((NetPacketHeaderL1 *) packet->data)->id;
		reliable	= (((NetPacketHeaderL1 *) packet->data)->flags & NETL1_RELIABLE) != 0;
		resent	    = (((NetPacketHeaderL1 *) packet->data)->flags & NETL1_RESENT) != 0;
	}
	~NetOutPacketL1 () {
		delete packet;
	}
};

///////////////////////////////////////////////////////////////////////////////
//	NetProcessThreadCallback
//
///	Callback interface for objects that wish to use the NetConL1's process thread
class NetProcessThreadCallback {
public:
	virtual void ProcessThreadCallback () = 0;
};

///////////////////////////////////////////////////////////////////////////////
//	NetConL1
//
///	Layer 1 network connection implementation.
/// Note: This is an internal part of the network engine. Applications generally
/// don't construct NetConL1 objects directly. Instead you should construct a
/// NetConL2 object to represent your connection.
/// See NetConL2, NetConLowUDP and NetListenLowUDP.
///
/// NetConL1 implements:
/// \li Connection handling (incl. handshake sequence, timeouts and keepalives)
/// \li Reliable packet sending.

class NetConL1 : public NetHasErrorStateThreadsafe, public Threaded {

	/// Settings that cannot be changed once the object has been constructed.
	NetSettingsStaticL1				m_settingsStatic;

	/// Settings that can be changed at any time
	NetSettingsL1					m_settings;

	/// Low level network connection object. E.g. NetConLowUDP for UDP/IP
	NetConLow						*m_connection;

//	std::list<NetSimplePacket *>	m_recvBuffer;
	std::list<NetInPacketL1 *>	    m_recvBuffer;
	std::list<NetOutPacketL1 *>		m_sendBuffer;
	bool							m_handShaking;
	int								m_sendIDReliable, 
									m_sendIDUnreliable;
	unsigned int					m_lastSent,			// Used to coordinate keepalives
									m_lastReceived;		// Used to detect timeouts

	/// Used to track which packets have already been received, and filter out duplicates.
	/// Packets can be sent multiple times (e.g. if a confirmation packet is lost,
	/// or deliberately duplicated if NetSettingsL1::dup > 1).
	NetRevolvingBitBuffer			m_reliableReceived,
									m_unreliableReceived;

	// Threading

	/// Each connection has its own thread for processing network events.
	Thread							m_processThread;

	/// Used to lock any operation that could clash with the processing thread.
	ThreadLock						m_processLock,

	/// Locks the incoming packet queue
									m_inQueueLock,

	/// Locks the outgoing packet queue
									m_outQueueLock,

	/// Used to lock any state data
									m_stateLock;

	/// Callback used to hook into the connection's processing thread.
	NetProcessThreadCallback		*m_callback;

	/// Used by thread callback to request a wakeup, for special processing
	unsigned int					m_wakeupTime;		

	void Validate ();
	void BuildHeader (	
		NetPacketHeaderL1   *header,
		bool				reliable,
		NetL1Type			type,
		unsigned long		id) {

		// Encode layer 1 packet header		
		header->id		= id;
		header->flags	= (type & NETL1_TYPEMASK) | (reliable ? NETL1_RELIABLE : 0);
	}
	void BuildAndSend (bool reliable, NetL1Type type, int id, unsigned int tickCount, bool dups = true) {
		NetPacketHeaderL1	header;
		BuildHeader (&header, reliable, type, id);
		int dupFactor = dups ? m_settings.dup : 1;
		for (int i = 0; i < dupFactor; i++) 
			m_connection->Send ((char *) &header, sizeof (header));
		m_stateLock.Lock ();
			m_lastSent = tickCount;
		m_stateLock.Unlock ();
	}
	bool ProcessRecvPacket (unsigned int tickCount);

protected:
	virtual void ThreadExecute ();

public:

	/// Construct a network connection with supplied settings
	NetConL1 (NetConLow *connection, NetSettingsStaticL1& settings);

	/// Construct a network connection with default settings
	NetConL1 (NetConLow *connection);

	/// Destroy the connection.
	/// Will automatically clean disconnect (if there is no error), or 
	/// simply drop the connection otherwise.
	virtual ~NetConL1 ();

	// Member access
	
	/// Read/write connection settings
	NetSettingsL1	Settings ()		{ 

		// Locking unnecessary on read, as only UI thread writes to settings
		return m_settings;
	}
	void SetSettings (NetSettingsL1& settings) {
		m_processLock.Lock ();
			m_settings = settings;
		m_processLock.Unlock ();
	}

	/// Return true if connection is a client connection, false if is a server connection.
	bool			HandShaking ()	{ 
		m_stateLock.Lock ();
			bool result = m_handShaking; 
		m_stateLock.Unlock ();
		return result;
	}

	/// Connect to address, passing request string.
	/// Request string will be returned by NetListenLow::RequestString on the server
	/// when the connection request is received (NetListenLow::ConnectionPending() == true)
	bool Connect (std::string address, std::string requestString);	

	/// Connect to address, passing a blank request string ("").
	bool Connect (std::string address) { return Connect (address, ""); }
	
	/// Disconnect.
	/// \param clean	is true to attempt to disconnect cleanly. If successful,
	///					the other connection will be notified of the disconnect.
	///					If clean is false, the other connection will not be 
	///					notified and will simply receive no responses from this
	///					connection until it times out.
	void Disconnect (bool clean);

	///	Note: Connection is considered connected even in the handshaking stage.
	bool Connected ();

	/// True if a data packet is pending
	bool DataPending ();

	/// Size of the pending data packet in bytes
	unsigned int PendingDataSize ();

    /// Returns true if the pending packet was resent.
    /// Call this ONLY if DataPending() returns true.
    bool PendingIsResent();

	/// Receive part of the pending data packet.
	/// PendingDataSize() must return true before you call this method.
	void ReceivePart (char *data, unsigned int offset, unsigned int& size);
	
	/// Receive a pending packet and signal we have finished with it
	void Receive (char *data, unsigned int &size) {
		m_inQueueLock.Lock ();
			ReceivePart (data, 0, size);
			DonePendingData ();
		m_inQueueLock.Unlock ();
	}

	/// Signal that we have finished with a pending data packet.
	void DonePendingData ();	

	/// Maximum number of bytes that can be sent in a single packet.
	/// (Not counting header data.)
	unsigned int MaxPacketSize () { 

		// Allow room for our header
		return Min (m_settingsStatic.prefMaxPacketSize, m_connection->MaxPacketSize ()) - sizeof (NetPacketHeaderL1);
	}

	/// Send a packet of data.
	/// \param reliable if true, the packet will be sent continually until a 
	///					confirmation is received. If false, the packet will
	///					be sent once (or ::Settings().dup times) and forgotten 
	///					about.
	virtual void Send (char *data, unsigned int size, bool reliable);

	void StartThread () {
		m_processThread.Start (this);
//        m_processThread.RaisePriority ();
	}

	///	Hook into callback mechanism.
	///	Note: Any object that calls this method should be sure to 
	/// later NULL out the callback before destruction.
	void HookCallback (NetProcessThreadCallback *callback) {
		m_processLock.Lock ();
			m_callback = callback;
		m_processLock.Unlock ();
	}

	/// Lock out the callback thread.
	/// Use to ensure the callback code (setup via HookCallback) is not running.
	void LockCallback () {
		m_processLock.Lock ();
	}
	void UnlockCallback () {
		m_processLock.Unlock ();
	}

	/// Request that m_callback be called in msec milliseconds.
	///	The m_callback will then be called on the connections process thread.
	void RequestWakeup (unsigned int msec) {

		// Must only be used by objects using the thread callback mechanism,
		// (and must only be called during that callback, from that thread.)
		if (m_wakeupTime == INFINITE || msec < m_wakeupTime)
			m_wakeupTime = msec;
	}

    std::string Address() { return m_connection->Address(); }
};

///////////////////////////////////////////////////////////////////////////////
//	NetConReqValidatorL1
//
///	Implements NetConReqValidator.
class NetConReqValidatorL1 : public NetConReqValidator{
public:
	virtual bool IsConnectionRequest (NetSimplePacket *packet, std::string& requestString);
};

}

#endif
		