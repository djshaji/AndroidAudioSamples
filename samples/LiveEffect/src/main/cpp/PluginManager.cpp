#include "PluginManager.h"

bool PluginManager::loadLibrary (std::string pluginfile) {
    IN ;
    LOGD("Going to load shared library %s", pluginfile.c_str());
    SharedLibrary * library = new SharedLibrary (pluginfile);
    char * err = library -> load() ;
    if (err == NULL) {
        LOGD("Loaded shared library %s\n", pluginfile.c_str());
        library ->setSampleRate(sampleRate);
        library -> loadPlugins() ;
        LOGD("Loaded plugins for %s", pluginfile.c_str());
        libraries.push_back(library);
        OUT ;
        return true ;
    } else {
        LOGE("Failed to load shared library %s: %s", pluginfile.c_str(), err);
        OUT ;
        return false ;
    }
}

void PluginManager::loadLibraries () {
    IN ;
    // So I learnt this today
    // how cool is this: very
    for (std::string library : default_plugins) {
        loadLibrary(library);
        LOGV("loaded shared library: %s", library.c_str());
    }
    OUT ;
}

void PluginManager::addPluginToRack (unsigned long sharedLibrary, unsigned long index) {
    IN
    Plugin * plugin = new Plugin (libraries.at (sharedLibrary)->descriptorFunction, index);
    LOGV("adding %s from %s to rack", plugin -> descriptor->Name, libraries.at (sharedLibrary)->so_file.c_str());
    activePlugins.push_back(plugin);
    OUT
}

void PluginManager::process (LADSPA_Data * inputData, LADSPA_Data * outputData, unsigned long samplesToProcess) {
    // How cool is this: very
    for (Plugin *plugin: activePlugins) {
        if (!plugin->active)
            continue;
        if (plugin -> inputPort != -1)
            plugin -> descriptor -> connect_port (plugin -> handle, plugin -> inputPort, (LADSPA_Data *) inputData);
        if (plugin -> outputPort != -1)
            plugin -> descriptor -> connect_port (plugin -> handle, plugin -> outputPort, (LADSPA_Data *) outputData);
        plugin -> descriptor -> run (plugin-> handle, samplesToProcess);
    }
}

PluginManager::PluginManager(PluginManager *pManager) {

}
