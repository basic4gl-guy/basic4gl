//---------------------------------------------------------------------------
// Created 26-Feb-2005: Thomas Mulgrew (tmulgrew@slingshot.co.nz)
// Copyright (C) Thomas Mulgrew
#pragma hdrstop

#include "TomNetBasicLib.h"
#include "TomFileIOBasicLib.h"

#include <winsock2.h>
#include "UDP/NetLowLevelUDP.h"
#include "NetLayer2.h"

#include <sstream>

using NetLib4Games::NetConL2;
using NetLib4Games::NetListenLow;
using NetLib4Games::NetListenLowUDP;
using NetLib4Games::NetConLow;
using NetLib4Games::NetConLowUDP;

//---------------------------------------------------------------------------

#pragma package(smart_init)

////////////////////////////////////////////////////////////////////////////////
//  Error handling
//
/// We will use the FileError mechanism created in TomFileIOBasicLib.
extern std::string lastError;

////////////////////////////////////////////////////////////////////////////////
//  NetServerWrapper
//
/// Wraps around a NetListenLowUDP.
/// Was originally intended to boost the main thread priority when any servers
/// are running. However this behaviour turned out to be unwanted, and was
/// disabled. The wrapper currently adds NO functionality at all..

static int serverCount = 0;

class NetServerWrapper {
    NetListenLowUDP *m_server;
public:
    NetServerWrapper (unsigned int port) {

        m_server = new NetListenLowUDP (port);
        serverCount++;
    }
    ~NetServerWrapper () {
        serverCount--;
        delete m_server;
    }

    NetListenLowUDP *Server () { return m_server; }
};

////////////////////////////////////////////////////////////////////////////////
//  NetServerStore
//
//  Stores NetListenLow network servers.

typedef vmPointerResourceStore<NetServerWrapper> NetServerStore;

////////////////////////////////////////////////////////////////////////////////
//  NetConnectionStore
//
//  Stores NetConL2 network connections.

typedef vmPointerResourceStore<NetConL2> NetConnectionStore;

////////////////////////////////////////////////////////////////////////////////
//  Globals

NetServerStore      servers;
NetConnectionStore  connections;

////////////////////////////////////////////////////////////////////////////////
//  NetMessageStream
//
//  A network message wrapped up as an input or output stream.
//  By inheriting from FileStream, we can plug into the file I/O mechanism and
//  allow users to use the same code as for accessing files to read/write network
//  messages.

class NetMessageStream : public FileStream {
public:
    int                 connectionHandle;   // Note: We use connection handles rather than
                                            // references, so that we can fail gracefully
                                            // if the connection is deleted before we
                                            // have finalised the message.
    int                 channel;
    bool                reliable, smoothed;

    NetMessageStream (
        int                 _connectionHandle,
        int                 _channel,
        bool                _reliable,
        bool                _smoothed,
        GenericIStream      *_in)
        :
        FileStream          (_in),
        connectionHandle    (_connectionHandle),
        channel             (_channel),
        reliable            (_reliable),
        smoothed            (_smoothed) { ; }

    NetMessageStream (
        int                 _connectionHandle,
        int                 _channel,
        bool                _reliable,
        bool                _smoothed,
        GenericOStream      *_out)
        :
        FileStream          (_out),
        connectionHandle    (_connectionHandle),
        channel             (_channel),
        reliable            (_reliable),
        smoothed            (_smoothed) { ; }

    virtual ~NetMessageStream ();
};

NetMessageStream::~NetMessageStream () {
    if (out != NULL && connections.IndexStored (connectionHandle)) {

        // Send pending packet
        std::stringstream   *stream = (std::stringstream *) out;                // (Net messages are always string streams)
        NetConL2            *connection = connections.Value (connectionHandle);

        if (channel >= 0 && channel < NETL2_MAXCHANNELS && stream->str ().c_str () != NULL)
            connection->Send (
                (char *) stream->str ().c_str (),
                stream->str ().length (),
                channel,
                reliable,
                smoothed);
    }
};

