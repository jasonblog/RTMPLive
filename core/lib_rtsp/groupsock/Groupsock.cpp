#include "Groupsock.hh"
#include "GroupsockHelper.hh"
#include "TunnelEncaps.hh"
#ifndef NO_SSTREAM
# include <sstream>
#endif
#include <stdio.h>
OutputSocket::OutputSocket(UsageEnvironment& env)
    : Socket(env, 0),
    fSourcePort(0), fLastSentTTL(256)
{}

OutputSocket::OutputSocket(UsageEnvironment& env, Port port)
    : Socket(env, port),
    fSourcePort(0), fLastSentTTL(256)
{}

OutputSocket::~OutputSocket()
{}

Boolean OutputSocket::write(netAddressBits address, Port port, u_int8_t ttl,
                            unsigned char* buffer, unsigned bufferSize)
{
    struct in_addr destAddr;

    destAddr.s_addr = address;
    if ((unsigned) ttl == fLastSentTTL) {
        if (!writeSocket(env(), socketNum(), destAddr, port, buffer, bufferSize)) {
            return False;
        }
    } else {
        if (!writeSocket(env(), socketNum(), destAddr, port, ttl, buffer, bufferSize)) {
            return False;
        }
        fLastSentTTL = (unsigned) ttl;
    }
    if (sourcePortNum() == 0) {
        if (!getSourcePort(env(), socketNum(), fSourcePort)) {
            if (DebugLevel >= 1) {
                env() << *this
                      << ": failed to get source port: "
                      << env().getResultMsg() << "\n";
            }
            return False;
        }
    }
    return True;
}

Boolean OutputSocket
::handleRead(unsigned char *, unsigned,
             unsigned&, struct sockaddr_in&)
{
    return True;
}

destRecord
::destRecord(struct in_addr const& addr, Port const& port, u_int8_t ttl,
             destRecord* next)
    : fNext(next), fGroupEId(addr, port.num(), ttl), fPort(port)
{}

destRecord::~destRecord()
{
    delete fNext;
}

NetInterfaceTrafficStats Groupsock::statsIncoming;
NetInterfaceTrafficStats Groupsock::statsOutgoing;
NetInterfaceTrafficStats Groupsock::statsRelayedIncoming;
NetInterfaceTrafficStats Groupsock::statsRelayedOutgoing;
Groupsock::Groupsock(UsageEnvironment& env, struct in_addr const& groupAddr,
                     Port port, u_int8_t ttl)
    : OutputSocket(env, port),
    deleteIfNoMembers(False), isSlave(False),
    fIncomingGroupEId(groupAddr, port.num(), ttl), fDests(NULL), fTTL(ttl)
{
    addDestination(groupAddr, port);
    if (!socketJoinGroup(env, socketNum(), groupAddr.s_addr)) {
        if (DebugLevel >= 1) {
            env << *this << ": failed to join group: "
                << env.getResultMsg() << "\n";
        }
    }
    if (ourIPAddress(env) == 0) {
        if (DebugLevel >= 0) {
            env << "Unable to determine our source address: "
                << env.getResultMsg() << "\n";
        }
    }
    if (DebugLevel >= 2) {
        env << *this << ": created\n";
    }
}

Groupsock::Groupsock(UsageEnvironment& env, struct in_addr const& groupAddr,
                     struct in_addr const& sourceFilterAddr,
                     Port port)
    : OutputSocket(env, port),
    deleteIfNoMembers(False), isSlave(False),
    fIncomingGroupEId(groupAddr, sourceFilterAddr, port.num()),
    fDests(NULL), fTTL(255)
{
    addDestination(groupAddr, port);
    if (!socketJoinGroupSSM(env, socketNum(), groupAddr.s_addr,
                            sourceFilterAddr.s_addr))
    {
        if (DebugLevel >= 3) {
            env << *this << ": SSM join failed: "
                << env.getResultMsg();
            env << " - trying regular join instead\n";
        }
        if (!socketJoinGroup(env, socketNum(), groupAddr.s_addr)) {
            if (DebugLevel >= 1) {
                env << *this << ": failed to join group: "
                    << env.getResultMsg() << "\n";
            }
        }
    }
    if (DebugLevel >= 2) {
        env << *this << ": created\n";
    }
}

