#include "MPEG2TransportFileServerMediaSubsession.hh"
#include "SimpleRTPSink.hh"
MPEG2TransportFileServerMediaSubsession * MPEG2TransportFileServerMediaSubsession::createNew(UsageEnvironment& env,
                                                                                             char const*     fileName,
                                                                                             char const*     indexFileName,
                                                                                             Boolean         reuseFirstSource)
{
    MPEG2TransportStreamIndexFile* indexFile;

    if (indexFileName != NULL && reuseFirstSource) {
        env
            <<
            "MPEG2TransportFileServerMediaSubsession::createNew(): ignoring the index file name, because \"reuseFirstSource\" is set\n";
        indexFile = NULL;
    } else {
        indexFile = MPEG2TransportStreamIndexFile::createNew(env, indexFileName);
    }
    return new MPEG2TransportFileServerMediaSubsession(env, fileName, indexFile,
                                                       reuseFirstSource);
}

MPEG2TransportFileServerMediaSubsession
::MPEG2TransportFileServerMediaSubsession(UsageEnvironment               & env,
                                          char const*                    fileName,
                                          MPEG2TransportStreamIndexFile* indexFile,
                                          Boolean                        reuseFirstSource)
    : FileServerMediaSubsession(env, fileName, reuseFirstSource),
    fIndexFile(indexFile), fDuration(0.0), fClientSessionHashTable(NULL)
{
    if (fIndexFile != NULL) {
        fDuration = fIndexFile->getPlayingDuration();
        fClientSessionHashTable = HashTable::create(ONE_WORD_HASH_KEYS);
    }
}

MPEG2TransportFileServerMediaSubsession
::~MPEG2TransportFileServerMediaSubsession()
{
    if (fIndexFile != NULL) {
        Medium::close(fIndexFile);
        while (1) {
            ClientTrickPlayState* client =
                (ClientTrickPlayState *) (fClientSessionHashTable->RemoveNext());
            if (client == NULL) {
                break;
            }
            delete client;
        }
        delete fClientSessionHashTable;
    }
}

#define TRANSPORT_PACKET_SIZE                188
#define TRANSPORT_PACKETS_PER_NETWORK_PACKET 7
void MPEG2TransportFileServerMediaSubsession
::startStream(unsigned clientSessionId, void* streamToken, TaskFunc* rtcpRRHandler,
              void* rtcpRRHandlerClientData, unsigned short& rtpSeqNum,
              unsigned& rtpTimestamp,
              ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler,
              void* serverRequestAlternativeByteHandlerClientData)
{
    if (fIndexFile != NULL) {
        ClientTrickPlayState* client = lookupClient(clientSessionId);
        if (client != NULL && client->areChangingScale()) {
            client->updateStateOnPlayChange(True);
            OnDemandServerMediaSubsession::pauseStream(clientSessionId, streamToken);
            client->updateStateOnScaleChange();
        }
    }
    OnDemandServerMediaSubsession::startStream(clientSessionId, streamToken,
                                               rtcpRRHandler, rtcpRRHandlerClientData,
                                               rtpSeqNum, rtpTimestamp,
                                               serverRequestAlternativeByteHandler,
                                               serverRequestAlternativeByteHandlerClientData);
}

void MPEG2TransportFileServerMediaSubsession
::pauseStream(unsigned clientSessionId, void* streamToken)
{
    if (fIndexFile != NULL) {
        ClientTrickPlayState* client = lookupClient(clientSessionId);
        if (client != NULL) {
            client->updateStateOnPlayChange(False);
        }
    }
    OnDemandServerMediaSubsession::pauseStream(clientSessionId, streamToken);
}

void MPEG2TransportFileServerMediaSubsession
::seekStream(unsigned clientSessionId, void* streamToken, double& seekNPT, double streamDuration, u_int64_t& numBytes)
{
    OnDemandServerMediaSubsession::seekStream(clientSessionId, streamToken, seekNPT, streamDuration, numBytes);
    if (fIndexFile != NULL) {
        ClientTrickPlayState* client = lookupClient(clientSessionId);
        if (client != NULL) {
            unsigned long numTSPacketsToStream = client->updateStateFromNPT(seekNPT, streamDuration);
            numBytes = numTSPacketsToStream * TRANSPORT_PACKET_SIZE;
        }
    }
}

