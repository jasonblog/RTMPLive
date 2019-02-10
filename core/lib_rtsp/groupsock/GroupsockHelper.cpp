#include "GroupsockHelper.hh"
#if defined(__WIN32__) || defined(_WIN32)
# include <time.h>
extern "C" int initializeWinsockIfNecessary();
#else
# include <stdarg.h>
# include <time.h>
# include <fcntl.h>
# define initializeWinsockIfNecessary() 1
#endif
#include <stdio.h>
netAddressBits SendingInterfaceAddr   = INADDR_ANY;
netAddressBits ReceivingInterfaceAddr = INADDR_ANY;
static void socketErr(UsageEnvironment& env, char const* errorMsg)
{
    env.setResultErrMsg(errorMsg);
}

NoReuse::NoReuse(UsageEnvironment& env)
    : fEnv(env)
{
    groupsockPriv(fEnv)->reuseFlag = 0;
}

NoReuse::~NoReuse()
{
    groupsockPriv(fEnv)->reuseFlag = 1;
    reclaimGroupsockPriv(fEnv);
}

_groupsockPriv * groupsockPriv(UsageEnvironment& env)
{
    if (env.groupsockPriv == NULL) {
        _groupsockPriv* result = new _groupsockPriv;
        result->socketTable = NULL;
        result->reuseFlag   = 1;
        env.groupsockPriv   = result;
    }
    return (_groupsockPriv *) (env.groupsockPriv);
}

void reclaimGroupsockPriv(UsageEnvironment& env)
{
    _groupsockPriv* priv = (_groupsockPriv *) (env.groupsockPriv);

    if (priv->socketTable == NULL && priv->reuseFlag == 1) {
        delete priv;
        env.groupsockPriv = NULL;
    }
}

static int createSocket(int type)
{
    int sock;

    #ifdef SOCK_CLOEXEC
    sock = socket(AF_INET, type | SOCK_CLOEXEC, 0);
    if (sock != -1 || errno != EINVAL) {
        return sock;
    }
    #endif
    sock = socket(AF_INET, type, 0);
    #ifdef FD_CLOEXEC
    if (sock != -1) {
        fcntl(sock, F_SETFD, FD_CLOEXEC);
    }
    #endif
    return sock;
}

int setupDatagramSocket(UsageEnvironment& env, Port port)
{
    if (!initializeWinsockIfNecessary()) {
        socketErr(env, "Failed to initialize 'winsock': ");
        return -1;
    }
    int newSocket = createSocket(SOCK_DGRAM);
    if (newSocket < 0) {
        socketErr(env, "unable to create datagram socket: ");
        return newSocket;
    }
    int reuseFlag = groupsockPriv(env)->reuseFlag;
    reclaimGroupsockPriv(env);
    if (setsockopt(newSocket, SOL_SOCKET, SO_REUSEADDR,
                   (const char *) &reuseFlag, sizeof reuseFlag) < 0)
    {
        socketErr(env, "setsockopt(SO_REUSEADDR) error: ");
        closeSocket(newSocket);
        return -1;
    }
    #if defined(__WIN32__) || defined(_WIN32)
    #else
    # ifdef SO_REUSEPORT
    if (setsockopt(newSocket, SOL_SOCKET, SO_REUSEPORT,
                   (const char *) &reuseFlag, sizeof reuseFlag) < 0)
    {
        socketErr(env, "setsockopt(SO_REUSEPORT) error: ");
        closeSocket(newSocket);
        return -1;
    }
    # endif
    # ifdef IP_MULTICAST_LOOP
    const u_int8_t loop = 1;
    if (setsockopt(newSocket, IPPROTO_IP, IP_MULTICAST_LOOP,
                   (const char *) &loop, sizeof loop) < 0)
    {
        socketErr(env, "setsockopt(IP_MULTICAST_LOOP) error: ");
        closeSocket(newSocket);
        return -1;
    }
    # endif
    #endif // if defined(__WIN32__) || defined(_WIN32)
    netAddressBits addr = INADDR_ANY;
    #if defined(__WIN32__) || defined(_WIN32)
    #else
    if (port.num() != 0 || ReceivingInterfaceAddr != INADDR_ANY) {
    #endif
    if (port.num() == 0) {
        addr = ReceivingInterfaceAddr;
    }
    MAKE_SOCKADDR_IN(name, addr, port.num());
    if (bind(newSocket, (struct sockaddr *) &name, sizeof name) != 0) {
        char tmpBuffer[100];
        sprintf(tmpBuffer, "bind() error (port number: %d): ",
                ntohs(port.num()));
        socketErr(env, tmpBuffer);
        closeSocket(newSocket);
        return -1;
    }
    #if defined(__WIN32__) || defined(_WIN32)
    #else
} // setupDatagramSocket

    #endif
    if (SendingInterfaceAddr != INADDR_ANY) {
        struct in_addr addr;
        addr.s_addr = SendingInterfaceAddr;
        if (setsockopt(newSocket, IPPROTO_IP, IP_MULTICAST_IF,
                       (const char *) &addr, sizeof addr) < 0)
        {
            socketErr(env, "error setting outgoing multicast interface: ");
            closeSocket(newSocket);
            return -1;
        }
    }
    return newSocket;
}
Boolean makeSocketNonBlocking(int sock)
{
    #if defined(__WIN32__) || defined(_WIN32)
    unsigned long arg = 1;
    return ioctlsocket(sock, FIONBIO, &arg) == 0;

    #elif defined(VXWORKS)
    int arg = 1;
    return ioctl(sock, FIONBIO, (int) &arg) == 0;

    #else
    int curFlags = fcntl(sock, F_GETFL, 0);
    return fcntl(sock, F_SETFL, curFlags | O_NONBLOCK) >= 0;

    #endif // if defined(__WIN32__) || defined(_WIN32)
}

