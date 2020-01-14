/*	NetLayer1.cpp

	Created 7-Jan-2004: Thomas Mulgrew (tmulgrew@slingshot.co.nz)

	NetLib4Games layer 1.
*/

#include "NetLayer1.h"
#include "NetLog.h"

namespace NetLib4Games {

#define WAITEXTENSION 2		
#define MAX_CON_REQ_SIZE 4096

#include <windows.h>
// Defines "GetTickCount"
// If porting to another platform, just need to replace GetTickCount() with another function.

#ifdef NETLOG
inline static std::string Desc (NetPacketHeaderL1 *header) {
	bool			reliable	= (header->flags & NETL1_RELIABLE) != 0;
    bool            resent      = (header->flags & NETL1_RESENT) != 0;
	NetL1Type		type		= (NetL1Type) NETL1_GETTYPE (header->flags);
	unsigned long id			= header->id;
	std::string		typeStr;
	switch (type) {
	case l1User:		typeStr = "User"; break;
	case l1KeepAlive:	typeStr = "KeepAlive"; break;
	case l1Confirm:		typeStr = "Confirm"; break;
	case l1Connect:		typeStr = "Connect"; break;
	case l1Accept:		typeStr = "Accept"; break;
	case l1Disconnect:	typeStr = "Disconnect"; break;
	default:			typeStr = "UNKNOWN!?!";
	};
	return (reliable ? (std::string)"Reliable, " : (std::string)"Unreliable, ") +
        (resent ? (std::string)"Resent, " : (std::string)"") +
        typeStr + (std::string)", id: " + IntToString(id);
}
inline static std::string Desc (NetSimplePacket *packet) {
	return Desc ((NetPacketHeaderL1 *) packet->data);
}
#endif

///////////////////////////////////////////////////////////////////////////////
//	Globals
NetConReqValidatorL1 validatorL1;

///////////////////////////////////////////////////////////////////////////////
//	NetConL1
NetConL1::NetConL1 (NetConLow *connection, NetSettingsStaticL1& settings) : 
	m_connection			(connection),
	m_settingsStatic		(settings),
	m_reliableReceived		(m_settingsStatic.reliableBitBufSize,	0),
	m_unreliableReceived	(m_settingsStatic.unreliableBitBufSize, 0),
	m_sendIDReliable		(0),
	m_sendIDUnreliable		(0),
	m_wakeupTime			(INFINITE) {

	assert (m_connection != NULL); 

	NetLog ("Create L1 connection");

	// Init state
	m_handShaking	= m_connection->Connected ();

	// Init timing
	m_lastSent		= 
	m_lastReceived	= GetTickCount ();

	// Validate 
	Validate ();
}

NetConL1::NetConL1 (NetConLow *connection) :
	m_connection			(connection),
	m_settingsStatic		(),
	m_reliableReceived		(m_settingsStatic.reliableBitBufSize,	0),
	m_unreliableReceived	(m_settingsStatic.unreliableBitBufSize, 0),
	m_sendIDReliable		(0),
	m_sendIDUnreliable		(0),
	m_wakeupTime			(INFINITE) {

	assert (m_connection != NULL); 

	NetLog ("Create L1 connection");

	// Init state
	m_handShaking	= m_connection->Connected ();

	// Init timing
	m_lastSent		= 
	m_lastReceived	= GetTickCount ();

	// Validate 
	Validate ();
}

NetConL1::~NetConL1 () {
	Disconnect (!Error ());

    // Close the processing thread
    m_processThread.Terminate ();

	// Clear send/receive buffers
//	for (	std::list<NetSimplePacket *>::iterator i = m_recvBuffer.begin ();
	for (	std::list<NetInPacketL1 *>::iterator i = m_recvBuffer.begin ();
			i != m_recvBuffer.end ();
			i++) 
		delete *i;
	m_recvBuffer.clear ();
	for (	std::list<NetOutPacketL1 *>::iterator i2 = m_sendBuffer.begin ();
			i2 != m_sendBuffer.end ();
			i2++) 
		delete *i2;
	m_sendBuffer.clear ();

	if (m_connection != NULL)
		delete m_connection;

	NetLog ("Delete L1 connection");
}

void NetConL1::Validate () {

	// Validate the connection and ensure that it is useable.
	// Called by constructors.
	// Calling code should check error state to see whether connection is useable
	if (m_connection->Error ())
		SetError (m_connection->GetError ());

    // Check max packet size, but only if connected. The UDP protocol at least
    // must be connected before it can correctly return the maximum packet size
	else if (Connected () && m_connection->MaxPacketSize () <= sizeof (NetPacketHeaderL1))
		SetError ("Underlying network protocol packet size is too small!");

	else
		ClearError ();

	// TODO!: Validate m_settingsStatic

	// Disconnect if error
	if (Error () && Connected ())
		Disconnect (false);
}

bool NetConL1::Connect (std::string address, std::string connectionRequest) {
	
	m_processLock.Lock ();
		if (m_connection->Client () && !Connected () && m_connection->Connect (address)) {

			NetLog ("Send L1 connection request");

			// Switch to handshaking mode
			m_handShaking = true;

			// Client connections must start the handshaking sequence
			if (m_connection->Client ()) {

				// Build connection packet
				char buf [sizeof (NetPacketHeaderL1) + MAX_CON_REQ_SIZE];

				// Setup header
				BuildHeader ((NetPacketHeaderL1 *) buf, false, l1Connect, 0);

				// Add connection string
				// Calculate length
				int size = connectionRequest.length ();

				// May need to trim to fit buffer
				if (size >= MAX_CON_REQ_SIZE)
					size = MAX_CON_REQ_SIZE - 1;

				// Or packet
				if (size >= m_connection->MaxPacketSize () - sizeof (NetPacketHeaderL1))
					size = m_connection->MaxPacketSize () - sizeof (NetPacketHeaderL1) - 1;

				// Insert string
				if (size > 0)
					memcpy (buf + sizeof (NetPacketHeaderL1), connectionRequest.c_str (), size);

				// Send packet
				for (int i = 0; i < m_settings.dup; i++) 
					m_connection->Send (buf, sizeof (NetPacketHeaderL1) + size);
				m_stateLock.Lock ();
					m_lastSent = GetTickCount ();
				m_stateLock.Unlock ();
			}

		}

		bool result = Connected ();
	m_processLock.Unlock ();
	return result;
}

void NetConL1::Disconnect (bool clean) {
	m_processLock.Lock ();
		if (Connected ()) {
			if (clean) {

				NetLog ("Send clean disconnect");

				// Send clean disconnect packet(s)
				NetPacketHeaderL1 header;
				BuildHeader (	
					&header,
					false, 
					l1Disconnect,
					0);

				for (int i = 0; i < m_settings.dup; i++)
					m_connection->Send ((char *) &header, sizeof (header));
			}

			// Low level disconnect
			m_connection->Disconnect ();
		}
	m_processLock.Unlock ();
}

bool NetConL1::Connected () {
	return m_connection->Connected ();
}

void NetConL1::Send (char *data, unsigned int size, bool reliable) {	
	if (!Connected ())
		return;

	// Create new packet. Allow room for our header
	NetSimplePacket		*packet = new NetSimplePacket (size + sizeof (NetPacketHeaderL1));
	NetPacketHeaderL1	*packetHeader = (NetPacketHeaderL1 *) packet->data;
	char				*packetData	= packet->data + sizeof (NetPacketHeaderL1);

	// Setup header
	if (reliable)
		BuildHeader (packetHeader, true,  l1User, m_sendIDReliable++);
	else
		BuildHeader (packetHeader, false, l1User, m_sendIDUnreliable++);

	NetLog ((std::string) "Send L1 packet, " + Desc (packetHeader));

	// Copy in user data
	if (size > 0)
		memcpy (packetData, data, size);

	// Send packet
	// Note: Packets are not sent during the handshaking stage, 
	// Instead they are queued, and flushed once the actual handshake succeeds.
	m_stateLock.Lock ();
		if (!HandShaking ())
			for (int i = 0; i < m_settings.dup; i++)
				m_connection->Send (packet->data, packet->size);

		// Queue packet if necessary
		if (HandShaking () || reliable) {

			NetLog ("Queue outgoing L1 packet");

			// Calculate due time
			// Packets queued while handshaking are sent as soon as possible
			// Reliable packets are sent after a resend delay
			unsigned int due;
			if (HandShaking ())	due = GetTickCount ();
			else				due = GetTickCount () + m_settings.reliableResend;
			
			// Create queued outgoing packet
			m_outQueueLock.Lock ();
				m_sendBuffer.push_back (new NetOutPacketL1 (due, packet));
			m_outQueueLock.Unlock ();
		}
		else

			// Packet not queued, delete it
			delete packet;
	m_stateLock.Unlock ();
}

bool NetConL1::DataPending () {
	m_inQueueLock.Lock ();
		bool result = !m_recvBuffer.empty ();
	m_inQueueLock.Unlock ();
	return result;
}

unsigned int NetConL1::PendingDataSize () {
	m_inQueueLock.Lock ();
//		int result = DataPending () ? m_recvBuffer.front ()->size : 0;
		int result = DataPending () ? m_recvBuffer.front ()->packet.size : 0;
	m_inQueueLock.Unlock ();
	return result;
}

bool NetConL1::PendingIsResent() {
    m_inQueueLock.Lock();
        bool result = DataPending() ? m_recvBuffer.front()->resent : false;
    m_inQueueLock.Unlock();
    return result;
}

void NetConL1::ReceivePart (char *data, unsigned int offset, unsigned int& size) {
	assert (data != NULL);
	m_inQueueLock.Lock ();
		if (DataPending ()) {
		
			// Find topmost queued packet
//			NetSimplePacket *packet = m_recvBuffer.front ();
			NetSimplePacket *packet = &m_recvBuffer.front()->packet;

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
	
void NetConL1::DonePendingData () {
	m_inQueueLock.Lock ();
		if (DataPending ()) {
//			NetSimplePacket *packet = m_recvBuffer.front ();
			NetInPacketL1 *packet = m_recvBuffer.front ();
			m_recvBuffer.pop_front ();
			delete packet;
		}
	m_inQueueLock.Unlock ();
};

void NetConL1::ThreadExecute () {

	while (!m_processThread.Terminating ()) {

		// Wait for data event on connection, or until it is time to 
		// perform a maintenance action such as sending a keepalive,
		// resending a reliable packet etc.		

		// Calculate when the next maintenance action is due
		m_processLock.Lock ();

			DWORD wait;
			if (m_connection->Connected ()) {

				DWORD nextEvent = m_wakeupTime;

				// Timeout?
				DWORD event = m_lastReceived + (m_handShaking ? m_settings.handshakeTimeout : m_settings.timeout);
				if (event < nextEvent)
					nextEvent = event;

				// Keepalive?
				event = m_lastSent + (m_handShaking ? m_settings.reliableResend : m_settings.keepAlive);
				if (event < nextEvent) 
					nextEvent = event;

				// Reliable resend?
				if (!m_handShaking) {
					m_outQueueLock.Lock ();

						for (	std::list<NetOutPacketL1 *>::iterator i = m_sendBuffer.begin ();
								i != m_sendBuffer.end ();
								i++) {
							event = (*i)->due;
							if (event < nextEvent)
								nextEvent = event;
						}

					m_outQueueLock.Unlock ();
				}

				// Calculate duration to wait
				nextEvent += WAITEXTENSION;
				DWORD tickCount = GetTickCount ();
				wait = nextEvent;
				if (wait > tickCount) 
					wait -= tickCount;
				else
					wait = 0;
			}
			else
				wait = INFINITE;

		m_processLock.Unlock ();

		// Wait for data or next due event
		ThreadEvent *events [2] = { &m_connection->Event (), &m_processThread.TerminateEvent () };
        if (wait > 0)
    		WaitForEvents (events, 2, false, wait);
		m_wakeupTime = INFINITE;

		m_processLock.Lock ();
			if (!m_processThread.Terminating () && m_connection->Connected ()) {
					
				DWORD tickCount = GetTickCount ();

				// Process incoming packets
				m_inQueueLock.Lock ();
					while (ProcessRecvPacket (tickCount))							
						;
				m_inQueueLock.Unlock ();

				// Process outgoing packets
				m_outQueueLock.Lock ();

					std::list<NetOutPacketL1 *>::iterator i = m_sendBuffer.begin ();
					while (i != m_sendBuffer.end ()) {			
						if (tickCount > (*i)->due) {

							// Send the packet
							NetLog ("Send buffered outgoing L1 packet. " + Desc ((*i)->packet));

                            // Mark the packet as resent
                            NetPacketHeaderL1* header = (NetPacketHeaderL1*) (*i)->packet->data;
                            header->flags |= NETL1_RESENT;

							m_connection->Send ((*i)->packet->data, (*i)->packet->size);
							m_lastSent = tickCount;

							// If packet is reliable, leave it in the buffer
							if ((*i)->reliable) {

								// Update due date of next resend
								(*i)->due = tickCount + m_settings.reliableResend;

								// Move onto next
								i++;
							}
							// Otherwise delete it, and remove from buffer.
							else {
 
								delete *i;
								i = m_sendBuffer.erase (i);
							}
						}
						else
							i++;
					}

				m_outQueueLock.Unlock ();

				////////////////////////////
				// Keepalives and timeouts

				// Timeouts
				unsigned int timeout = m_handShaking ? m_settings.handshakeTimeout : m_settings.timeout;
				if (tickCount >= m_lastReceived + timeout) {

					NetLog ("L1 connection timed out");
					Disconnect (true);
				}

				// Keep alives
				else if (m_connection->Connected () && m_handShaking) {
				
					// Keepalives are not sent during handshaking, but the client does keep
					// sending requests until accepted, disconnected or timed out
					if (m_connection->Client () && tickCount >= m_lastSent + m_settings.reliableResend) {

						NetLog ("Resend L1 connect request");
						BuildAndSend (false, l1Connect, 0, tickCount, false);
					}
				}
				else {
				
					// Send keepalive
					if (tickCount > m_lastSent + m_settings.keepAlive) {

						NetLog ("Send L1 keepalive");
						BuildAndSend (false, l1KeepAlive, 0, tickCount, false);
					}
				}
			}

			// Process thread callback
			if (m_callback != NULL)
				m_callback->ProcessThreadCallback ();

		m_processLock.Unlock ();		
	}
}

bool NetConL1::ProcessRecvPacket (unsigned int tickCount) {

	// Process the next inbound packet.
	if (!m_connection->DataPending ())		
		return false;

	// Record that data was received.
	m_lastReceived = tickCount;

	// Get packet size
	unsigned int size = m_connection->PendingDataSize ();
		// Note: If a packet is received that is smaller than our layer 1 header,
		// then it is erroneous. All we can do is ignore it and continue.

	NetLog ("Receive L1 packet, " + IntToString (size) + " bytes");

	if (size >= sizeof (NetPacketHeaderL1)) {

		// Read packet header
		NetPacketHeaderL1 header;
		unsigned int recvSize = sizeof (header);
		m_connection->ReceivePart ((char *) &header, 0, recvSize);

		NetLog ("Incoming L1 packet. " + Desc (&header));

		// Decode header
		bool			reliable	= (header.flags & NETL1_RELIABLE) != 0;
        bool            resent      = (header.flags & NETL1_RESENT) != 0;
		NetL1Type		type		= (NetL1Type) NETL1_GETTYPE (header.flags);
		unsigned long	id			= header.id;

		// Process packet
		switch (type) {
			case l1User: {

					NetRevolvingBitBuffer& buffer = reliable ? m_reliableReceived : m_unreliableReceived;
					bool truesRemoved, falsesRemoved;

					// Reserve room in reliables-received array
					buffer.SetTop (id + 1, false, truesRemoved, falsesRemoved);

					// If we had to remove some "false" values from the bottom, this means
					// that there are packets that were never received!
					// If this is the reliable buffer, then we have an error as we are
					// unable to guarantee that reliable packets were delivered.
					if (falsesRemoved && reliable) {
						Disconnect (true);
						SetError ("Reliable packet never arrived");
						break;									
					}

					// If the packet is in range, and we haven't already received
					// one for this id, then process it. Otherwise discard it as 
					// a duplicate.
					if (buffer.InRange (id) && !buffer.Value (id)) {								

						NetLog ("Queue incoming L1 packet");

                        unsigned int dataSize = size - sizeof(header);
                        NetInPacketL1 *packet = new NetInPacketL1(dataSize, resent);
                        if (dataSize > 0)
                            m_connection->ReceivePart(packet->packet.data, sizeof(header), dataSize);

						// Create and queue received packet
//						NetSimplePacket *packet = new NetSimplePacket (size - sizeof (header));
//						if (size > sizeof (header)) {
//							unsigned int recvSize = size - sizeof (header);
//							m_connection->ReceivePart (packet->data, sizeof (header), recvSize);
//						}
						m_inQueueLock.Lock ();
							m_recvBuffer.push_back (packet);
						m_inQueueLock.Unlock ();

						// Mark ID as received
						buffer.Value (id) = true;
					}
					else
						NetLog ("Already received this L1 packet, ignore duplicate");

					// Always confirm reliable packets.
					// Even duplicates, as the original confirmation message may
					// have been lost.
					if (reliable) {

						NetLog ("Confirm L1 packet");
						BuildAndSend (false, l1Confirm, id, tickCount, false);
					}
				}
				break;

			case l1KeepAlive: 
				break;

			case l1Confirm: {
				
					// Remove confirmed packet from send buffer
					m_outQueueLock.Lock ();
						std::list<NetOutPacketL1 *>::iterator i = m_sendBuffer.begin ();
						while (i != m_sendBuffer.end ()) {
							if ((*i)->id == id) {

								// Delete packet
								delete *i;

								// Remove entry from buffer
								i = m_sendBuffer.erase (i);
							}
							else
								i++;
						}
					m_outQueueLock.Unlock ();
				}
				break;

			case l1Connect: {

					if (!m_connection->Client ()) {
						
						// Connection request
						// Send accepted packet.
						// Note, we do this regardless of whether we are handshaking or not.
						// This is because the other end may not know that we have progressed
						// past the handshaking stage (eg a previous "accept" may not have
						// gotten through.)

						NetLog ("Send L1 accept");

						// Send connection accepted packet
						BuildAndSend (false, l1Accept, 0, tickCount, false);

						// Handshaking is now complete
						m_stateLock.Lock ();
							m_handShaking = false;
						m_stateLock.Unlock ();
					}
				}
				break;

			case l1Accept: {

					// Connection accepted
					if (m_connection->Client ()) {
						m_stateLock.Lock ();
							m_handShaking = false;
						m_stateLock.Unlock ();
					}
				}
				break;

			case l1Disconnect: {

					// Clean disconnect received
					Disconnect (false);
				}
				break;
		}
	}

	// Finished with this packet
	m_connection->DonePendingData ();

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//	NetConReqValidatorL1

bool NetConReqValidatorL1::IsConnectionRequest (NetSimplePacket *packet, std::string& requestString) {
	assert (packet != NULL);

	// Is packet large enough?
	if (packet->size >= sizeof (NetPacketHeaderL1)) {

		// Map to header
		NetPacketHeaderL1 *header = (NetPacketHeaderL1 *)packet->data;

		// Check type
		if ((NetL1Type) NETL1_GETTYPE (header->flags) == l1Connect) {
			
			// Extract request string
			char buf [MAX_CON_REQ_SIZE];
			unsigned int size = packet->size - sizeof (NetPacketHeaderL1);
			if (size >= MAX_CON_REQ_SIZE)
				size = MAX_CON_REQ_SIZE - 1;
			if (size > 0)
				memcpy (buf, packet->data + sizeof (NetPacketHeaderL1), size);
			buf [size] = 0;
			requestString = buf;

			return true;
		}
	}
	
	return false;
}

}