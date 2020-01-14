/*	NetLayer2.h

	Created 10-Feb-2005: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

	NetLib4Games layer 2.

	Introduces:
		* Messages (data that spans multiple packets)
		* Ordering
		* Smoothing
		* Channels

	Messages are sent through channels.
	Each channel can be either ordered or unordered. Within a channel, messages
	can be critical or uncritical and smoothed or unsmoothed.

	Messages sent through an ordered channel are guaranteed to be received in 
	the same order. This does not apply accross two different channels however.
*/

#ifndef _NetLayer2_h
#define _NetLayer2_h

#include "NetLayer1.h"

#define NET_L2TIMINGBUFFERSIZE 25

// NetSettingsStaticL2 defaults
#define NET_L2BUFFERPACKETS 1024

// NetSettingsL2 defaults
#define NET_L2SMOOTHINGPERCENTAGE 80

namespace NetLib4Games {

///////////////////////////////////////////////////////////////////////////////
//	NetSettingsStaticL2
//
///	Static network settings for layer 2. Must be set before the NetConL2 
/// connection is created. (Defaults will be used if not explicitly supplied.)

struct NetSettingsStaticL2 {

	/// Size of Layer 2 buffer in packets.
	/// Once the buffer reaches this size, unreliable messages will be removed
	/// from the oldest first. If there are no unreliable messages in the buffer
	/// then we have a network error.
	/// Due to the nature of unreliable packets, we expect broken unreliable 
	/// packets to persist in the buffer until this mechanism deletes them.
	unsigned int maxBufferPackets;

	/// Layer one settings. (Every NetConL2 contains an internal NetConL1. 
	/// these are the settings specific to that layer.)
	NetSettingsStaticL1 l1Settings;

	NetSettingsStaticL2 () {

		// Use default settings
		maxBufferPackets	= NET_L2BUFFERPACKETS;
	}

	NetSettingsStaticL2 (const NetSettingsStaticL2& s) {
		maxBufferPackets	= s.maxBufferPackets;
		l1Settings			= s.l1Settings;
	}
};

///////////////////////////////////////////////////////////////////////////////
//	NetSettingsL2
//
///	Network settings for NetConL2. Unlike NetSettingsStaticL2, these can be
/// changed after the NetConL2 has been constructed.
struct NetSettingsL2 {

	///	Smoothing delay is adjusted so that this percentage of packets are 
	/// considered to be on-time or early.
	int smoothingPercentage;

	NetSettingsL2 () {

		// Use default settings
		smoothingPercentage	= NET_L2SMOOTHINGPERCENTAGE;
	}

	NetSettingsL2 (const NetSettingsL2& s) {
		smoothingPercentage = s.smoothingPercentage;
	}
};

///////////////////////////////////////////////////////////////////////////////
//	Constants/flags/bitmasks

#define NETL2_RELIABLE		0x80
#define NETL2_SMOOTHED		0x40
#define NETL2_ORDERED		0x20
#define NETL2_CHANNELMASK	0x1f
#define NETL2_GETCHANNEL(x) ((x)&NETL2_CHANNELMASK)
#define NETL2_MAXCHANNELS	32

///////////////////////////////////////////////////////////////////////////////
//	NetPacketHeaderL2
//
///	Layer 2 network packet header
#pragma pack(push,1)
struct NetPacketHeaderL2 {
	byte			channelFlags;
	unsigned long	messageIndex;
	unsigned long	reliableIndex;
	unsigned short	packetCount;
	unsigned short	packetIndex;
	unsigned int	tickCount;			// Sender's GetTickCount () at time of sending
};
#pragma pack(pop)

///////////////////////////////////////////////////////////////////////////////
//	NetMessageL2
//
///	Layer 2 network message
struct NetMessageL2 {
	int						channel;
	bool					reliable;
	bool					ordered;
    bool                    smoothed;
	unsigned long			messageIndex;
	unsigned long			reliableIndex;
	unsigned short			packetCount;
	unsigned int			tickCount;
	unsigned short			receivedCount;
	int						dataSize;
	bool					tickCountRegistered;
	NetSimplePacket			**packets;