Groupsock::~Groupsock()
{
    if (isSSM()) {
        if (!socketLeaveGroupSSM(env(), socketNum(), groupAddress().s_addr,
                                 sourceFilterAddress().s_addr))
        {
            socketLeaveGroup(env(), socketNum(), groupAddress().s_addr);
        }
    } else {
        socketLeaveGroup(env(), socketNum(), groupAddress().s_addr);
    }
    delete fDests;
    if (DebugLevel >= 2) {
        env() << *this << ": deleting\n";
    }
}

void Groupsock::changeDestinationParameters(struct in_addr const& newDestAddr,
                                            Port newDestPort, int newDestTTL)
{
    if (fDests == NULL) {
        return;
    }
    struct in_addr destAddr = fDests->fGroupEId.groupAddress();
    if (newDestAddr.s_addr != 0) {
        if (newDestAddr.s_addr != destAddr.s_addr &&
            IsMulticastAddress(newDestAddr.s_addr))
        {
            socketLeaveGroup(env(), socketNum(), destAddr.s_addr);
            socketJoinGroup(env(), socketNum(), newDestAddr.s_addr);
        }
        destAddr.s_addr = newDestAddr.s_addr;
    }
    portNumBits destPortNum = fDests->fGroupEId.portNum();
    if (newDestPort.num() != 0) {
        if (newDestPort.num() != destPortNum &&
            IsMulticastAddress(destAddr.s_addr))
        {
            changePort(newDestPort);
            socketJoinGroup(env(), socketNum(), destAddr.s_addr);
        }
        destPortNum   = newDestPort.num();
        fDests->fPort = newDestPort;
    }
    u_int8_t destTTL = ttl();
    if (newDestTTL != ~0) {
        destTTL = (u_int8_t) newDestTTL;
    }
    fDests->fGroupEId = GroupEId(destAddr, destPortNum, destTTL);
}

void Groupsock::addDestination(struct in_addr const& addr, Port const& port)
{
    for (destRecord* dests = fDests; dests != NULL; dests = dests->fNext) {
        if (addr.s_addr == dests->fGroupEId.groupAddress().s_addr &&
            port.num() == dests->fPort.num())
        {
            return;
        }
    }
    fDests = new destRecord(addr, port, ttl(), fDests);
}

void Groupsock::removeDestination(struct in_addr const& addr, Port const& port)
{
    for (destRecord** destsPtr = &fDests; *destsPtr != NULL;
         destsPtr = &((*destsPtr)->fNext))
    {
        if (addr.s_addr == (*destsPtr)->fGroupEId.groupAddress().s_addr &&
            port.num() == (*destsPtr)->fPort.num())
        {
            destRecord* next = (*destsPtr)->fNext;
            (*destsPtr)->fNext = NULL;
            delete (*destsPtr);
            *destsPtr = next;
            return;
        }
    }
}

void Groupsock::removeAllDestinations()
{
    delete fDests;
    fDests = NULL;
}

void Groupsock::multicastSendOnly()
{
    #if 0
    socketLeaveGroup(env(), socketNum(), fIncomingGroupEId.groupAddress().s_addr);
    for (destRecord* dests = fDests; dests != NULL; dests = dests->fNext) {
        socketLeaveGroup(env(), socketNum(), dests->fGroupEId.groupAddress().s_addr);
    }
    #endif
}

