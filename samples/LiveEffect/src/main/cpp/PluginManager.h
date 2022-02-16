#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <vector>
#include "ladspa.h"
#include "SharedLibrary.h"

class PluginManager {
public:
    PluginManager (unsigned long _sampleRate) {
        sampleRate = _sampleRate ;
    }

    PluginManager(PluginManager *pManager);

    std:: vector <std::string> default_plugins = {
            "libamp.so",
            "libnoise.so",
            "libsine.so",
            "libdelay.so",
            "libfilter.so"
    } ;

    unsigned long sampleRate ;
    std::vector<SharedLibrary *> libraries ;
    std::vector<Plugin *> activePlugins ;

    void process(LADSPA_Data *inputData, LADSPA_Data *outputData, unsigned long samplesToProcess);
    bool loadLibrary(std::string plugin_file);
    void loadLibraries();

    void addPluginToRack(unsigned long sharedLibrary, unsigned long index);
};


#endif //PLUGIN_MANAGER_H