void MPEG2TransportFileServerMediaSubsession
::setStreamScale(unsigned clientSessionId, void* streamToken, float scale)
{
    if (fIndexFile != NULL) {
        ClientTrickPlayState* client = lookupClient(clientSessionId);
        if (client != NULL) {
            client->setNextScale(scale);
        }
    }
    OnDemandServerMediaSubsession::setStreamScale(clientSessionId, streamToken, scale);
}

void MPEG2TransportFileServerMediaSubsession
::deleteStream(unsigned clientSessionId, void *& streamToken)
{
    if (fIndexFile != NULL) {
        ClientTrickPlayState* client = lookupClient(clientSessionId);
        if (client != NULL) {
            client->updateStateOnPlayChange(False);
        }
    }
    OnDemandServerMediaSubsession::deleteStream(clientSessionId, streamToken);
}

ClientTrickPlayState * MPEG2TransportFileServerMediaSubsession::newClientTrickPlayState()
{
    return new ClientTrickPlayState(fIndexFile);
}

FramedSource * MPEG2TransportFileServerMediaSubsession
::createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate)
{
    unsigned const inputDataChunkSize =
        TRANSPORT_PACKETS_PER_NETWORK_PACKET * TRANSPORT_PACKET_SIZE;
    ByteStreamFileSource* fileSource =
        ByteStreamFileSource::createNew(envir(), fFileName, inputDataChunkSize);

    if (fileSource == NULL) {
        return NULL;
    }
    fFileSize = fileSource->fileSize();
    if (fFileSize > 0 && fDuration > 0.0) {
        estBitrate = (unsigned) ((int64_t) fFileSize / (125 * fDuration) + 0.5);
    } else {
        estBitrate = 5000;
    }
    MPEG2TransportStreamFramer* framer =
        MPEG2TransportStreamFramer::createNew(envir(), fileSource);
    if (fIndexFile != NULL) {
        ClientTrickPlayState* client = lookupClient(clientSessionId);
        if (client == NULL) {
            client = newClientTrickPlayState();
            fClientSessionHashTable->Add((char const *) clientSessionId, client);
        }
        client->setSource(framer);
    }
    return framer;
}

RTPSink * MPEG2TransportFileServerMediaSubsession
::createNewRTPSink(Groupsock* rtpGroupsock,
                   unsigned char,
                   FramedSource *)
{
    return SimpleRTPSink::createNew(envir(), rtpGroupsock,
                                    33, 90000, "video", "MP2T",
                                    1, True, False);
}

void MPEG2TransportFileServerMediaSubsession::testScaleFactor(float& scale)
{
    if (fIndexFile != NULL && fDuration > 0.0) {
        int iScale = scale < 0.0 ? (int) (scale - 0.5f) : (int) (scale + 0.5f);
        if (iScale == 0) {
            iScale = 1;
        }
        scale = (float) iScale;
    } else {
        scale = 1.0f;
    }
}

float MPEG2TransportFileServerMediaSubsession::duration() const
{
    return fDuration;
}

ClientTrickPlayState * MPEG2TransportFileServerMediaSubsession
::lookupClient(unsigned clientSessionId)
{
    return (ClientTrickPlayState *) (fClientSessionHashTable->Lookup((char const *) clientSessionId));
}

ClientTrickPlayState::ClientTrickPlayState(MPEG2TransportStreamIndexFile* indexFile)
    : fIndexFile(indexFile),
    fOriginalTransportStreamSource(NULL),
    fTrickModeFilter(NULL), fTrickPlaySource(NULL),
    fFramer(NULL),
    fScale(1.0f), fNextScale(1.0f), fNPT(0.0f),
    fTSRecordNum(0), fIxRecordNum(0)
{}