////////////////////////////////////////////////////////////////////////////////
//  Helper functions
bool CheckError (NetLib4Games::HasErrorState *obj) {
    assert (obj != NULL);

    if (obj->Error ()) {
        lastError = obj->GetError ();
        return false;
    }
    else {
        lastError = "";
        return true;
    }
}

////////////////////////////////////////////////////////////////////////////////
//  Function wrappers

void WrapNewServer (TomVM& vm) {

    // Create listener for socket
    NetServerWrapper *server = new NetServerWrapper (vm.GetIntParam (1));

    // Store it
    if (!CheckError (server->Server ())) {
        delete server;
        vm.Reg ().IntVal () = 0;
    }
    else
        vm.Reg ().IntVal () = servers.Alloc (server);
}

void WrapDeleteServer (TomVM& vm) {
    int index = vm.GetIntParam (1);
    if (index > 0 && servers.IndexValid (index))
        servers.Free (index);
    else
        lastError = "Invalid network server handle";
}

void WrapConnectionPending (TomVM& vm) {

    // Find server
    int index = vm.GetIntParam (1);
    if (index > 0 && servers.IndexValid (index)) {
        NetListenLow *server = servers.Value (index)->Server ();
        vm.Reg().IntVal () = server->ConnectionPending() ? -1 : 0;
    }
    else {
        vm.Reg ().IntVal () = 0;
        lastError = "Invalid network server handle";
    }
}

void WrapAcceptConnection (TomVM& vm) {

    // Find server
    int index = vm.GetIntParam (1);
    if (index > 0 && servers.IndexValid (index)) {
        NetListenLow *server = servers.Value (index)->Server ();

        // Accept connection (if pending)
        if (server->ConnectionPending ()) {
            NetConL2 *connection = new NetConL2 (server->AcceptConnection ());
            if (!CheckError (connection)) {
                delete connection;
                vm.Reg ().IntVal () = 0;
            }
            else

                // Store connection
                vm.Reg ().IntVal () = connections.Alloc (connection);
        }
        else
            vm.Reg ().IntVal () = 0;
    }
    else
        lastError = "Invalid network server handle";
}

void WrapRejectConnection (TomVM& vm) {

    // Find server
    int index = vm.GetIntParam (1);
    if (index > 0 && servers.IndexValid (index)) {
        NetListenLow *server = servers.Value (index)->Server ();

        // Reject connection
        if (server->ConnectionPending()) {
            server->RejectConnection();
            CheckError (server);
        }
    }
    else
        lastError = "Invalid network server handle";
}

void WrapNewConnection (TomVM& vm) {

    // Calculate address string
    std::string addressString = vm.GetStringParam (2) + ':' + ::IntToString (vm.GetIntParam (1));

    // Create new connection
    NetConL2 *connection = new NetConL2 (new NetConLowUDP ());
    connection->Connect (addressString);
    if (!CheckError (connection)) {
        delete connection;
        vm.Reg ().IntVal () = 0;
    }
    else

        // Store connection
        vm.Reg ().IntVal () = connections.Alloc (connection);
}

void WrapDeleteConnection (TomVM& vm) {
    int index = vm.GetIntParam (1);
    if (index > 0 && connections.IndexValid (index))
        connections.Free (index);
    else
        lastError = "Invalid network connection handle";
}

void WrapConnectionHandShaking (TomVM& vm) {

    // Find connection
    int index = vm.GetIntParam (1);
    if (index > 0 && connections.IndexValid (index)) {
        NetConL2 *connection = connections.Value (index);
        vm.Reg ().IntVal () = connection->HandShaking();
    }
    else {
        vm.Reg ().IntVal () = 0;
        lastError = "Invalid network connection handle";
    }
}

void WrapConnectionConnected (TomVM& vm) {

    // Find connection
    int index = vm.GetIntParam (1);
    if (index > 0 && connections.IndexValid (index)) {
        NetConL2 *connection = connections.Value (index);
        vm.Reg ().IntVal () = connection->Connected();
    }
    else {
        vm.Reg ().IntVal () = 0;
        lastError = "Invalid network connection handle";
    }
}

