#include "MPEG2TransportStreamIndexFile.hh"
#include "InputFile.hh"
MPEG2TransportStreamIndexFile
::MPEG2TransportStreamIndexFile(UsageEnvironment& env, char const* indexFileName)
    : Medium(env),
    fFileName(strDup(indexFileName)), fFid(NULL), fMPEGVersion(0), fCurrentIndexRecordNum(0),
    fCachedPCR(0.0f), fCachedTSPacketNumber(0), fNumIndexRecords(0)
{
    u_int64_t indexFileSize = GetFileSize(indexFileName, NULL);

    if (indexFileSize % INDEX_RECORD_SIZE != 0) {
        env << "Warning: Size of the index file \"" << indexFileName
            << "\" (" << (unsigned) indexFileSize
            << ") is not a multiple of the index record size ("
            << INDEX_RECORD_SIZE << ")\n";
    }
    fNumIndexRecords = (unsigned long) (indexFileSize / INDEX_RECORD_SIZE);
}

MPEG2TransportStreamIndexFile * MPEG2TransportStreamIndexFile
::createNew(UsageEnvironment& env, char const* indexFileName)
{
    if (indexFileName == NULL) {
        return NULL;
    }
    MPEG2TransportStreamIndexFile* indexFile =
        new MPEG2TransportStreamIndexFile(env, indexFileName);
    if (indexFile->getPlayingDuration() == 0.0f) {
        delete indexFile;
        indexFile = NULL;
    }
    return indexFile;
}

MPEG2TransportStreamIndexFile::~MPEG2TransportStreamIndexFile()
{
    closeFid();
    delete[] fFileName;
}

void MPEG2TransportStreamIndexFile
::lookupTSPacketNumFromNPT(float& npt, unsigned long& tsPacketNumber,
                           unsigned long& indexRecordNumber)
{
    if (npt <= 0.0 || fNumIndexRecords == 0) {
        npt = 0.0f;
        tsPacketNumber = indexRecordNumber = 0;
        return;
    }
    if (npt == fCachedPCR) {
        tsPacketNumber    = fCachedTSPacketNumber;
        indexRecordNumber = fCachedIndexRecordNumber;
        return;
    }
    Boolean success       = False;
    unsigned long ixFound = 0;
    do {
        unsigned long ixLeft = 0, ixRight = fNumIndexRecords - 1;
        float pcrLeft = 0.0f, pcrRight;
        if (!readIndexRecord(ixRight)) {
            break;
        }
        pcrRight = pcrFromBuf();
        if (npt > pcrRight) {
            npt = pcrRight;
        }
        while (ixRight - ixLeft > 1 && pcrLeft < npt && npt <= pcrRight) {
            unsigned long ixNew = ixLeft
                                  + (unsigned long) (((npt - pcrLeft) / (pcrRight - pcrLeft)) * (ixRight - ixLeft));
            if (ixNew == ixLeft || ixNew == ixRight) {
                ixNew = (ixLeft + ixRight) / 2;
            }
            if (!readIndexRecord(ixNew)) {
                break;
            }
            float pcrNew = pcrFromBuf();
            if (pcrNew < npt) {
                pcrLeft = pcrNew;
                ixLeft  = ixNew;
            } else {
                pcrRight = pcrNew;
                ixRight  = ixNew;
            }
        }
        if (ixRight - ixLeft > 1 || npt <= pcrLeft || npt > pcrRight) {
            break;
        }
        ixFound = ixRight;
        success = rewindToCleanPoint(ixFound);
    } while (0);
    if (success && readIndexRecord(ixFound)) {
        npt = fCachedPCR = pcrFromBuf();
        tsPacketNumber    = fCachedTSPacketNumber = tsPacketNumFromBuf();
        indexRecordNumber = fCachedIndexRecordNumber = ixFound;
    } else {
        npt = 0.0f;
        tsPacketNumber = indexRecordNumber = 0;
    }
    closeFid();
} // MPEG2TransportStreamIndexFile::lookupTSPacketNumFromNPT