Boolean makeSocketBlocking(int sock, unsigned writeTimeoutInMilliseconds)
{
    Boolean result;

    #if defined(__WIN32__) || defined(_WIN32)
    unsigned long arg = 0;
    result = ioctlsocket(sock, FIONBIO, &arg) == 0;
    #elif defined(VXWORKS)
    int arg = 0;
    result = ioctl(sock, FIONBIO, (int) &arg) == 0;
    #else
    int curFlags = fcntl(sock, F_GETFL, 0);
    result = fcntl(sock, F_SETFL, curFlags & (~O_NONBLOCK)) >= 0;
    #endif
    if (writeTimeoutInMilliseconds > 0) {
        #ifdef SO_SNDTIMEO
        struct timeval tv;
        tv.tv_sec  = writeTimeoutInMilliseconds / 1000;
        tv.tv_usec = (writeTimeoutInMilliseconds % 1000) * 1000;
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *) &tv, sizeof tv);
        #endif
    }
    return result;
}

int setupStreamSocket(UsageEnvironment& env,
                      Port port, Boolean makeNonBlocking)
{
    if (!initializeWinsockIfNecessary()) {
        socketErr(env, "Failed to initialize 'winsock': ");
        return -1;
    }
    int newSocket = createSocket(SOCK_STREAM);
    if (newSocket < 0) {
        socketErr(env, "unable to create stream socket: ");
        return newSocket;
    }
    int reuseFlag = groupsockPriv(env)->reuseFlag;
    reclaimGroupsockPriv(env);
    if (setsockopt(newSocket, SOL_SOCKET, SO_REUSEADDR,
                   (const char *) &reuseFlag, sizeof reuseFlag) < 0)
    {
        socketErr(env, "setsockopt(SO_REUSEADDR) error: ");
        closeSocket(newSocket);
        return -1;
    }
    #ifdef REUSE_FOR_TCP
    # if defined(__WIN32__) || defined(_WIN32)
    # else
    #  ifdef SO_REUSEPORT
    if (setsockopt(newSocket, SOL_SOCKET, SO_REUSEPORT,
                   (const char *) &reuseFlag, sizeof reuseFlag) < 0)
    {
        socketErr(env, "setsockopt(SO_REUSEPORT) error: ");
        closeSocket(newSocket);
        return -1;
    }
    #  endif
    # endif
    #endif // ifdef REUSE_FOR_TCP
    #if defined(__WIN32__) || defined(_WIN32)
    #else
    if (port.num() != 0 || ReceivingInterfaceAddr != INADDR_ANY) {
    #endif
    MAKE_SOCKADDR_IN(name, ReceivingInterfaceAddr, port.num());
    if (bind(newSocket, (struct sockaddr *) &name, sizeof name) != 0) {
        char tmpBuffer[100];
        sprintf(tmpBuffer, "bind() error (port number: %d): ",
                ntohs(port.num()));
        socketErr(env, tmpBuffer);
        closeSocket(newSocket);
        return -1;
    }
    #if defined(__WIN32__) || defined(_WIN32)
    #else
} // setupStreamSocket

    #endif
    if (makeNonBlocking) {
        if (!makeSocketNonBlocking(newSocket)) {
            socketErr(env, "failed to make non-blocking: ");
            closeSocket(newSocket);
            return -1;
        }
    }
    return newSocket;
}
int readSocket(UsageEnvironment& env,
               int socket, unsigned char* buffer, unsigned bufferSize,
               struct sockaddr_in& fromAddress)
{
    SOCKLEN_T addressSize = sizeof fromAddress;
    int bytesRead         = recvfrom(socket, (char *) buffer, bufferSize, 0,
                                     (struct sockaddr *) &fromAddress,
                                     &addressSize);

