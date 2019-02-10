#if (defined(__WIN32__) || defined(_WIN32)) && !defined(_WIN32_WCE)
# include <io.h>
# include <fcntl.h>
#endif
#ifndef _WIN32_WCE
# include <sys/stat.h>
#endif
#include <string.h>
#include "OutputFile.hh"
FILE * OpenOutputFile(UsageEnvironment& env, char const* fileName)
{
    FILE* fid;

    if (strcmp(fileName, "stdout") == 0) {
        fid = stdout;
        #if (defined(__WIN32__) || defined(_WIN32)) && !defined(_WIN32_WCE)
        _setmode(_fileno(stdout), _O_BINARY);
        #endif
    } else if (strcmp(fileName, "stderr") == 0) {
        fid = stderr;
        #if (defined(__WIN32__) || defined(_WIN32)) && !defined(_WIN32_WCE)
        _setmode(_fileno(stderr), _O_BINARY);
        #endif
    } else {
        fid = fopen(fileName, "wb");
    }
    if (fid == NULL) {
        env.setResultMsg("unable to open file \"", fileName, "\"");
    }
    return fid;
}

void CloseOutputFile(FILE* fid)
{
    if (fid != NULL && fid != stdout && fid != stderr) {
        fclose(fid);
    }
}