unsigned long ClientTrickPlayState::updateStateFromNPT(double npt, double streamDuration)
{
    fNPT = (float) npt;
    unsigned long tsRecordNum, ixRecordNum;
    fIndexFile->lookupTSPacketNumFromNPT(fNPT, tsRecordNum, ixRecordNum);
    updateTSRecordNum();
    if (tsRecordNum != fTSRecordNum) {
        fTSRecordNum = tsRecordNum;
        fIxRecordNum = ixRecordNum;
        reseekOriginalTransportStreamSource();
        fFramer->clearPIDStatusTable();
    }
    unsigned long numTSRecordsToStream = 0;
    float pcrLimit = 0.0;
    if (streamDuration > 0.0) {
        streamDuration += npt - (double) fNPT;
        if (streamDuration > 0.0) {
            if (fNextScale == 1.0f) {
                unsigned long toTSRecordNum, toIxRecordNum;
                float toNPT = (float) (fNPT + streamDuration);
                fIndexFile->lookupTSPacketNumFromNPT(toNPT, toTSRecordNum, toIxRecordNum);
                if (toTSRecordNum > tsRecordNum) {
                    numTSRecordsToStream = toTSRecordNum - tsRecordNum;
                }
            } else {
                int direction = fNextScale < 0.0 ? -1 : 1;
                pcrLimit = (float) (streamDuration / (fNextScale * direction));
            }
        }
    }
    fFramer->setNumTSPacketsToStream(numTSRecordsToStream);
    fFramer->setPCRLimit(pcrLimit);
    return numTSRecordsToStream;
} // ClientTrickPlayState::updateStateFromNPT

void ClientTrickPlayState::updateStateOnScaleChange()
{
    fScale = fNextScale;
    if (fTrickPlaySource != NULL) {
        fTrickModeFilter->forgetInputSource();
        Medium::close(fTrickPlaySource);
        fTrickPlaySource = NULL;
        fTrickModeFilter = NULL;
    }
    if (fNextScale != 1.0f) {
        UsageEnvironment& env = fIndexFile->envir();
        fTrickModeFilter = MPEG2TransportStreamTrickModeFilter
                           ::createNew(env, fOriginalTransportStreamSource, fIndexFile, int(fNextScale));
        fTrickModeFilter->seekTo(fTSRecordNum, fIxRecordNum);
        fTrickPlaySource = MPEG2TransportStreamFromESSource::createNew(env);
        fTrickPlaySource->addNewVideoSource(fTrickModeFilter, fIndexFile->mpegVersion());
        fFramer->changeInputSource(fTrickPlaySource);
    } else {
        reseekOriginalTransportStreamSource();
        fFramer->changeInputSource(fOriginalTransportStreamSource);
    }
}

void ClientTrickPlayState::updateStateOnPlayChange(Boolean reverseToPreviousVSH)
{
    updateTSRecordNum();
    if (fTrickPlaySource == NULL) {
        fIndexFile->lookupPCRFromTSPacketNum(fTSRecordNum, reverseToPreviousVSH, fNPT, fIxRecordNum);
    } else {
        fIxRecordNum = fTrickModeFilter->nextIndexRecordNum();
        if ((long) fIxRecordNum < 0) {
            fIxRecordNum = 0;
        }
        unsigned long transportRecordNum;
        float pcr;
        u_int8_t offset, size, recordType;
        if (fIndexFile->readIndexRecordValues(fIxRecordNum, transportRecordNum,
                                              offset, size, pcr, recordType))
        {
            fTSRecordNum = transportRecordNum;
            fNPT         = pcr;
        }
    }
}

void ClientTrickPlayState::setSource(MPEG2TransportStreamFramer* framer)
{
    fFramer = framer;
    fOriginalTransportStreamSource = (ByteStreamFileSource *) (framer->inputSource());
}

void ClientTrickPlayState::updateTSRecordNum()
{
    if (fFramer != NULL) {
        fTSRecordNum += (unsigned long) (fFramer->tsPacketCount());
    }
}

void ClientTrickPlayState::reseekOriginalTransportStreamSource()
{
    u_int64_t tsRecordNum64 = (u_int64_t) fTSRecordNum;

    fOriginalTransportStreamSource->seekToByteAbsolute(tsRecordNum64 * TRANSPORT_PACKET_SIZE);
}
