#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <vector>
#include "ladspa.h"
#include "SharedLibrary.h"

class PluginManager {
public:
    PluginManager (unsigned long _sampleRate) {
        sampleRate = _sampleRate ;
        activePlugins_ptr = reinterpret_cast<Plugin **>(static_cast<Plugin *>(calloc(ACTIVE_PLUGIN_MAX,
                                                                                     sizeof(Plugin *))));
    }

    PluginManager(PluginManager *pManager);

    std:: vector <std::string> default_plugins = {
//            "libamp.so",
            "libnoise.so"
//            "libsine.so",
//            "libdelay.so",
//            "libfilter.so"
    } ;

    unsigned long sampleRate = 48000;
    int ACTIVE_PLUGIN_MAX = 99 ;
    std::vector<SharedLibrary *> libraries ;
    std::vector<Plugin *> activePlugins ;
    Plugin ** activePlugins_ptr ;

    void process(LADSPA_Data *inputData, LADSPA_Data *outputData, unsigned long samplesToProcess);
    bool loadLibrary(std::string plugin_file);
    void loadLibraries();

    void addPluginToRack(unsigned long sharedLibrary, unsigned long index);
};


#endif //PLUGIN_MANAGER_H