	NetMessageL2 (	int _channel, 
					bool _reliable, 
					bool _smoothed, 
					bool _ordered, 
					unsigned long _messageIndex,
					unsigned long _reliableIndex,
					unsigned short _packetCount,
					unsigned int _tickCount);
	~NetMessageL2 () {

		// Delete packets
		if (packets != NULL) {

			// Delete assigned packets
			for (int i = 0; i < packetCount; i++) 
				if (packets [i] != NULL)
					delete packets [i];

			// Delete packet array
			delete[] packets;
		}
	}

	bool Complete () { return receivedCount >= packetCount; }

	bool Buffer (NetSimplePacket *packet, int packetIndex);
	void CopyData (char *data, unsigned int offset, unsigned int& size);
};

///////////////////////////////////////////////////////////////////////////////
//	NetOutChannelL2
//
///	Layer 2 outgoing network channel.
class NetOutChannelL2 : public HasErrorState {
	int				m_channel;
	bool			m_ordered;
	unsigned long	m_messageIndex,
					m_reliableIndex;
public:
	NetOutChannelL2 (int channel, bool ordered) :
		m_channel		(channel),
		m_ordered		(ordered),
		m_messageIndex	(0),
		m_reliableIndex (0) { ; }

	// Member access
	int				Channel ()			{ return m_channel; }
	bool			Ordered ()			{ return m_ordered; }
	unsigned long	MessageIndex ()		{ return m_messageIndex; }
	unsigned long	ReliableIndex ()	{ return m_reliableIndex; }

	void Send (
		NetConL1&		connection, 
		const char		*data, 
		int				size, 
		bool			reliable,
		bool			smoothed,
		unsigned int	tickCount);
};

///////////////////////////////////////////////////////////////////////////////
//	NetInChannelL2
//
///	Layer 2 incoming network channel.

class NetInChannelL2Callback {
public:
	virtual void QueueMessage (NetMessageL2 *msg) = 0;
	virtual void RegisterTickCountDifference (int difference) = 0;
	virtual void RequestWakeup (unsigned int msec) = 0;
};

class NetInChannelL2 : public HasErrorState {

	/// True if channel is ordered.
	/// Ordered channels guarantee delivery of messages in order sent.
	int							m_channel;
	bool						m_ordered;
	unsigned long				m_messageIndex,
								m_reliableIndex;
	int							m_packetCount;
	std::list<NetMessageL2 *>	m_messages;
	unsigned int				m_maxBufferPackets;

	void Clear ();

public:
	NetInChannelL2 (int channel, bool ordered, unsigned int maxBufferPackets) : m_channel (channel), m_ordered (ordered), m_maxBufferPackets (maxBufferPackets), m_messageIndex (0), m_reliableIndex (0), m_packetCount (0) { ; }
	~NetInChannelL2 () { Clear (); }

	// Member access
	int		Channel () { return m_channel; }
	bool	Ordered () { return m_ordered; }

	void Buffer (NetSimplePacket *packet, bool reliable, bool smoothed, bool resent, int messageIndex, int reliableIndex, int packetIndex, int packetCount, unsigned int tickCount);
	void PromoteMessages (NetInChannelL2Callback *callback, unsigned int tickCount, int adjustment, bool doSmoothing);
	void CullMessages ();
};

///////////////////////////////////////////////////////////////////////////////
//	NetTimingBufferL2
//
///	Sorts and stores timing differences on recent messages.
///	The timing difference =
///		Sender.GetTickCount () at the time of sending the packet
///		-
///		Receiver.GetTickCount () at the time the packet is received.
///
/// The timing buffer is used to calculate when a message is considered early
/// or late.
class NetTimingBufferL2 {
	int		m_receivedIndex [NET_L2TIMINGBUFFERSIZE];
	int		m_sortedIndex [NET_L2TIMINGBUFFERSIZE];
	int		m_receivedPos;
	bool	m_bufferFull;

