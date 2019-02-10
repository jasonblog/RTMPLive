#include "RTSPCommon.hh"
#include "Locale.hh"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#if defined(__WIN32__) || defined(_WIN32) || defined(_QNX4)
#else
# include <signal.h>
# define USE_SIGNALS 1
#endif
static void decodeURL(char* url)
{
    char* cursor = url;

    while (*cursor) {
        if ((cursor[0] == '%') &&
            cursor[1] && isxdigit(cursor[1]) &&
            cursor[2] && isxdigit(cursor[2]))
        {
            char hex[3];
            hex[0]  = cursor[1];
            hex[1]  = cursor[2];
            hex[2]  = '\0';
            *url++  = (char) strtol(hex, NULL, 16);
            cursor += 3;
        } else {
            *url++ = *cursor++;
        }
    }
    *url = '\0';
}

Boolean parseRTSPRequestString(char const* reqStr,
                               unsigned    reqStrSize,
                               char*       resultCmdName,
                               unsigned    resultCmdNameMaxSize,
                               char*       resultURLPreSuffix,
                               unsigned    resultURLPreSuffixMaxSize,
                               char*       resultURLSuffix,
                               unsigned    resultURLSuffixMaxSize,
                               char*       resultCSeq,
                               unsigned    resultCSeqMaxSize,
                               char*       resultSessionIdStr,
                               unsigned    resultSessionIdStrMaxSize,
                               unsigned    & contentLength)
{
    unsigned i;

    for (i = 0; i < reqStrSize; ++i) {
        char c = reqStr[i];
        if (!(c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\0')) {
            break;
        }
    }
    if (i == reqStrSize) {
        return False;
    }
    Boolean parseSucceeded = False;
    unsigned i1 = 0;
    for (; i1 < resultCmdNameMaxSize - 1 && i < reqStrSize; ++i, ++i1) {
        char c = reqStr[i];
        if (c == ' ' || c == '\t') {
            parseSucceeded = True;
            break;
        }
        resultCmdName[i1] = c;
    }
    resultCmdName[i1] = '\0';
    if (!parseSucceeded) {
        return False;
    }
    unsigned j = i + 1;
    while (j < reqStrSize && (reqStr[j] == ' ' || reqStr[j] == '\t')) {
        ++j;
    }
    for (; (int) j < (int) (reqStrSize - 8); ++j) {
        if ((reqStr[j] == 'r' || reqStr[j] == 'R') &&
            (reqStr[j + 1] == 't' || reqStr[j + 1] == 'T') &&
            (reqStr[j + 2] == 's' || reqStr[j + 2] == 'S') &&
            (reqStr[j + 3] == 'p' || reqStr[j + 3] == 'P') &&
            reqStr[j + 4] == ':' && reqStr[j + 5] == '/')
        {
            j += 6;
            if (reqStr[j] == '/') {
                ++j;
                while (j < reqStrSize && reqStr[j] != '/' && reqStr[j] != ' ') {
                    ++j;
                }
            } else {
                --j;
            }
            i = j;
            break;
        }
    }
    parseSucceeded = False;
    for (unsigned k = i + 1; (int) k < (int) (reqStrSize - 5); ++k) {
        if (reqStr[k] == 'R' && reqStr[k + 1] == 'T' &&
            reqStr[k + 2] == 'S' && reqStr[k + 3] == 'P' && reqStr[k + 4] == '/')
        {
            while (--k >= i && reqStr[k] == ' ') {}
            unsigned k1 = k;
            while (k1 > i && reqStr[k1] != '/') {
                --k1;
            }
            unsigned n = 0, k2 = k1 + 1;
            if (k2 <= k) {
                if (k - k1 + 1 > resultURLSuffixMaxSize) {
                    return False;
                }
                while (k2 <= k) {
                    resultURLSuffix[n++] = reqStr[k2++];
                }
            }
            resultURLSuffix[n] = '\0';
            n  = 0;
            k2 = i + 1;
            if (k2 + 1 <= k1) {
                if (k1 - i > resultURLPreSuffixMaxSize) {
                    return False;
                }
                while (k2 <= k1 - 1) {
                    resultURLPreSuffix[n++] = reqStr[k2++];
                }
            }
            resultURLPreSuffix[n] = '\0';
            decodeURL(resultURLPreSuffix);
            i = k + 7;
            parseSucceeded = True;
            break;
        }
    }
    if (!parseSucceeded) {
        return False;
    }
    parseSucceeded = False;
    for (j = i; (int) j < (int) (reqStrSize - 5); ++j) {
        if (_strncasecmp("CSeq:", &reqStr[j], 5) == 0) {
            j += 5;
            while (j < reqStrSize && (reqStr[j] == ' ' || reqStr[j] == '\t')) {
                ++j;
            }
            unsigned n;
            for (n = 0; n < resultCSeqMaxSize - 1 && j < reqStrSize; ++n, ++j) {
                char c = reqStr[j];
                if (c == '\r' || c == '\n') {
                    parseSucceeded = True;
                    break;
                }
                resultCSeq[n] = c;
            }
            resultCSeq[n] = '\0';
            break;
        }
    }
    if (!parseSucceeded) {
        return False;
    }
    resultSessionIdStr[0] = '\0';
    for (j = i; (int) j < (int) (reqStrSize - 8); ++j) {
        if (_strncasecmp("Session:", &reqStr[j], 8) == 0) {
            j += 8;
            while (j < reqStrSize && (reqStr[j] == ' ' || reqStr[j] == '\t')) {
                ++j;
            }
            unsigned n;
            for (n = 0; n < resultSessionIdStrMaxSize - 1 && j < reqStrSize; ++n, ++j) {
                char c = reqStr[j];
                if (c == '\r' || c == '\n') {
                    break;
                }
                resultSessionIdStr[n] = c;
            }
            resultSessionIdStr[n] = '\0';
            break;
        }
    }
    contentLength = 0;
    for (j = i; (int) j < (int) (reqStrSize - 15); ++j) {
        if (_strncasecmp("Content-Length:", &(reqStr[j]), 15) == 0) {
            j += 15;
            while (j < reqStrSize && (reqStr[j] == ' ' || reqStr[j] == '\t')) {
                ++j;
            }
            unsigned num;
            if (sscanf(&reqStr[j], "%u", &num) == 1) {
                contentLength = num;
            }
        }
    }
    return True;
} // parseRTSPRequestString

Boolean parseRangeParam(char const* paramStr,
                        double& rangeStart, double& rangeEnd,
                        char *& absStartTime, char *& absEndTime,
                        Boolean& startTimeIsNow)
{
    delete[] absStartTime;
    delete[] absEndTime;
    absStartTime   = absEndTime = NULL;
    startTimeIsNow = False;
    double start, end;
    int numCharsMatched1 = 0, numCharsMatched2 = 0, numCharsMatched3 = 0, numCharsMatched4 = 0;
    Locale l("C", Numeric);
    if (sscanf(paramStr, "npt = %lf - %lf", &start, &end) == 2) {
        rangeStart = start;
        rangeEnd   = end;
    } else if (sscanf(paramStr, "npt = %n%lf -", &numCharsMatched1, &start) == 1) {
        if (paramStr[numCharsMatched1] == '-') {
            rangeStart     = 0.0;
            startTimeIsNow = True;
            rangeEnd       = -start;
        } else {
            rangeStart = start;
            rangeEnd   = 0.0;
        }
    } else if (sscanf(paramStr, "npt = now - %lf", &end) == 1) {
        rangeStart     = 0.0;
        startTimeIsNow = True;
        rangeEnd       = end;
    } else if (sscanf(paramStr, "npt = now -%n", &numCharsMatched2) == 0 && numCharsMatched2 > 0) {
        rangeStart     = 0.0;
        startTimeIsNow = True;
        rangeEnd       = 0.0;
    } else if (sscanf(paramStr, "clock = %n", &numCharsMatched3) == 0 && numCharsMatched3 > 0) {
        rangeStart = rangeEnd = 0.0;
        char const* utcTimes = &paramStr[numCharsMatched3];
        size_t len       = strlen(utcTimes) + 1;
        char* as         = new char[len];
        char* ae         = new char[len];
        int sscanfResult = sscanf(utcTimes, "%[^-]-%s", as, ae);
        if (sscanfResult == 2) {
            absStartTime = as;
            absEndTime   = ae;
        } else if (sscanfResult == 1) {
            absStartTime = as;
            delete[] ae;
        } else {
            delete[] as;
            delete[] ae;
            return False;
        }
    } else if (sscanf(paramStr, "smtpe = %n", &numCharsMatched4) == 0 && numCharsMatched4 > 0) {} else {
        return False;
    }
    return True;
} // parseRangeParam

Boolean parseRangeHeader(char const* buf,
                         double& rangeStart, double& rangeEnd,
                         char *& absStartTime, char *& absEndTime,
                         Boolean& startTimeIsNow)
{
    while (1) {
        if (*buf == '\0') {
            return False;
        }
        if (_strncasecmp(buf, "Range: ", 7) == 0) {
            break;
        }
        ++buf;
    }
    char const* fields = buf + 7;
    while (*fields == ' ') {
        ++fields;
    }
    return parseRangeParam(fields, rangeStart, rangeEnd, absStartTime, absEndTime, startTimeIsNow);
}

Boolean parseScaleHeader(char const* buf, float& scale)
{
    scale = 1.0;
    while (1) {
        if (*buf == '\0') {
            return False;
        }
        if (_strncasecmp(buf, "Scale:", 6) == 0) {
            break;
        }
        ++buf;
    }
    char const* fields = buf + 6;
    while (*fields == ' ') {
        ++fields;
    }
    float sc;
    if (sscanf(fields, "%f", &sc) == 1) {
        scale = sc;
    } else {
        return False;
    }
    return True;
}

static Boolean isSeparator(char c)
{
    return c == ' ' || c == ',' || c == ';' || c == ':';
}

Boolean RTSPOptionIsSupported(char const* commandName, char const* optionsResponseString)
{
    do {
        if (commandName == NULL || optionsResponseString == NULL) {
            break;
        }
        unsigned const commandNameLen = strlen(commandName);
        if (commandNameLen == 0) {
            break;
        }
        while (1) {
            while (*optionsResponseString != '\0' && isSeparator(*optionsResponseString)) {
                ++optionsResponseString;
            }
            if (*optionsResponseString == '\0') {
                break;
            }
            if (strncmp(commandName, optionsResponseString, commandNameLen) == 0) {
                optionsResponseString += commandNameLen;
                if (*optionsResponseString == '\0' || isSeparator(*optionsResponseString)) {
                    return True;
                }
            }
            while (*optionsResponseString != '\0' && !isSeparator(*optionsResponseString)) {
                ++optionsResponseString;
            }
        }
    } while (0);
    return False;
}

char const * dateHeader()
{
    static char buf[200];

    #if !defined(_WIN32_WCE)
    time_t tt = time(NULL);
    strftime(buf, sizeof buf, "Date: %a, %b %d %Y %H:%M:%S GMT\r\n", gmtime(&tt));
    #else
    SYSTEMTIME SystemTime;
    GetSystemTime(&SystemTime);
    WCHAR dateFormat[] = L"ddd, MMM dd yyyy";
    WCHAR timeFormat[] = L"HH:mm:ss GMT\r\n";
    WCHAR inBuf[200];
    DWORD locale = LOCALE_NEUTRAL;
    int ret      = GetDateFormat(locale, 0, &SystemTime,
                                 (LPTSTR) dateFormat, (LPTSTR) inBuf, sizeof inBuf);
    inBuf[ret - 1] = ' ';
    ret = GetTimeFormat(locale, 0, &SystemTime,
                        (LPTSTR) timeFormat,
                        (LPTSTR) inBuf + ret, (sizeof inBuf) - ret);
    wcstombs(buf, inBuf, wcslen(inBuf));
    #endif // if !defined(_WIN32_WCE)
    return buf;
}

void ignoreSigPipeOnSocket(int socketNum)
{
    #ifdef USE_SIGNALS
    # ifdef SO_NOSIGPIPE
    int set_option = 1;
    setsockopt(socketNum, SOL_SOCKET, SO_NOSIGPIPE, &set_option, sizeof set_option);
    # else
    signal(SIGPIPE, SIG_IGN);
    # endif
    #endif
}
