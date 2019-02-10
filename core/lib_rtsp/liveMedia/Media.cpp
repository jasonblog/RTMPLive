#include "Media.hh"
#include "HashTable.hh"
Medium::Medium(UsageEnvironment& env)
    : fEnviron(env), fNextTask(NULL)
{
    MediaLookupTable::ourMedia(env)->generateNewName(fMediumName, mediumNameMaxLen);
    env.setResultMsg(fMediumName);
    MediaLookupTable::ourMedia(env)->addNew(this, fMediumName);
}

Medium::~Medium()
{
    fEnviron.taskScheduler().unscheduleDelayedTask(fNextTask);
}

Boolean Medium::lookupByName(UsageEnvironment& env, char const* mediumName,
                             Medium *& resultMedium)
{
    resultMedium = MediaLookupTable::ourMedia(env)->lookup(mediumName);
    if (resultMedium == NULL) {
        env.setResultMsg("Medium ", mediumName, " does not exist");
        return False;
    }
    return True;
}

void Medium::close(UsageEnvironment& env, char const* name)
{
    MediaLookupTable::ourMedia(env)->remove(name);
}

void Medium::close(Medium* medium)
{
    if (medium == NULL) {
        return;
    }
    close(medium->envir(), medium->name());
}

Boolean Medium::isSource() const
{
    return False;
}

Boolean Medium::isSink() const
{
    return False;
}

Boolean Medium::isRTCPInstance() const
{
    return False;
}

Boolean Medium::isRTSPClient() const
{
    return False;
}

Boolean Medium::isRTSPServer() const
{
    return False;
}

Boolean Medium::isMediaSession() const
{
    return False;
}

Boolean Medium::isServerMediaSession() const
{
    return False;
}

Boolean Medium::isDarwinInjector() const
{
    return False;
}

_Tables * _Tables::getOurTables(UsageEnvironment& env, Boolean createIfNotPresent)
{
    if (env.liveMediaPriv == NULL && createIfNotPresent) {
        env.liveMediaPriv = new _Tables(env);
    }
    return (_Tables *) (env.liveMediaPriv);
}

void _Tables::reclaimIfPossible()
{
    if (mediaTable == NULL && socketTable == NULL) {
        fEnv.liveMediaPriv = NULL;
        delete this;
    }
}

_Tables::_Tables(UsageEnvironment& env)
    : mediaTable(NULL), socketTable(NULL), fEnv(env)
{}

_Tables::~_Tables()
{}

MediaLookupTable * MediaLookupTable::ourMedia(UsageEnvironment& env)
{
    _Tables* ourTables = _Tables::getOurTables(env);

    if (ourTables->mediaTable == NULL) {
        ourTables->mediaTable = new MediaLookupTable(env);
    }
    return ourTables->mediaTable;
}

Medium * MediaLookupTable::lookup(char const* name) const
{
    return (Medium *) (fTable->Lookup(name));
}

void MediaLookupTable::addNew(Medium* medium, char* mediumName)
{
    fTable->Add(mediumName, (void *) medium);
}

void MediaLookupTable::remove(char const* name)
{
    Medium* medium = lookup(name);

    if (medium != NULL) {
        fTable->Remove(name);
        if (fTable->IsEmpty()) {
            _Tables* ourTables = _Tables::getOurTables(fEnv);
            delete this;
            ourTables->mediaTable = NULL;
            ourTables->reclaimIfPossible();
        }
        delete medium;
    }
}

void MediaLookupTable::generateNewName(char* mediumName,
                                       unsigned)
{
    sprintf(mediumName, "liveMedia%d", fNameGenerator++);
}

MediaLookupTable::MediaLookupTable(UsageEnvironment& env)
    : fEnv(env), fTable(HashTable::create(STRING_HASH_KEYS)), fNameGenerator(0)
{}

MediaLookupTable::~MediaLookupTable()
{
    delete fTable;
}