    if (bytesRead < 0) {
        int err = env.getErrno();
        if (err == 111
            #if defined(__WIN32__) || defined(_WIN32)
            || err == 0 || err == EWOULDBLOCK
            #else
            || err == EAGAIN
            #endif
            || err == 113)
        {
            fromAddress.sin_addr.s_addr = 0;
            return 0;
        }
        socketErr(env, "recvfrom() error: ");
    } else if (bytesRead == 0) {
        return -1;
    }
    return bytesRead;
}

Boolean writeSocket(UsageEnvironment& env,
                    int socket, struct in_addr address, Port port,
                    u_int8_t ttlArg,
                    unsigned char* buffer, unsigned bufferSize)
{
    #if defined(__WIN32__) || defined(_WIN32)
    # define TTL_TYPE int
    #else
    # define TTL_TYPE u_int8_t
    #endif
    TTL_TYPE ttl = (TTL_TYPE) ttlArg;
    if (setsockopt(socket, IPPROTO_IP, IP_MULTICAST_TTL,
                   (const char *) &ttl, sizeof ttl) < 0)
    {
        socketErr(env, "setsockopt(IP_MULTICAST_TTL) error: ");
        return False;
    }
    return writeSocket(env, socket, address, port, buffer, bufferSize);
}

Boolean writeSocket(UsageEnvironment& env,
                    int socket, struct in_addr address, Port port,
                    unsigned char* buffer, unsigned bufferSize)
{
    do {
        MAKE_SOCKADDR_IN(dest, address.s_addr, port.num());
        int bytesSent = sendto(socket, (char *) buffer, bufferSize, 0,
                               (struct sockaddr *) &dest, sizeof dest);
        if (bytesSent != (int) bufferSize) {
            char tmpBuf[100];
            sprintf(tmpBuf, "writeSocket(%d), sendTo() error: wrote %d bytes instead of %u: ", socket, bytesSent,
                    bufferSize);
            socketErr(env, tmpBuf);
            break;
        }
        return True;
    } while (0);
    return False;
}

static unsigned getBufferSize(UsageEnvironment& env, int bufOptName,
                              int socket)
{
    unsigned curSize;
    SOCKLEN_T sizeSize = sizeof curSize;

    if (getsockopt(socket, SOL_SOCKET, bufOptName,
                   (char *) &curSize, &sizeSize) < 0)
    {
        socketErr(env, "getBufferSize() error: ");
        return 0;
    }
    return curSize;
}

unsigned getSendBufferSize(UsageEnvironment& env, int socket)
{
    return getBufferSize(env, SO_SNDBUF, socket);
}

unsigned getReceiveBufferSize(UsageEnvironment& env, int socket)
{
    return getBufferSize(env, SO_RCVBUF, socket);
}

static unsigned setBufferTo(UsageEnvironment& env, int bufOptName,
                            int socket, unsigned requestedSize)
{
    SOCKLEN_T sizeSize = sizeof requestedSize;

    setsockopt(socket, SOL_SOCKET, bufOptName, (char *) &requestedSize, sizeSize);
    return getBufferSize(env, bufOptName, socket);
}

unsigned setSendBufferTo(UsageEnvironment& env,
                         int socket, unsigned requestedSize)
{
    return setBufferTo(env, SO_SNDBUF, socket, requestedSize);
}

unsigned setReceiveBufferTo(UsageEnvironment& env,
                            int socket, unsigned requestedSize)
{
    return setBufferTo(env, SO_RCVBUF, socket, requestedSize);
}