void WrapMessagePending (TomVM& vm) {

    // Find connection
    int index = vm.GetIntParam (1);
    if (index > 0 && connections.IndexValid (index)) {
        NetConL2 *connection = connections.Value (index);

        // Check for data
        vm.Reg ().IntVal () = connection->DataPending () ? -1 : 0;
    }
    else {
        vm.Reg ().IntVal () = 0;
        lastError = "Invalid network connection handle";
    }
}

char buffer [65536];

void WrapReceiveMessage (TomVM& vm) {

    // Find connection
    int index = vm.GetIntParam (1);
    if (index > 0 && connections.IndexValid (index)) {
        NetConL2 *connection = connections.Value (index);

        if (connection->DataPending ()) {

            // Get message properties
            int     channel     = connection->PendingChannel ();
            bool    reliable    = connection->PendingReliable (),
                    smoothed    = connection->PendingSmoothed ();

            // Get message data
            unsigned int size = 65536;
            connection->Receive (buffer, size);

            // Copy into string stream
            std::stringstream *stream = new std::stringstream;
            stream->write (buffer, size);

            // Create message
            NetMessageStream *message = new NetMessageStream (
                index,
                channel,
                reliable,
                smoothed,
                (GenericIStream *) stream);

            // Store it
            vm.Reg ().IntVal () = fileStreams.Alloc (message);
        }
        else
            vm.Reg ().IntVal () = 0;
    }
    else {
        vm.Reg ().IntVal () = 0;
        lastError = "Invalid network connection handle";
    }
}

void WrapMessageChannel (TomVM& vm) {

    // Find connection
    int index = vm.GetIntParam (1);
    if (index > 0 && connections.IndexValid (index)) {
        NetConL2 *connection = connections.Value (index);
        if (connection->DataPending ())
            vm.Reg ().IntVal () = connection->PendingChannel();
        else
            vm.Reg ().IntVal () = 0;
    }
    else {
        vm.Reg ().IntVal () = 0;
        lastError = "Invalid network connection handle";
    }
}

void WrapMessageReliable (TomVM& vm) {

    // Find connection
    int index = vm.GetIntParam (1);
    if (index > 0 && connections.IndexValid (index)) {
        NetConL2 *connection = connections.Value (index);
        if (connection->DataPending ())
            vm.Reg ().IntVal () = connection->PendingReliable();
        else
            vm.Reg ().IntVal () = 0;
    }
    else {
        vm.Reg ().IntVal () = 0;
        lastError = "Invalid network connection handle";
    }
}

void WrapMessageSmoothed (TomVM& vm) {

    // Find connection
    int index = vm.GetIntParam (1);
    if (index > 0 && connections.IndexValid (index)) {
        NetConL2 *connection = connections.Value (index);
        if (connection->DataPending ())
            vm.Reg ().IntVal () = connection->PendingSmoothed();
        else
            vm.Reg ().IntVal () = 0;
    }
    else {
        vm.Reg ().IntVal () = 0;
        lastError = "Invalid network connection handle";
    }
}

void WrapSendMessage (TomVM& vm) {

    // Find connection
    int index = vm.GetIntParam (4);
    if (index > 0 && connections.IndexValid (index)) {

        // Verify channel
        int channel = vm.GetIntParam (3);
        if (channel >= 0 && channel < NETL2_MAXCHANNELS) {

            // Create message
            NetMessageStream *message = new NetMessageStream (
                    index,
                    channel,
                    vm.GetIntParam (2),
                    vm.GetIntParam (1),
                    (GenericOStream *) new std::stringstream);

            // Store message
            vm.Reg ().IntVal () = fileStreams.Alloc (message);
        }
        else {
            vm.Reg ().IntVal () = 0;
            lastError = "Invalid channel index. Must be 0 - 31.";
        }
    }
    else {
        vm.Reg ().IntVal () = 0;
        lastError = "Invalid network connection handle";
    }
}