void MPEG2TransportStreamIndexFile
::lookupPCRFromTSPacketNum(unsigned long& tsPacketNumber, Boolean reverseToPreviousCleanPoint,
                           float& pcr, unsigned long& indexRecordNumber)
{
    if (tsPacketNumber == 0 || fNumIndexRecords == 0) {
        pcr = 0.0f;
        indexRecordNumber = 0;
        return;
    }
    if (tsPacketNumber == fCachedTSPacketNumber) {
        pcr = fCachedPCR;
        indexRecordNumber = fCachedIndexRecordNumber;
        return;
    }
    Boolean success       = False;
    unsigned long ixFound = 0;
    do {
        unsigned long ixLeft = 0, ixRight = fNumIndexRecords - 1;
        unsigned long tsLeft = 0, tsRight;
        if (!readIndexRecord(ixRight)) {
            break;
        }
        tsRight = tsPacketNumFromBuf();
        if (tsPacketNumber > tsRight) {
            tsPacketNumber = tsRight;
        }
        while (ixRight - ixLeft > 1 && tsLeft < tsPacketNumber && tsPacketNumber <= tsRight) {
            unsigned long ixNew = ixLeft
                                  + (unsigned long) (((tsPacketNumber - tsLeft) / (tsRight - tsLeft))
                                                     * (ixRight - ixLeft));
            if (ixNew == ixLeft || ixNew == ixRight) {
                ixNew = (ixLeft + ixRight) / 2;
            }
            if (!readIndexRecord(ixNew)) {
                break;
            }
            unsigned long tsNew = tsPacketNumFromBuf();
            if (tsNew < tsPacketNumber) {
                tsLeft = tsNew;
                ixLeft = ixNew;
            } else {
                tsRight = tsNew;
                ixRight = ixNew;
            }
        }
        if (ixRight - ixLeft > 1 || tsPacketNumber <= tsLeft || tsPacketNumber > tsRight) {
            break;
        }
        ixFound = ixRight;
        if (reverseToPreviousCleanPoint) {
            success = rewindToCleanPoint(ixFound);
        } else {
            success = True;
        }
    } while (0);
    if (success && readIndexRecord(ixFound)) {
        pcr = fCachedPCR = pcrFromBuf();
        fCachedTSPacketNumber = tsPacketNumFromBuf();
        if (reverseToPreviousCleanPoint) {
            tsPacketNumber = fCachedTSPacketNumber;
        }
        indexRecordNumber = fCachedIndexRecordNumber = ixFound;
    } else {
        pcr = 0.0f;
        indexRecordNumber = 0;
    }
    closeFid();
} // MPEG2TransportStreamIndexFile::lookupPCRFromTSPacketNum

Boolean MPEG2TransportStreamIndexFile
::readIndexRecordValues(unsigned long indexRecordNum,
                        unsigned long& transportPacketNum, u_int8_t& offset,
                        u_int8_t& size, float& pcr, u_int8_t& recordType)
{
    if (!readIndexRecord(indexRecordNum)) {
        return False;
    }
    transportPacketNum = tsPacketNumFromBuf();
    offset     = offsetFromBuf();
    size       = sizeFromBuf();
    pcr        = pcrFromBuf();
    recordType = recordTypeFromBuf();
    return True;
}

float MPEG2TransportStreamIndexFile::getPlayingDuration()
{
    if (fNumIndexRecords == 0 || !readOneIndexRecord(fNumIndexRecords - 1)) {
        return 0.0f;
    }
    return pcrFromBuf();
}

int MPEG2TransportStreamIndexFile::mpegVersion()
{
    if (fMPEGVersion != 0) {
        return fMPEGVersion;
    }
    if (!readOneIndexRecord(0)) {
        return 0;
    }
    setMPEGVersionFromRecordType(recordTypeFromBuf());
    return fMPEGVersion;
}

Boolean MPEG2TransportStreamIndexFile::openFid()
{
    if (fFid == NULL && fFileName != NULL) {
        if ((fFid = OpenInputFile(envir(), fFileName)) != NULL) {
            fCurrentIndexRecordNum = 0;
        }
    }
    return fFid != NULL;
}

