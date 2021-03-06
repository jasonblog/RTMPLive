#ifndef WEBRTC_COMMON_H_
#define WEBRTC_COMMON_H_
#include <map>
#include "webrtc/base/basictypes.h"
#include "webrtc/base/constructormagic.h"
namespace webrtc
{
enum class ConfigOptionID {
    kMyExperimentForTest,
    kAlgo1CostFunctionForTest,
    kTemporalLayersFactory,
    kNetEqCapacityConfig,
    kNetEqFastAccelerate,
    kVoicePacing,
    kExtendedFilter,
    kDelayAgnostic,
    kExperimentalAgc,
    kExperimentalNs,
    kBeamforming,
    kIntelligibility,
    kEchoCanceller3,
    kAecRefinedAdaptiveFilter
};
class Config
{
public:
    template<typename T> const T& Get() const;
    template<typename T> void Set(T* value);
    Config() {}
    ~Config()
    {
        for (OptionMap::iterator it = options_.begin();
             it != options_.end(); ++it) {
            delete it->second;
        }
    }
private:
    struct BaseOption {
        virtual ~BaseOption() {}
    };
    template<typename T>
    struct Option : BaseOption {
        explicit Option(T* v): value(v) {}
        ~Option()
        {
            delete value;
        }
        T* value;
    };
    template<typename T>
    static ConfigOptionID identifier()
    {
        return T::identifier;
    }
    template<typename T>
    static const T& default_value()
    {
        RTC_DEFINE_STATIC_LOCAL(const T, def, ());
        return def;
    }
    typedef std::map<ConfigOptionID, BaseOption*> OptionMap;
    OptionMap options_;
    Config(const Config&);
    void operator=(const Config&);
};
template<typename T>
const T& Config::Get() const
{
    OptionMap::const_iterator it = options_.find(identifier<T>());
    if (it != options_.end()) {
        const T* t = static_cast<Option<T>*>(it->second)->value;
        if (t) {
            return *t;
        }
    }
    return default_value<T>();
}
template<typename T>
void Config::Set(T* value)
{
    BaseOption*& it = options_[identifier<T>()];
    delete it;
    it = new Option<T>(value);
}
}  
#endif  