	unsigned int	SortedPos (int difference);

public:
	NetTimingBufferL2 () : m_bufferFull (false), m_receivedPos (0) { ; }

	void Clear () {
		m_bufferFull = false;
		m_receivedPos = 0;
	}
	void LogDifference (int difference);
	int Difference (unsigned int i) {
		assert (m_bufferFull);
		assert (i < NET_L2TIMINGBUFFERSIZE);
		return m_sortedIndex [i];
	}
	bool BufferFull () {
		return m_bufferFull;
	}		
};

///////////////////////////////////////////////////////////////////////////////
//	NetConL2
//
///	Layer 2 network connection implementation.
/// This is the main network connection of NetLib4Games. 
/// For every network connection there will be one of these objects constructed
/// on the client, and another one on the server that is constructed when the 
/// connection is accepted.
///
/// (NetLib4Games was built up in layers, where each layer sits atop the previous 
/// one and introduces new functionality. It just happened that by layer 2 it
/// was functional enough to be useful. Therefore all connection objects are 
/// NetConL2!)
///
class NetConL2 : public HasErrorState, public NetProcessThreadCallback, public NetInChannelL2Callback {

	// Underlying layer 1 connection
	NetConL1					m_connection;

	// Channels
	NetOutChannelL2				*m_outChannels [NETL2_MAXCHANNELS];
	NetInChannelL2				*m_inChannels  [NETL2_MAXCHANNELS];
	std::list<NetMessageL2 *>	m_messageQueue;

	// Layer 2 settings
	NetSettingsL2				m_settings;
	NetSettingsStaticL2			m_settingsStatic;

	// Thread handling
	ThreadLock					m_inQueueLock;

	// Timing
	NetTimingBufferL2			m_timingBuffer;

	void Init ();
	void CheckError () {
		m_connection.LockError ();
		if (m_connection.Error ()) {
			SetError ("Layer 1: " + m_connection.GetError ());
			m_connection.UnlockError ();
			if (Connected ())
				Disconnect (true);
		}
		else
			m_connection.UnlockError ();
	}
	void CheckObject (HasErrorState *obj) {
		if (obj->Error ()) {
			SetError (obj->GetError ());
			if (Connected ())
				Disconnect (true);
		}
	}
	void CheckObject (HasErrorStateThreadsafe *obj) {
		obj->LockError ();
		if (obj->Error ()) {
			SetError (obj->GetError ());
			obj->UnlockError ();
			if (Connected ())
				Disconnect (true);
		}
		else
			obj->UnlockError ();
	}

public:

	/// Construct a new network connection.
	/// \param connection Is a low level NetConLow object that handles
	/// the network protocol specific part of the network connection.
	///	(The UDP/IP version is NetConLowUDP.)
	/// \param settings (optional) the static network settings.
	/// The static settings cannot be changed once the connection is 
	/// constructed. (Unlike the L1Settings() and L2Settings() which 
	/// can be.)	
	///
	/// For client NetConL2 objects, the connection parameter is typically 
	/// constructed at the same time, like so:\code
	/// NetConL2 *clientConnection = new NetConL2(new NetConLowUDP());
	/// \endcode
	/// The NetConL2 is constructed as unconnected and must be connected
	/// by calling Connect().
	/// 
	/// For server connections NetListenLow::AcceptConnection() returns a 
	/// low level connection object that can be passed to the constructor like so:\code
	/// if (listener->ConnectionPending()) {
	///		NetConL2 *serverConnection = new NetConL2(listener->AcceptConnection());
	///		...
	///	\endcode
	/// This constructs a NetConL2 representing the newly accepted connection.
	/// The NetConL2 is constructed as connected. You do not need to call
	/// Connect().
	NetConL2 (NetConLow *connection, NetSettingsStaticL2& settings);
	NetConL2 (NetConLow *connection);
	~NetConL2 ();

	// Passed through to layer 1 connection

