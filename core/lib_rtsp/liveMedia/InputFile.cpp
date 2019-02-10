#include "InputFile.hh"
#include <string.h>
FILE * OpenInputFile(UsageEnvironment& env, char const* fileName)
{
    FILE* fid;

    if (strcmp(fileName, "stdin") == 0) {
        fid = stdin;
        #if (defined(__WIN32__) || defined(_WIN32)) && !defined(_WIN32_WCE)
        _setmode(_fileno(stdin), _O_BINARY);
        #endif
    } else {
        fid = fopen(fileName, "rb");
        if (fid == NULL) {
            env.setResultMsg("unable to open file \"", fileName, "\"");
        }
    }
    return fid;
}

void CloseInputFile(FILE* fid)
{
    if (fid != NULL && fid != stdin) {
        fclose(fid);
    }
}

u_int64_t GetFileSize(char const* fileName, FILE* fid)
{
    u_int64_t fileSize = 0;

    if (fid != stdin) {
        #if !defined(_WIN32_WCE)
        if (fileName == NULL) {
        #endif
        if (fid != NULL && SeekFile64(fid, 0, SEEK_END) >= 0) {
            fileSize = (u_int64_t) TellFile64(fid);
            if (fileSize == (u_int64_t) -1) {
                fileSize = 0;
            }
            SeekFile64(fid, 0, SEEK_SET);
        }
        #if !defined(_WIN32_WCE)
    } else {
        struct stat sb;
        if (stat(fileName, &sb) == 0) {
            fileSize = sb.st_size;
        }
    }
        #endif
    }
    return fileSize;
}

int64_t SeekFile64(FILE* fid, int64_t offset, int whence)
{
    if (fid == NULL) {
        return -1;
    }
    clearerr(fid);
    fflush(fid);
    #if (defined(__WIN32__) || defined(_WIN32)) && !defined(_WIN32_WCE)
    return _lseeki64(_fileno(fid), offset, whence) == (int64_t) -1 ? -1 : 0;

    #else
    # if defined(_WIN32_WCE)
    return fseek(fid, (long) (offset), whence);

    # else
    return fseeko(fid, (off_t) (offset), whence);

    # endif
    #endif
}

int64_t TellFile64(FILE* fid)
{
    if (fid == NULL) {
        return -1;
    }
    clearerr(fid);
    fflush(fid);
    #if (defined(__WIN32__) || defined(_WIN32)) && !defined(_WIN32_WCE)
    return _telli64(_fileno(fid));

    #else
    # if defined(_WIN32_WCE)
    return ftell(fid);

    # else
    return ftello(fid);

    # endif
    #endif
}

Boolean FileIsSeekable(FILE* fid)
{
    if (SeekFile64(fid, 1, SEEK_CUR) < 0) {
        return False;
    }
    SeekFile64(fid, -1, SEEK_CUR);
    return True;
}
