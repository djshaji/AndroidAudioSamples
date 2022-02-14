#ifndef SHARED_LIBRARY_H
#define SHARED_LIBRARY_H

#include <dlfcn.h>
#include <logging_macros.h>
#include <cstring>
#include <string>
#include <list>
#include "ladspa.h"
#include "Plugin.h"

class SharedLibrary {
public:
    std::list <Plugin> plugins ;
    std::string so_file ;
    int total_plugins = 0 ;
    void * dl_handle = NULL;
    LADSPA_Descriptor_Function descriptorFunction ;

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
        }

        OUT ;
        return NULL ;
    }

    void loadPlugins () {
        IN ;
        for (int i = 0 ; i < total_plugins ; i ++) {
            Plugin plugin = Plugin (descriptorFunction, i);
            plugins.push_front(plugin);
        }
        OUT ;
    }
};

#endif // SHARED_LIBRARY_H