static unsigned increaseBufferTo(UsageEnvironment& env, int bufOptName,
                                 int socket, unsigned requestedSize)
{
    unsigned curSize = getBufferSize(env, bufOptName, socket);

    while (requestedSize > curSize) {
        SOCKLEN_T sizeSize = sizeof requestedSize;
        if (setsockopt(socket, SOL_SOCKET, bufOptName,
                       (char *) &requestedSize, sizeSize) >= 0)
        {
            return requestedSize;
        }
        requestedSize = (requestedSize + curSize) / 2;
    }
    return getBufferSize(env, bufOptName, socket);
}

unsigned increaseSendBufferTo(UsageEnvironment& env,
                              int socket, unsigned requestedSize)
{
    return increaseBufferTo(env, SO_SNDBUF, socket, requestedSize);
}

unsigned increaseReceiveBufferTo(UsageEnvironment& env,
                                 int socket, unsigned requestedSize)
{
    return increaseBufferTo(env, SO_RCVBUF, socket, requestedSize);
}

static void clearMulticastAllSocketOption(int socket)
{
    #ifdef IP_MULTICAST_ALL
    int multicastAll = 0;
    (void) setsockopt(socket, IPPROTO_IP, IP_MULTICAST_ALL, (void *) &multicastAll, sizeof multicastAll);
    #endif
}

Boolean socketJoinGroup(UsageEnvironment& env, int socket,
                        netAddressBits groupAddress)
{
    if (!IsMulticastAddress(groupAddress)) {
        return True;
    }
    struct ip_mreq imr;
    imr.imr_multiaddr.s_addr = groupAddress;
    imr.imr_interface.s_addr = ReceivingInterfaceAddr;
    if (setsockopt(socket, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                   (const char *) &imr, sizeof(struct ip_mreq)) < 0)
    {
        #if defined(__WIN32__) || defined(_WIN32)
        if (env.getErrno() != 0) {
        #endif
        socketErr(env, "setsockopt(IP_ADD_MEMBERSHIP) error: ");
        return False;

        #if defined(__WIN32__) || defined(_WIN32)
    }
        #endif
    }
    clearMulticastAllSocketOption(socket);
    return True;
}

Boolean socketLeaveGroup(UsageEnvironment&, int socket,
                         netAddressBits groupAddress)
{
    if (!IsMulticastAddress(groupAddress)) {
        return True;
    }
    struct ip_mreq imr;
    imr.imr_multiaddr.s_addr = groupAddress;
    imr.imr_interface.s_addr = ReceivingInterfaceAddr;
    if (setsockopt(socket, IPPROTO_IP, IP_DROP_MEMBERSHIP,
                   (const char *) &imr, sizeof(struct ip_mreq)) < 0)
    {
        return False;
    }
    return True;
}

#if !defined(IP_ADD_SOURCE_MEMBERSHIP)
struct ip_mreq_source {
    struct  in_addr imr_multiaddr;
    struct  in_addr imr_sourceaddr;
    struct  in_addr imr_interface;
};
#endif
#ifndef IP_ADD_SOURCE_MEMBERSHIP
# ifdef LINUX
#  define IP_ADD_SOURCE_MEMBERSHIP  39
#  define IP_DROP_SOURCE_MEMBERSHIP 40
# else
#  define IP_ADD_SOURCE_MEMBERSHIP  25
#  define IP_DROP_SOURCE_MEMBERSHIP 26
# endif
#endif
Boolean socketJoinGroupSSM(UsageEnvironment& env, int socket,
                           netAddressBits groupAddress,
                           netAddressBits sourceFilterAddr)
{
    if (!IsMulticastAddress(groupAddress)) {
        return True;
    }
    struct ip_mreq_source imr;
    #ifdef __ANDROID__
    imr.imr_multiaddr  = groupAddress;
    imr.imr_sourceaddr = sourceFilterAddr;
    imr.imr_interface  = ReceivingInterfaceAddr;
    #else
    imr.imr_multiaddr.s_addr  = groupAddress;
    imr.imr_sourceaddr.s_addr = sourceFilterAddr;
    imr.imr_interface.s_addr  = ReceivingInterfaceAddr;
    #endif
    if (setsockopt(socket, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP,
                   (const char *) &imr, sizeof(struct ip_mreq_source)) < 0)
    {
        socketErr(env, "setsockopt(IP_ADD_SOURCE_MEMBERSHIP) error: ");
        return False;
    }
    clearMulticastAllSocketOption(socket);
    return True;
}