void WrapConnectionAddress(TomVM& vm) {

    // Find connection
    int index = vm.GetIntParam(1);
    if (index > 0 && connections.IndexValid(index)) {
        NetConL2 *connection = connections.Value (index);
        vm.RegString() = connection->Address();
    }
    else
        vm.RegString() = "";
}

void WrapSetConnectionHandshakeTimeout(TomVM& vm) {

    // Find connection
    int index = vm.GetIntParam(2);
    if (index > 0 && connections.IndexValid(index)) {
        NetConL2 *connection = connections.Value(index);

        // Get value
        int value = vm.GetIntParam(1);
        if (value < 1)
            value = 1;

        // Update settings
        NetLib4Games::NetSettingsL1 settings = connection->L1Settings();
        settings.handshakeTimeout = value;
        connection->SetL1Settings(settings);        
    }
}

void WrapSetConnectionTimeout(TomVM& vm) {

    // Find connection
    int index = vm.GetIntParam(2);
    if (index > 0 && connections.IndexValid(index)) {
        NetConL2 *connection = connections.Value(index);

        // Get value
        int value = vm.GetIntParam(1);
        if (value < 1)
            value = 1;

        // Update settings
        NetLib4Games::NetSettingsL1 settings = connection->L1Settings();
        settings.timeout = value;
        connection->SetL1Settings(settings);        
    }
}

void WrapSetConnectionKeepAlive(TomVM& vm) {

    // Find connection
    int index = vm.GetIntParam(2);
    if (index > 0 && connections.IndexValid(index)) {
        NetConL2 *connection = connections.Value(index);

        // Get value
        int value = vm.GetIntParam(1);
        if (value < 1)
            value = 1;

        // Update settings
        NetLib4Games::NetSettingsL1 settings = connection->L1Settings();
        settings.keepAlive = value;
        connection->SetL1Settings(settings);        
    }
}

void WrapSetConnectionReliableResend(TomVM& vm) {

    // Find connection
    int index = vm.GetIntParam(2);
    if (index > 0 && connections.IndexValid(index)) {
        NetConL2 *connection = connections.Value(index);

        // Get value
        int value = vm.GetIntParam(1);
        if (value < 1)
            value = 1;
        if (value > 10000)
            value = 10000;

        // Update settings
        NetLib4Games::NetSettingsL1 settings = connection->L1Settings();
        settings.reliableResend = value;
        connection->SetL1Settings(settings);
    }
}

void WrapSetConnectionDuplicates(TomVM& vm) {

    // Find connection
    int index = vm.GetIntParam(2);
    if (index > 0 && connections.IndexValid(index)) {
        NetConL2 *connection = connections.Value(index);

        // Get value
        int value = vm.GetIntParam(1);
        if (value < 1)
            value = 1;
        if (value > 100)
            value = 100;

        // Update settings
        NetLib4Games::NetSettingsL1 settings = connection->L1Settings();
        settings.dup = value;
        connection->SetL1Settings(settings);
    }
}

void WrapSetConnectionSmoothingPercentage(TomVM& vm) {

    // Find connection
    int index = vm.GetIntParam(2);
    if (index > 0 && connections.IndexValid(index)) {
        NetConL2 *connection = connections.Value(index);

        // Get value
        int value = vm.GetIntParam(1);
        if (value < 0)
            value = 0;
        if (value > 100)
            value = 100;

        // Update settings
        NetLib4Games::NetSettingsL2 settings = connection->Settings();
        settings.smoothingPercentage = value;
        connection->SetSettings(settings);
    }
}

////////////////////////////////////////////////////////////////////////////////
//  Initialisation