Boolean Groupsock::output(UsageEnvironment& env, u_int8_t ttlToSend,
                          unsigned char* buffer, unsigned bufferSize,
                          DirectedNetInterface* interfaceNotToFwdBackTo)
{
    do {
        Boolean writeSuccess = True;
        for (destRecord* dests = fDests; dests != NULL; dests = dests->fNext) {
            if (!write(dests->fGroupEId.groupAddress().s_addr, dests->fPort, ttlToSend,
                       buffer, bufferSize))
            {
                writeSuccess = False;
                break;
            }
        }
        if (!writeSuccess) {
            break;
        }
        statsOutgoing.countPacket(bufferSize);
        statsGroupOutgoing.countPacket(bufferSize);
        int numMembers = 0;
        if (!members().IsEmpty()) {
            numMembers =
                outputToAllMembersExcept(interfaceNotToFwdBackTo,
                                         ttlToSend, buffer, bufferSize,
                                         ourIPAddress(env));
            if (numMembers < 0) {
                break;
            }
        }
        if (DebugLevel >= 3) {
            env << *this << ": wrote " << bufferSize << " bytes, ttl "
                << (unsigned) ttlToSend;
            if (numMembers > 0) {
                env << "; relayed to " << numMembers << " members";
            }
            env << "\n";
        }
        return True;
    } while (0);
    if (DebugLevel >= 0) {
        env.setResultMsg("Groupsock write failed: ", env.getResultMsg());
    }
    return False;
} // Groupsock::output

Boolean Groupsock::handleRead(unsigned char* buffer, unsigned bufferMaxSize,
                              unsigned& bytesRead,
                              struct sockaddr_in& fromAddress)
{
    bytesRead = 0;
    int maxBytesToRead = bufferMaxSize - TunnelEncapsulationTrailerMaxSize;
    int numBytes       = readSocket(env(), socketNum(),
                                    buffer, maxBytesToRead, fromAddress);
    if (numBytes < 0) {
        if (DebugLevel >= 0) {
            env().setResultMsg("Groupsock read failed: ",
                               env().getResultMsg());
        }
        return False;
    }
    if (isSSM() &&
        fromAddress.sin_addr.s_addr != sourceFilterAddress().s_addr)
    {
        return True;
    }
    bytesRead = numBytes;
    int numMembers = 0;
    if (!wasLoopedBackFromUs(env(), fromAddress)) {
        statsIncoming.countPacket(numBytes);
        statsGroupIncoming.countPacket(numBytes);
        numMembers =
            outputToAllMembersExcept(NULL, ttl(),
                                     buffer, bytesRead,
                                     fromAddress.sin_addr.s_addr);
        if (numMembers > 0) {
            statsRelayedIncoming.countPacket(numBytes);
            statsGroupRelayedIncoming.countPacket(numBytes);
        }
    }
    if (DebugLevel >= 3) {
        env() << *this << ": read " << bytesRead << " bytes from " << AddressString(fromAddress).val();
        if (numMembers > 0) {
            env() << "; relayed to " << numMembers << " members";
        }
        env() << "\n";
    }
    return True;
} // Groupsock::handleRead

Boolean Groupsock::wasLoopedBackFromUs(UsageEnvironment  & env,
                                       struct sockaddr_in& fromAddress)
{
    if (fromAddress.sin_addr.s_addr
        == ourIPAddress(env))
    {
        if (fromAddress.sin_port == sourcePortNum()) {
            #ifdef DEBUG_LOOPBACK_CHECKING
            if (DebugLevel >= 3) {
                env() << *this << ": got looped-back packet\n";
            }
            #endif
            return True;
        }
    }
    return False;
}

