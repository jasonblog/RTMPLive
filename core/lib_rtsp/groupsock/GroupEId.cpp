#include "GroupEId.hh"
#include "strDup.hh"
#include <string.h>
void Scope::assign(u_int8_t ttl, const char* publicKey)
{
    fTTL       = ttl;
    fPublicKey = strDup(publicKey == NULL ? "nokey" : publicKey);
}

void Scope::clean()
{
    delete[] fPublicKey;
    fPublicKey = NULL;
}

Scope::Scope(u_int8_t ttl, const char* publicKey)
{
    assign(ttl, publicKey);
}

Scope::Scope(const Scope& orig)
{
    assign(orig.ttl(), orig.publicKey());
}

Scope& Scope::operator = (const Scope& rightSide)
{
    if (&rightSide != this) {
        if (publicKey() == NULL ||
            strcmp(publicKey(), rightSide.publicKey()) != 0)
        {
            clean();
            assign(rightSide.ttl(), rightSide.publicKey());
        } else {
            fTTL = rightSide.ttl();
        }
    }
    return *this;
}

Scope::~Scope()
{
    clean();
}

unsigned Scope::publicKeySize() const
{
    return fPublicKey == NULL ? 0 : strlen(fPublicKey);
}

GroupEId::GroupEId(struct in_addr const& groupAddr,
                   portNumBits portNum, Scope const& scope,
                   unsigned numSuccessiveGroupAddrs)
{
    struct in_addr sourceFilterAddr;

    sourceFilterAddr.s_addr = ~0;
    init(groupAddr, sourceFilterAddr, portNum, scope, numSuccessiveGroupAddrs);
}

GroupEId::GroupEId(struct in_addr const& groupAddr,
                   struct in_addr const& sourceFilterAddr,
                   portNumBits         portNum,
                   unsigned            numSuccessiveGroupAddrs)
{
    init(groupAddr, sourceFilterAddr, portNum, 255, numSuccessiveGroupAddrs);
}

GroupEId::GroupEId()
{}

Boolean GroupEId::isSSM() const
{
    return fSourceFilterAddress.s_addr != netAddressBits(~0);
}

void GroupEId::init(struct in_addr const& groupAddr,
                    struct in_addr const& sourceFilterAddr,
                    portNumBits         portNum,
                    Scope const         & scope,
                    unsigned            numSuccessiveGroupAddrs)
{
    fGroupAddress            = groupAddr;
    fSourceFilterAddress     = sourceFilterAddr;
    fNumSuccessiveGroupAddrs = numSuccessiveGroupAddrs;
    fPortNum = portNum;
    fScope   = scope;
}
