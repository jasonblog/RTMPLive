#include "JPEGVideoSource.hh"
JPEGVideoSource::JPEGVideoSource(UsageEnvironment& env)
    : FramedSource(env)
{}

JPEGVideoSource::~JPEGVideoSource()
{}

u_int8_t const * JPEGVideoSource::quantizationTables(u_int8_t & precision,
                                                     u_int16_t& length)
{
    precision = 0;
    length    = 0;
    return NULL;
}

u_int16_t JPEGVideoSource::restartInterval()
{
    return 0;
}

Boolean JPEGVideoSource::isJPEGVideoSource() const
{
    return True;
}