int Groupsock::outputToAllMembersExcept(DirectedNetInterface* exceptInterface,
                                        u_int8_t ttlToFwd,
                                        unsigned char* data, unsigned size,
                                        netAddressBits sourceAddr)
{
    if (ttlToFwd == 0) {
        return 0;
    }
    DirectedNetInterfaceSet::Iterator iter(members());
    unsigned numMembers = 0;
    DirectedNetInterface* interf;
    while ((interf = iter.next()) != NULL) {
        if (interf == exceptInterface) {
            continue;
        }
        UsageEnvironment& saveEnv = env();
        if (!interf->SourceAddrOKForRelaying(saveEnv, sourceAddr)) {
            if (strcmp(saveEnv.getResultMsg(), "") != 0) {
                return -1;
            } else {
                continue;
            }
        }
        if (numMembers == 0) {
            TunnelEncapsulationTrailer* trailerInPacket =
                (TunnelEncapsulationTrailer *) &data[size];
            TunnelEncapsulationTrailer* trailer;
            Boolean misaligned = ((uintptr_t) trailerInPacket & 3) != 0;
            unsigned trailerOffset;
            u_int8_t tunnelCmd;
            if (isSSM()) {
                trailerOffset = TunnelEncapsulationTrailerAuxSize;
                tunnelCmd     = TunnelDataAuxCmd;
            } else {
                trailerOffset = 0;
                tunnelCmd     = TunnelDataCmd;
            }
            unsigned trailerSize = TunnelEncapsulationTrailerSize + trailerOffset;
            unsigned tmpTr[TunnelEncapsulationTrailerMaxSize];
            if (misaligned) {
                trailer = (TunnelEncapsulationTrailer *) &tmpTr;
            } else {
                trailer = trailerInPacket;
            }
            trailer += trailerOffset;
            if (fDests != NULL) {
                trailer->address() = fDests->fGroupEId.groupAddress().s_addr;
                trailer->port()    = fDests->fPort;
            }
            trailer->ttl()     = ttlToFwd;
            trailer->command() = tunnelCmd;
            if (isSSM()) {
                trailer->auxAddress() = sourceFilterAddress().s_addr;
            }
            if (misaligned) {
                memmove(trailerInPacket, trailer - trailerOffset, trailerSize);
            }
            size += trailerSize;
        }
        interf->write(data, size);
        ++numMembers;
    }
    return numMembers;
} // Groupsock::outputToAllMembersExcept

UsageEnvironment& operator << (UsageEnvironment& s, const Groupsock& g)
{
    UsageEnvironment& s1 = s << timestampString() << " Groupsock("
                             << g.socketNum() << ": "
                             << AddressString(g.groupAddress()).val()
                             << ", " << g.port() << ", ";

    if (g.isSSM()) {
        return s1 << "SSM source: "
                  <<  AddressString(g.sourceFilterAddress()).val() << ")";
    } else {
        return s1 << (unsigned) (g.ttl()) << ")";
    }
}

static HashTable *& getSocketTable(UsageEnvironment& env)
{
    _groupsockPriv* priv = groupsockPriv(env);

    if (priv->socketTable == NULL) {
        priv->socketTable = HashTable::create(ONE_WORD_HASH_KEYS);
    }
    return priv->socketTable;
}

static Boolean unsetGroupsockBySocket(Groupsock const* groupsock)
{
    do {
        if (groupsock == NULL) {
            break;
        }
        int sock = groupsock->socketNum();
        if (sock < 0) {
            break;
        }
        HashTable *& sockets = getSocketTable(groupsock->env());
        Groupsock* gs        = (Groupsock *) sockets->Lookup((char *) (long) sock);
        if (gs == NULL || gs != groupsock) {
            break;
        }
        sockets->Remove((char *) (long) sock);
        if (sockets->IsEmpty()) {
            delete sockets;
            sockets = NULL;
            reclaimGroupsockPriv(gs->env());
        }
        return True;
    } while (0);
    return False;
}

static Boolean setGroupsockBySocket(UsageEnvironment& env, int sock,
                                    Groupsock* groupsock)
{
    do {
        if (sock < 0) {
            char buf[100];
            sprintf(buf, "trying to use bad socket (%d)", sock);
            env.setResultMsg(buf);
            break;
        }
        HashTable* sockets    = getSocketTable(env);
        Boolean alreadyExists =
            (sockets->Lookup((char *) (long) sock) != 0);
        if (alreadyExists) {
            char buf[100];
            sprintf(buf,
                    "Attempting to replace an existing socket (%d",
                    sock);
            env.setResultMsg(buf);
            break;
        }
        sockets->Add((char *) (long) sock, groupsock);
        return True;
    } while (0);
    return False;
}