Boolean socketLeaveGroupSSM(UsageEnvironment&, int socket,
                            netAddressBits groupAddress,
                            netAddressBits sourceFilterAddr)
{
    if (!IsMulticastAddress(groupAddress)) {
        return True;
    }
    struct ip_mreq_source imr;
    #ifdef __ANDROID__
    imr.imr_multiaddr  = groupAddress;
    imr.imr_sourceaddr = sourceFilterAddr;
    imr.imr_interface  = ReceivingInterfaceAddr;
    #else
    imr.imr_multiaddr.s_addr  = groupAddress;
    imr.imr_sourceaddr.s_addr = sourceFilterAddr;
    imr.imr_interface.s_addr  = ReceivingInterfaceAddr;
    #endif
    if (setsockopt(socket, IPPROTO_IP, IP_DROP_SOURCE_MEMBERSHIP,
                   (const char *) &imr, sizeof(struct ip_mreq_source)) < 0)
    {
        return False;
    }
    return True;
}

static Boolean getSourcePort0(int socket, portNumBits& resultPortNum)
{
    sockaddr_in test;

    test.sin_port = 0;
    SOCKLEN_T len = sizeof test;
    if (getsockname(socket, (struct sockaddr *) &test, &len) < 0) {
        return False;
    }
    resultPortNum = ntohs(test.sin_port);
    return True;
}

Boolean getSourcePort(UsageEnvironment& env, int socket, Port& port)
{
    portNumBits portNum = 0;

    if (!getSourcePort0(socket, portNum) || portNum == 0) {
        MAKE_SOCKADDR_IN(name, INADDR_ANY, 0);
        bind(socket, (struct sockaddr *) &name, sizeof name);
        if (!getSourcePort0(socket, portNum) || portNum == 0) {
            socketErr(env, "getsockname() error: ");
            return False;
        }
    }
    port = Port(portNum);
    return True;
}

static Boolean badAddressForUs(netAddressBits addr)
{
    netAddressBits nAddr = htonl(addr);

    return (nAddr == 0x7F000001 ||
            nAddr == 0 ||
            nAddr == (netAddressBits) (~0));
}

Boolean loopbackWorks = 1;
netAddressBits ourIPAddress(UsageEnvironment& env)
{
    static netAddressBits ourAddress = 0;
    int sock = -1;
    struct in_addr testAddr;

    if (ReceivingInterfaceAddr != INADDR_ANY) {
        ourAddress = ReceivingInterfaceAddr;
    }
    if (ourAddress == 0) {
        struct sockaddr_in fromAddr;
        fromAddr.sin_addr.s_addr = 0;
        do {
            loopbackWorks   = 0;
            testAddr.s_addr = our_inet_addr("228.67.43.91");
            Port testPort(15947);
            sock = setupDatagramSocket(env, testPort);
            if (sock < 0) {
                break;
            }
            if (!socketJoinGroup(env, sock, testAddr.s_addr)) {
                break;
            }
            unsigned char testString[] = "hostIdTest";
            unsigned testStringLength  = sizeof testString;
            if (!writeSocket(env, sock, testAddr, testPort, 0,
                             testString, testStringLength))
            {
                break;
            }
            fd_set rd_set;
            FD_ZERO(&rd_set);
            FD_SET((unsigned) sock, &rd_set);
            const unsigned numFds = sock + 1;
            struct timeval timeout;
            timeout.tv_sec  = 5;
            timeout.tv_usec = 0;
            int result = select(numFds, &rd_set, NULL, NULL, &timeout);
            if (result <= 0) {
                break;
            }
            unsigned char readBuffer[20];
            int bytesRead = readSocket(env, sock,
                                       readBuffer, sizeof readBuffer,
                                       fromAddr);
            if (bytesRead != (int) testStringLength ||
                strncmp((char *) readBuffer, (char *) testString, testStringLength) != 0)
            {
                break;
            }
            loopbackWorks = !badAddressForUs(fromAddr.sin_addr.s_addr);
        } while (0);
        if (sock >= 0) {
            socketLeaveGroup(env, sock, testAddr.s_addr);
            closeSocket(sock);
        }
        if (!loopbackWorks) {
            do {
                char hostname[100];
                hostname[0] = '\0';
                int result = gethostname(hostname, sizeof hostname);
                if (result != 0 || hostname[0] == '\0') {
                    env.setResultErrMsg("initial gethostname() failed");
                    break;
                }
                NetAddressList addresses(hostname);
                NetAddressList::Iterator iter(addresses);
                NetAddress const* address;
                netAddressBits addr = 0;
                while ((address = iter.nextAddress()) != NULL) {
                    netAddressBits a = *(netAddressBits *) (address->data());
                    if (!badAddressForUs(a)) {
                        addr = a;
                        break;
                    }
                }
                fromAddr.sin_addr.s_addr = addr;
            } while (0);
        }
        netAddressBits from = fromAddr.sin_addr.s_addr;
        if (badAddressForUs(from)) {
            char tmp[100];
            sprintf(tmp, "This computer has an invalid IP address: %s", AddressString(from).val());
            env.setResultMsg(tmp);
            from = 0;
        }
        ourAddress = from;
        struct timeval timeNow;
        gettimeofday(&timeNow, NULL);
        unsigned seed = ourAddress ^ timeNow.tv_sec ^ timeNow.tv_usec;
        our_srandom(seed);
    }
    return ourAddress;
} // ourIPAddress