void InitTomNetBasicLib (TomBasicCompiler& comp) {

    // Register resources
    comp.VM ().AddResources (servers);
    comp.VM ().AddResources (connections);

    // Constants
    comp.AddConstant("CHANNEL_UNORDERED",   0);
    comp.AddConstant("CHANNEL_ORDERED",     1);
    comp.AddConstant("CHANNEL_MAX",         NETL2_MAXCHANNELS - 1);

    // Functions
    comp.AddFunction ("NewServer",              WrapNewServer,              compParamTypeList () << VTP_INT,                    true, true,  VTP_INT, true);
    comp.AddFunction ("DeleteServer",           WrapDeleteServer,           compParamTypeList () << VTP_INT,                    true, false, VTP_INT, true);
    comp.AddFunction ("ConnectionPending",      WrapConnectionPending,      compParamTypeList () << VTP_INT,                    true, true,  VTP_INT);
    comp.AddFunction ("AcceptConnection",       WrapAcceptConnection,       compParamTypeList () << VTP_INT,                    true, true,  VTP_INT, true);
    comp.AddFunction ("RejectConnection",       WrapRejectConnection,       compParamTypeList () << VTP_INT,                    true, false, VTP_INT, true);
    comp.AddFunction ("NewConnection",          WrapNewConnection,          compParamTypeList () << VTP_STRING << VTP_INT,      true, true,  VTP_INT, true);
    comp.AddFunction ("DeleteConnection",       WrapDeleteConnection,       compParamTypeList () << VTP_INT,                    true, false, VTP_INT, true);
    comp.AddFunction ("ConnectionHandShaking",  WrapConnectionHandShaking,  compParamTypeList () << VTP_INT,                    true, true,  VTP_INT);
    comp.AddFunction ("ConnectionConnected",    WrapConnectionConnected,    compParamTypeList () << VTP_INT,                    true, true,  VTP_INT);
    comp.AddFunction ("MessagePending",         WrapMessagePending,         compParamTypeList () << VTP_INT,                    true, true,  VTP_INT);
    comp.AddFunction ("ReceiveMessage",         WrapReceiveMessage,         compParamTypeList () << VTP_INT,                    true, true,  VTP_INT);
    comp.AddFunction ("MessageChannel",         WrapMessageChannel,         compParamTypeList () << VTP_INT,                    true, true,  VTP_INT);
    comp.AddFunction ("MessageReliable",        WrapMessageReliable,        compParamTypeList () << VTP_INT,                    true, true,  VTP_INT);
    comp.AddFunction ("MessageSmoothed",        WrapMessageSmoothed,        compParamTypeList () << VTP_INT,                    true, true,  VTP_INT);
    comp.AddFunction ("SendMessage",            WrapSendMessage,            compParamTypeList () << VTP_INT << VTP_INT << VTP_INT << VTP_INT, true, true,  VTP_INT);
    comp.AddFunction ("ConnectionAddress",      WrapConnectionAddress,      compParamTypeList () << VTP_INT,                    true, true,  VTP_STRING);

    // L1 settings
    comp.AddFunction ("SetConnectionHandshakeTimeout",      WrapSetConnectionHandshakeTimeout,      compParamTypeList() << VTP_INT << VTP_INT,      true, false, VTP_INT);
    comp.AddFunction ("SetConnectionTimeout",               WrapSetConnectionTimeout,               compParamTypeList() << VTP_INT << VTP_INT,      true, false, VTP_INT);
    comp.AddFunction ("SetConnectionKeepAlive",             WrapSetConnectionKeepAlive,             compParamTypeList() << VTP_INT << VTP_INT,      true, false, VTP_INT);
    comp.AddFunction ("SetConnectionReliableResend",        WrapSetConnectionReliableResend,        compParamTypeList() << VTP_INT << VTP_INT,      true, false, VTP_INT);
    comp.AddFunction ("SetConnectionDuplicates",            WrapSetConnectionDuplicates,            compParamTypeList() << VTP_INT << VTP_INT,      true, false, VTP_INT);

    // L2 settings
    comp.AddFunction ("SetConnectionSmoothingPercentage",   WrapSetConnectionSmoothingPercentage,   compParamTypeList() << VTP_INT << VTP_INT,      true, false, VTP_INT);
}
