#include <logging_macros.h>
#include <string>
#include "ladspa.h"

class Plugin {
public:
    const LADSPA_Descriptor *descriptor ;
    int library_index = 0 ;

    Plugin (LADSPA_Descriptor_Function descriptorFunction, int index) {
        descriptor = descriptorFunction (index) ;
        if (descriptor == NULL) {
            LOGE ("Failed to load plugin at index %d\n", index);
            return ;
        }

        LOGD ("Loaded plugin %s\n", descriptor -> Name) ;
    }
};