Boolean MPEG2TransportStreamIndexFile::seekToIndexRecord(unsigned long indexRecordNumber)
{
    if (!openFid()) {
        return False;
    }
    if (indexRecordNumber == fCurrentIndexRecordNum) {
        return True;
    }
    if (SeekFile64(fFid, (int64_t) (indexRecordNumber * INDEX_RECORD_SIZE), SEEK_SET) != 0) {
        return False;
    }
    fCurrentIndexRecordNum = indexRecordNumber;
    return True;
}

Boolean MPEG2TransportStreamIndexFile::readIndexRecord(unsigned long indexRecordNum)
{
    do {
        if (!seekToIndexRecord(indexRecordNum)) {
            break;
        }
        if (fread(fBuf, INDEX_RECORD_SIZE, 1, fFid) != 1) {
            break;
        }
        ++fCurrentIndexRecordNum;
        return True;
    } while (0);
    return False;
}

Boolean MPEG2TransportStreamIndexFile::readOneIndexRecord(unsigned long indexRecordNum)
{
    Boolean result = readIndexRecord(indexRecordNum);

    closeFid();
    return result;
}

void MPEG2TransportStreamIndexFile::closeFid()
{
    if (fFid != NULL) {
        CloseInputFile(fFid);
        fFid = NULL;
    }
}

float MPEG2TransportStreamIndexFile::pcrFromBuf()
{
    unsigned pcr_int  = (fBuf[5] << 16) | (fBuf[4] << 8) | fBuf[3];
    u_int8_t pcr_frac = fBuf[6];

    return pcr_int + pcr_frac / 256.0f;
}

unsigned long MPEG2TransportStreamIndexFile::tsPacketNumFromBuf()
{
    return (fBuf[10] << 24) | (fBuf[9] << 16) | (fBuf[8] << 8) | fBuf[7];
}

void MPEG2TransportStreamIndexFile::setMPEGVersionFromRecordType(u_int8_t recordType)
{
    if (fMPEGVersion != 0) {
        return;
    }
    u_int8_t const recordTypeWithoutStartBit = recordType & ~0x80;
    if (recordTypeWithoutStartBit >= 1 && recordTypeWithoutStartBit <= 4) {
        fMPEGVersion = 2;
    } else if (recordTypeWithoutStartBit >= 5 && recordTypeWithoutStartBit <= 10) {
        fMPEGVersion = 5;
    } else if (recordTypeWithoutStartBit >= 11 && recordTypeWithoutStartBit <= 16) {
        fMPEGVersion = 6;
    }
}

Boolean MPEG2TransportStreamIndexFile::rewindToCleanPoint(unsigned long& ixFound)
{
    Boolean success = False;

    while (ixFound > 0) {
        if (!readIndexRecord(ixFound)) {
            break;
        }
        u_int8_t recordType = recordTypeFromBuf();
        setMPEGVersionFromRecordType(recordType);
        if ((recordType & 0x80) != 0) {
            recordType &= ~0x80;
            if (fMPEGVersion == 5) {
                if (recordType == 5) {
                    success = True;
                    break;
                }
            } else if (fMPEGVersion == 6) {
                if (recordType == 11) {
                    success = True;
                    break;
                }
            } else {
                if (recordType == 1) {
                    success = True;
                    break;
                } else if (recordType == 2) {
                    unsigned long newIxFound = ixFound;
                    while (--newIxFound > 0) {
                        if (!readIndexRecord(newIxFound)) {
                            break;
                        }
                        recordType = recordTypeFromBuf();
                        if ((recordType & 0x7F) != 1) {
                            break;
                        }
                        if ((recordType & 0x80) != 0) {
                            ixFound = newIxFound;
                            break;
                        }
                    }
                }
                success = True;
                break;
            }
        }
        --ixFound;
    }
    if (ixFound == 0) {
        success = True;
    }
    return success;
} // MPEG2TransportStreamIndexFile::rewindToCleanPoint