static Groupsock * getGroupsockBySocket(UsageEnvironment& env, int sock)
{
    do {
        if (sock < 0) {
            break;
        }
        HashTable* sockets = getSocketTable(env);
        return (Groupsock *) sockets->Lookup((char *) (long) sock);
    } while (0);
    return NULL;
}

Groupsock * GroupsockLookupTable::Fetch(UsageEnvironment& env,
                                        netAddressBits groupAddress,
                                        Port port, u_int8_t ttl,
                                        Boolean& isNew)
{
    isNew = False;
    Groupsock* groupsock;
    do {
        groupsock = (Groupsock *) fTable.Lookup(groupAddress, (~0), port);
        if (groupsock == NULL) {
            groupsock = AddNew(env, groupAddress, (~0), port, ttl);
            if (groupsock == NULL) {
                break;
            }
            isNew = True;
        }
    } while (0);
    return groupsock;
}

Groupsock * GroupsockLookupTable::Fetch(UsageEnvironment& env,
                                        netAddressBits groupAddress,
                                        netAddressBits sourceFilterAddr, Port port,
                                        Boolean& isNew)
{
    isNew = False;
    Groupsock* groupsock;
    do {
        groupsock =
            (Groupsock *) fTable.Lookup(groupAddress, sourceFilterAddr, port);
        if (groupsock == NULL) {
            groupsock = AddNew(env, groupAddress, sourceFilterAddr, port, 0);
            if (groupsock == NULL) {
                break;
            }
            isNew = True;
        }
    } while (0);
    return groupsock;
}

Groupsock * GroupsockLookupTable::Lookup(netAddressBits groupAddress, Port port)
{
    return (Groupsock *) fTable.Lookup(groupAddress, (~0), port);
}

Groupsock * GroupsockLookupTable::Lookup(netAddressBits groupAddress,
                                         netAddressBits sourceFilterAddr, Port port)
{
    return (Groupsock *) fTable.Lookup(groupAddress, sourceFilterAddr, port);
}

Groupsock * GroupsockLookupTable::Lookup(UsageEnvironment& env, int sock)
{
    return getGroupsockBySocket(env, sock);
}

Boolean GroupsockLookupTable::Remove(Groupsock const* groupsock)
{
    unsetGroupsockBySocket(groupsock);
    return fTable.Remove(groupsock->groupAddress().s_addr,
                         groupsock->sourceFilterAddress().s_addr,
                         groupsock->port());
}

Groupsock * GroupsockLookupTable::AddNew(UsageEnvironment& env,
                                         netAddressBits groupAddress,
                                         netAddressBits sourceFilterAddress,
                                         Port port, u_int8_t ttl)
{
    Groupsock* groupsock;

    do {
        struct in_addr groupAddr;
        groupAddr.s_addr = groupAddress;
        if (sourceFilterAddress == netAddressBits(~0)) {
            groupsock = new Groupsock(env, groupAddr, port, ttl);
        } else {
            struct in_addr sourceFilterAddr;
            sourceFilterAddr.s_addr = sourceFilterAddress;
            groupsock = new Groupsock(env, groupAddr, sourceFilterAddr, port);
        }
        if (groupsock == NULL || groupsock->socketNum() < 0) {
            break;
        }
        if (!setGroupsockBySocket(env, groupsock->socketNum(), groupsock)) {
            break;
        }
        fTable.Add(groupAddress, sourceFilterAddress, port, (void *) groupsock);
    } while (0);
    return groupsock;
}

GroupsockLookupTable::Iterator::Iterator(GroupsockLookupTable& groupsocks)
    : fIter(AddressPortLookupTable::Iterator(groupsocks.fTable))
{}

Groupsock * GroupsockLookupTable::Iterator::next()
{
    return (Groupsock *) fIter.next();
};
