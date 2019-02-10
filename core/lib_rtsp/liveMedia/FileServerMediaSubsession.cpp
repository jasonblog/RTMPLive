#include "FileServerMediaSubsession.hh"
FileServerMediaSubsession
::FileServerMediaSubsession(UsageEnvironment& env, char const* fileName,
                            Boolean reuseFirstSource)
    : OnDemandServerMediaSubsession(env, reuseFirstSource),
    fFileSize(0)
{
    fFileName = strDup(fileName);
}

FileServerMediaSubsession::~FileServerMediaSubsession()
{
    delete[](char *) fFileName;
}