netAddressBits chooseRandomIPv4SSMAddress(UsageEnvironment& env)
{
    (void) ourIPAddress(env);
    netAddressBits const first = 0xE8000100, lastPlus1 = 0xE8FFFFFF;
    netAddressBits const range = lastPlus1 - first;
    return ntohl(first + ((netAddressBits) our_random()) % range);
}

char const * timestampString()
{
    struct timeval tvNow;

    gettimeofday(&tvNow, NULL);
    #if !defined(_WIN32_WCE)
    static char timeString[9];
    char const* ctimeResult = ctime((time_t *) &tvNow.tv_sec);
    if (ctimeResult == NULL) {
        sprintf(timeString, "??:??:??");
    } else {
        char const* from = &ctimeResult[11];
        int i;
        for (i = 0; i < 8; ++i) {
            timeString[i] = from[i];
        }
        timeString[i] = '\0';
    }
    #else // if !defined(_WIN32_WCE)
    static char timeString[50];
    sprintf(timeString, "%lu.%06ld", tvNow.tv_sec, tvNow.tv_usec);
    #endif // if !defined(_WIN32_WCE)
    return (char const *) &timeString;
}

#if defined(__WIN32__) || defined(_WIN32)
static LONG initializeLock_gettimeofday = 0;
# if !defined(_WIN32_WCE)
#  include <sys/timeb.h>
# endif
int gettimeofday(struct timeval* tp, int *)
{
    static LARGE_INTEGER tickFrequency, epochOffset;
    static Boolean isInitialized = False;
    LARGE_INTEGER tickNow;

    # if !defined(_WIN32_WCE)
    QueryPerformanceCounter(&tickNow);
    # else
    tickNow.QuadPart = GetTickCount();
    # endif
    if (!isInitialized) {
        if (1 == InterlockedIncrement(&initializeLock_gettimeofday)) {
            # if !defined(_WIN32_WCE)
            struct timeb tb;
            ftime(&tb);
            tp->tv_sec  = tb.time;
            tp->tv_usec = 1000 * tb.millitm;
            QueryPerformanceFrequency(&tickFrequency);
            # else
            const LONGLONG epoch = 116444736000000000LL;
            FILETIME fileTime;
            LARGE_INTEGER time;
            GetSystemTimeAsFileTime(&fileTime);
            time.HighPart = fileTime.dwHighDateTime;
            time.LowPart  = fileTime.dwLowDateTime;
            tp->tv_sec    = (long) ((time.QuadPart - epoch) / 10000000L);
            tp->tv_usec   = 0;
            tickFrequency.QuadPart = 1000;
            # endif // if !defined(_WIN32_WCE)
            epochOffset.QuadPart =
                tp->tv_sec * tickFrequency.QuadPart + (tp->tv_usec * tickFrequency.QuadPart) / 1000000L
                - tickNow.QuadPart;
            isInitialized = True;
            return 0;
        } else {
            InterlockedDecrement(&initializeLock_gettimeofday);
            while (!isInitialized) {
                Sleep(1);
            }
        }
    }
    tickNow.QuadPart += epochOffset.QuadPart;
    tp->tv_sec        = (long) (tickNow.QuadPart / tickFrequency.QuadPart);
    tp->tv_usec       = (long) (((tickNow.QuadPart % tickFrequency.QuadPart) * 1000000L) / tickFrequency.QuadPart);
    return 0;
} // gettimeofday

#endif // if defined(__WIN32__) || defined(_WIN32)