	/// Returns true if the connection is in handshaking stage.
	/// The handshaking stage applies to client connections only, and 
	/// refers to the stage where the client is looking for the remote
	/// server and waiting for a connection accepted/rejected response
	/// (or a timeout).
	///
	/// While handshaking:
	///	\li Connected() returns true!
	///	\li	HandShaking() returns true
	///	
	/// After a handshake failed:
	///	\li Connected() returns false
	///	\li HandShaking() is undefined
	///
	/// After a handshake succeeded:
	///	\li Connected() returns true
	/// \li HandShaking() returns false
	///
	/// Therefore to wait for handshaking to complete (over-simplified example): \code
	/// while (connection.Connected() && connection.HandShaking())
	///		Sleep(50);
	///
	/// if (connection.Connected()) {
	///		// Connection accepted
	///		...
	/// } 
	/// else {
	///		// Connection rejected
	///		...
	/// }
	/// \endcode
	/// 
	/// Note: It is not strictly necessary to wait for the handshaking stage to complete.
	/// You can also simply proceed as if the connection was successful.
	/// No network messages will be received by the connection while in handshaking
	/// stage, and any messages sent will be automatically queued, and then sent
	/// when (and if) the connection is accepted.
	///
	/// The main consequence of simply proceeding is that if the connection fails, it
	/// will appear to have succeeded for a few seconds, and then been disconnected.
	bool HandShaking () {
		return m_connection.HandShaking ();
	}

	/// Attempt to connect to network address.
	/// For client connections only. (Server connections are automatically connected
	/// as soon as they are accepted.)
	/// 
	/// The format of the address string depends on the underlying protocol.
	/// For UDP/IP this is "address:port#", e.g:
	///	\li localhost:8000
	///	\li www.imadethisup.com:1234
	///	\li 127.0.0.1:5555
	///
	/// The request string (optional) is passed to the server as part of the connection
	/// request, and can be read (NetListenLow::RequestString()) before the connection
	/// is accepted or rejected.
	///
	/// If the connection manages to contact the server, it will result in a pending 
	/// connection request (NetListenLow::ConnectionPending() returns true at the server 
	/// end.)
	bool Connect (std::string address, std::string requestString) {
		m_inQueueLock.Lock ();
			m_timingBuffer.Clear ();
		m_inQueueLock.Unlock ();
		bool result = m_connection.Connect (address, requestString);
		CheckError ();
		return result;
	}
	bool Connect (std::string address) {
		return Connect (address, "");
	}

	/// Disconnect the network connection.
	/// \param clean Is true to attempt to send a clean disconnect notification to the remote PC.
	///				 or false to simply drop the connection.
	void Disconnect (bool clean) {
		m_connection.Disconnect (clean);
		CheckError ();
	}

	/// Returns true if the connection is connected.
	/// Also returns true if the connection is in the handshaking stage (see ::HandShaking)
	bool Connected () { 
		return m_connection.Connected (); 
	}
	NetSettingsL1 L1Settings () {
		return m_connection.Settings ();
	}
	void SetL1Settings (NetSettingsL1& settings) {
		m_connection.SetSettings (settings);
	}
	NetSettingsL2 Settings () {
		return m_settings;
	}
	void SetSettings (NetSettingsL2& settings) {
		m_inQueueLock.Lock ();
			m_settings = settings;
		m_inQueueLock.Unlock ();
	}

	// Network operations

