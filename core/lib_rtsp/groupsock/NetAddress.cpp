#include "NetAddress.hh"
#include "GroupsockHelper.hh"
#include <stddef.h>
#include <stdio.h>
#if defined(__WIN32__) || defined(_WIN32)
# define USE_GETHOSTBYNAME 1
#else
# ifndef INADDR_NONE
#  define INADDR_NONE 0xFFFFFFFF
# endif
#endif
NetAddress::NetAddress(u_int8_t const* data, unsigned length)
{
    assign(data, length);
}

NetAddress::NetAddress(unsigned length)
{
    fData = new u_int8_t[length];
    if (fData == NULL) {
        fLength = 0;
        return;
    }
    for (unsigned i = 0; i < length; ++i) {
        fData[i] = 0;
    }
    fLength = length;
}

NetAddress::NetAddress(NetAddress const& orig)
{
    assign(orig.data(), orig.length());
}

NetAddress& NetAddress::operator = (NetAddress const& rightSide)
{
    if (&rightSide != this) {
        clean();
        assign(rightSide.data(), rightSide.length());
    }
    return *this;
}

NetAddress::~NetAddress()
{
    clean();
}

void NetAddress::assign(u_int8_t const* data, unsigned length)
{
    fData = new u_int8_t[length];
    if (fData == NULL) {
        fLength = 0;
        return;
    }
    for (unsigned i = 0; i < length; ++i) {
        fData[i] = data[i];
    }
    fLength = length;
}

void NetAddress::clean()
{
    delete[] fData;
    fData   = NULL;
    fLength = 0;
}

NetAddressList::NetAddressList(char const* hostname)
    : fNumAddresses(0), fAddressArray(NULL)
{
    netAddressBits addr = our_inet_addr((char *) hostname);

    if (addr != INADDR_NONE) {
        fNumAddresses = 1;
        fAddressArray = new NetAddress *[fNumAddresses];
        if (fAddressArray == NULL) {
            return;
        }
        fAddressArray[0] = new NetAddress((u_int8_t *) &addr, sizeof(netAddressBits));
        return;
    }
    #if defined(USE_GETHOSTBYNAME) || defined(VXWORKS)
    struct hostent* host;
    # if defined(VXWORKS)
    char hostentBuf[512];
    host = (struct hostent *) resolvGetHostByName((char *) hostname, (char *) &hostentBuf, sizeof hostentBuf);
    # else
    host = gethostbyname((char *) hostname);
    # endif
    if (host == NULL || host->h_length != 4 || host->h_addr_list == NULL) {
        return;
    }
    u_int8_t const ** const hAddrPtr = (u_int8_t const **) host->h_addr_list;
    u_int8_t const** hAddrPtr1       = hAddrPtr;
    while (*hAddrPtr1 != NULL) {
        ++fNumAddresses;
        ++hAddrPtr1;
    }
    fAddressArray = new NetAddress *[fNumAddresses];
    if (fAddressArray == NULL) {
        return;
    }
    for (unsigned i = 0; i < fNumAddresses; ++i) {
        fAddressArray[i] = new NetAddress(hAddrPtr[i], host->h_length);
    }
    #else // if defined(USE_GETHOSTBYNAME) || defined(VXWORKS)
    struct addrinfo addrinfoHints;
    memset(&addrinfoHints, 0, sizeof addrinfoHints);
    addrinfoHints.ai_family = AF_INET;
    struct addrinfo* addrinfoResultPtr = NULL;
    int result = getaddrinfo(hostname, NULL, &addrinfoHints, &addrinfoResultPtr);
    if (result != 0 || addrinfoResultPtr == NULL) {
        return;
    }
    const struct addrinfo* p = addrinfoResultPtr;
    while (p != NULL) {
        if (p->ai_addrlen < 4) {
            continue;
        }
        ++fNumAddresses;
        p = p->ai_next;
    }
    fAddressArray = new NetAddress *[fNumAddresses];
    if (fAddressArray == NULL) {
        return;
    }
    unsigned i = 0;
    p = addrinfoResultPtr;
    while (p != NULL) {
        if (p->ai_addrlen < 4) {
            continue;
        }
        fAddressArray[i++] = new NetAddress((u_int8_t const *) &(((struct sockaddr_in *) p->ai_addr)->sin_addr.s_addr),
                                            4);
        p = p->ai_next;
    }
    freeaddrinfo(addrinfoResultPtr);
    #endif // if defined(USE_GETHOSTBYNAME) || defined(VXWORKS)
}

