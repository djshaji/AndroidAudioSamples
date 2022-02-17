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
    Plugin * p = libraries.at(sharedLibrary)->plugins.at(index) ;
    LOGD("adding plugin %s", p -> descriptor -> Name);
    activePlugins.push_back(p);
    OUT;
    return ;
    Plugin * plugin = new Plugin (libraries.at (sharedLibrary)->descriptorFunction, index);
    LOGV("adding %s from %s to rack", plugin -> descriptor->Name, libraries.at (sharedLibrary)->so_file.c_str());

    // I had forgot to do this
    plugin -> setSampleRate(sampleRate) ;
    plugin -> activate(sampleRate);

    activePlugins.push_back(plugin);
    activePlugins_ptr [0]  = plugin;
    OUT
}

void PluginManager::process (LADSPA_Data * inputData, LADSPA_Data * outputData, unsigned long samplesToProcess) {
    // How cool is this: very
//    IN
//    LOGD("going to run plugin %s", activePlugins.at(0)->Name);
    for (Plugin *plugin: activePlugins) {
//        LOGD("running plugin %s", plugin->Name);
        LOGD("input port: %d output port: %d", plugin->inputPort, plugin->outputPort);
        if (!plugin->active)
            continue;
        if (plugin -> inputPort != -1)
            plugin -> descriptor -> connect_port (plugin -> handle, plugin -> inputPort, (LADSPA_Data *) inputData);
        if (plugin -> outputPort != -1)
            plugin -> descriptor -> connect_port (plugin -> handle, plugin -> outputPort, (LADSPA_Data *) outputData);
        /*
        if (plugin -> inputPort == -1 && plugin -> outputPort == -1)
            LOGE ("no ports connected for %s, nothing to do!", plugin->Name);
        else {
            LOGD("running plugin [%s] input %d output %d for %lu samples",
                 plugin->Name, plugin->inputPort, plugin->outputPort, samplesToProcess);
            if (plugin->run == NULL) {
                LOGF("-----------| run function is NULL !!! |--------------") ;
            } else
                plugin->run(plugin->handle, samplesToProcess);
        }
        */

//        for (int i = 0 ; i < plugin->descriptor->PortCount; i++) {
//            LOGF("port info: %s [%d] -> %f", plugin->descriptor->PortNames[i], i, plugin->pluginControls[i]->val);
//        }

        float cutoff = 200 ;
        plugin->descriptor->connect_port (plugin->handle, 0, &cutoff);
        plugin->descriptor->run(plugin->handle, samplesToProcess);
    }
//    OUT
}

PluginManager::PluginManager(PluginManager *pManager) {

}
