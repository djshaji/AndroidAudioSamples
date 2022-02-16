#ifndef SHARED_LIBRARY_H
#define SHARED_LIBRARY_H

#include <dlfcn.h>
#include <logging_macros.h>
#include <cstring>
#include <string>
#include <vector>
#include "ladspa.h"
#include "Plugin.h"

class SharedLibrary {
public:
    std::vector <Plugin *> plugins ;
    std::string so_file ;
    int total_plugins = 0 ;
    void * dl_handle = NULL;
    unsigned long sampleRate ;
    LADSPA_Descriptor_Function descriptorFunction ;

    void setSampleRate (unsigned long _sampleRate) {
        sampleRate = _sampleRate ;
    }

    SharedLibrary (std::string plugin_file) {
        so_file = plugin_file ;
    }

    //> Returns NULL if ok, error otherwise
    char * load (void) {
        IN ;
        dl_handle = dlopen (so_file.c_str(), RTLD_LAZY);
        if (dl_handle == NULL) {
            char * err = dlerror () ;
            LOGE ("Failed to load library: %s\n", err);
            OUT ;
            return err ;
        }

        LOGD("dlopen [ok]. Looking for descriptor function ...");
        descriptorFunction = (LADSPA_Descriptor_Function) dlsym (dl_handle, "ladspa_descriptor");
        // count plugins
        if (descriptorFunction == NULL) {
            LOGE("Failed to find descriptor function") ;
        } else {
            LOGD("Descriptor function [ok] Counting plugins ...") ;
            for (total_plugins = 0;; total_plugins++) {
                const LADSPA_Descriptor *d = descriptorFunction(total_plugins);
                if (d == NULL) break;
            }

            LOGI("\t\t... found %d plugins", total_plugins);
        }

        OUT ;
        return NULL ;
    }

    void loadPlugins () {
        IN ;
        for (int i = 0 ; i < total_plugins ; i ++) {
            Plugin * plugin = new Plugin (descriptorFunction, i);
            plugin -> setSampleRate(sampleRate) ;
            plugin -> activate(sampleRate);
            LOGD("Loaded plugin %s: %s @ %d", so_file.c_str(), plugin->descriptor -> Name, i);
            plugins.push_back(plugin);
        }

        LOGV("Loaded %d plugins from %s", total_plugins, so_file.c_str());
        OUT ;
    }
};

#endif // SHARED_LIBRARY_H