NetAddressList::NetAddressList(NetAddressList const& orig)
{
    assign(orig.numAddresses(), orig.fAddressArray);
}

NetAddressList& NetAddressList::operator = (NetAddressList const& rightSide)
{
    if (&rightSide != this) {
        clean();
        assign(rightSide.numAddresses(), rightSide.fAddressArray);
    }
    return *this;
}

NetAddressList::~NetAddressList()
{
    clean();
}

void NetAddressList::assign(unsigned numAddresses, NetAddress** addressArray)
{
    fAddressArray = new NetAddress *[numAddresses];
    if (fAddressArray == NULL) {
        fNumAddresses = 0;
        return;
    }
    for (unsigned i = 0; i < numAddresses; ++i) {
        fAddressArray[i] = new NetAddress(*addressArray[i]);
    }
    fNumAddresses = numAddresses;
}

void NetAddressList::clean()
{
    while (fNumAddresses-- > 0) {
        delete fAddressArray[fNumAddresses];
    }
    delete[] fAddressArray;
    fAddressArray = NULL;
}

NetAddress const * NetAddressList::firstAddress() const
{
    if (fNumAddresses == 0) {
        return NULL;
    }
    return fAddressArray[0];
}

NetAddressList::Iterator::Iterator(NetAddressList const& addressList)
    : fAddressList(addressList), fNextIndex(0) {}

NetAddress const * NetAddressList::Iterator::nextAddress()
{
    if (fNextIndex >= fAddressList.numAddresses()) {
        return NULL;
    }
    return fAddressList.fAddressArray[fNextIndex++];
}

Port::Port(portNumBits num)
{
    fPortNum = htons(num);
}

UsageEnvironment& operator << (UsageEnvironment& s, const Port& p)
{
    return s << ntohs(p.num());
}

AddressPortLookupTable::AddressPortLookupTable()
    : fTable(HashTable::create(3))
{}

AddressPortLookupTable::~AddressPortLookupTable()
{
    delete fTable;
}

void * AddressPortLookupTable::Add(netAddressBits address1,
                                   netAddressBits address2,
                                   Port port, void* value)
{
    int key[3];

    key[0] = (int) address1;
    key[1] = (int) address2;
    key[2] = (int) port.num();
    return fTable->Add((char *) key, value);
}

void * AddressPortLookupTable::Lookup(netAddressBits address1,
                                      netAddressBits address2,
                                      Port           port)
{
    int key[3];

    key[0] = (int) address1;
    key[1] = (int) address2;
    key[2] = (int) port.num();
    return fTable->Lookup((char *) key);
}

Boolean AddressPortLookupTable::Remove(netAddressBits address1,
                                       netAddressBits address2,
                                       Port           port)
{
    int key[3];

    key[0] = (int) address1;
    key[1] = (int) address2;
    key[2] = (int) port.num();
    return fTable->Remove((char *) key);
}

AddressPortLookupTable::Iterator::Iterator(AddressPortLookupTable& table)
    : fIter(HashTable::Iterator::create(*(table.fTable)))
{}

AddressPortLookupTable::Iterator::~Iterator()
{
    delete fIter;
}

void * AddressPortLookupTable::Iterator::next()
{
    char const* key;

    return fIter->next(key);
}

Boolean IsMulticastAddress(netAddressBits address)
{
    netAddressBits addressInNetworkOrder = htonl(address);

    return addressInNetworkOrder > 0xE00000FF &&
           addressInNetworkOrder <= 0xEFFFFFFF;
}

AddressString::AddressString(struct sockaddr_in const& addr)
{
    init(addr.sin_addr.s_addr);
}

AddressString::AddressString(struct in_addr const& addr)
{
    init(addr.s_addr);
}

AddressString::AddressString(netAddressBits addr)
{
    init(addr);
}

void AddressString::init(netAddressBits addr)
{
    fVal = new char[16];
    netAddressBits addrNBO = htonl(addr);
    sprintf(fVal, "%u.%u.%u.%u", (addrNBO >> 24) & 0xFF, (addrNBO >> 16) & 0xFF, (addrNBO >> 8) & 0xFF, addrNBO & 0xFF);
}

AddressString::~AddressString()
{
    delete[] fVal;
}
