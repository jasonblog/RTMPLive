#include "FramedFileSource.hh"
FramedFileSource::FramedFileSource(UsageEnvironment& env, FILE* fid)
    : FramedSource(env), fFid(fid)
{}

FramedFileSource::~FramedFileSource()
{}