	/// Send a message to the remote PC.	
	///	\param data is the message data.
	///	\param size	is the size of the data.
	///	\param channel is used to specify whether message order is important. Valid values are 0-31 inclusive.
	///	Channel 0 is the unordered channel, and NetLib4Games will NOT enforce that messages are received in the same order that they are sent.
	/// Channels 1-31 are ordered channels. NetLib4Games will ensure that messages within the SAME channel are received in the 
	/// same order that they were sent, by either dropping out-of-order non-reliable messages, or by stalling the channel while it waits
	/// for an out-of-order reliable message to be received and slotted into it's correct position.
	/// \param reliable is true if the message MUST get through. NetLib4Games will automatically keep sending reliable messages
	/// until they are confirmed by the remote connection.
	/// Unreliable messages may be dropped.
	/// \smoothed is true to use the latency smoothing algorithm. This attempts to correct fluctuating lag and ensure that 
	/// network packets that are sent out at regular time intervals are received at regular time intervals.
	/// Be careful when using this option, as it effectively works by ADDING EXTRA LAG to messages that arrive quicker than
	/// average.
	///
	/// When the message arrives at the remote connection, it will cause that connection's DataPending() to return true,
	/// and the message can be read with the Receive() method (or similar).
	///
	/// Note:
	///
	/// Carefully choosing which messages are reliable and when ordering is important can
	/// make the difference between a game that is playable over internet conditions and
	/// one that isn't.
	///
	/// The intention is to minimise the chance of the network stalling when a packet is 
	/// dropped or arrives out of order, to maximise the chance that NetLib4Games can continue
	/// processing other packets.
	///
	/// For example, if you separate "chat messages" and "gameplay updates" into different channels,
	/// then the gameplay can keep running if a chat channel stalls (waiting for a dropped chat message
	/// to be resent).
	///
	/// Or, often position updates do not need to be sent reliably because they superceed each other.
	/// So if one update gets dropped, the game can simply continue and pick up the position from the
	/// next update.
	virtual void Send (const char *data, unsigned int size, int channel, bool reliable, bool smoothed);

	/// Returns true if a network message has been received.
	/// The message can then be read with Receive() or ReceivePart().
	bool DataPending ();

	/// Returns the size of the pending network message.
	/// Call this ONLY if DataPending() returns true.
	unsigned int PendingDataSize ();

	/// Receive part of the pending network message.
	/// Call this ONLY if DataPending() returns true.
	///	\param data The destination buffer.
	///	\param offset The offset in the pending network message to copy data from.
	/// \param size The number of bytes to copy.
	///
	/// Calling code MUST ensure that offset and size refer to a valid range of 
	/// data inside the pending message.
	/// Use PendingDataSize() to get the size of the pending message.
	///
	/// After calling ReceivePart, the pending message will still be pending.
	/// If/when you have finished with the pending message, you should call 
	/// DonePendingData() to inform NetLib4Games that you have finished with it.
	void ReceivePart (char *data, unsigned int offset, unsigned int& size);

	///	Receive a network message.
	/// Call this ONLY if DataPending() returns true.
	///	\param data The destination buffer.
	/// \param size The number of bytes to copy from the pending network message.
	///
	/// Calling code MUST ensure that size is not greater than the size of the 
	/// pending network message. Use PendingDataSize() to get the size
	/// of the pending message.
	///
	/// Receive automatically discards the network message after it has copied the
	/// data, so calling code MUST NOT CALL DonePendingData() itself.
	void Receive (char *data, unsigned int &size) {
		m_inQueueLock.Lock ();
			ReceivePart (data, 0, size);
			DonePendingData ();
		m_inQueueLock.Unlock ();
	}

	///	Indicates that we have finished with the pending message.
	/// Call this ONLY if DataPending() returns true.
	/// Use this method if you are reading a message with ReceivePart().
	/// You do not need to call DonePendingData() if you are using Receive() to 
	/// read incoming messages (as Receive() does this automatically).
	void DonePendingData ();	

	// Other pending data functions

	/// Return the channel # of the pending network message.
	/// Call this ONLY if DataPending() returns true.
	/// See Send() for more info.
	int PendingChannel ();

	/// Return the reliable flag of the pending network message.
	/// Call this ONLY if DataPending() returns true.
	/// See Send() for more info.
	bool PendingReliable ();

	/// Return the smoothed flag of the pending network message.
	/// Call this ONLY if DataPending() returns true.
	/// See Send() for more info.
	bool PendingSmoothed ();
//	bool PendingOrdered ();

	// Thread callback
	virtual void ProcessThreadCallback ();

	// NetInChannelL2Callback
	virtual void QueueMessage (NetMessageL2 *msg);
	virtual void RegisterTickCountDifference (int difference);
	virtual void RequestWakeup (unsigned int msec);

    std::string Address() { return m_connection.Address(); }
};

}

#endif