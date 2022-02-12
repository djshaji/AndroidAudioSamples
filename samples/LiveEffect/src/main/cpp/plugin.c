#include <android/log.h>
#include <dlfcn.h>
#include <stddef.h>
#include <stdlib.h>
#include <logging_macros.h>
#include <stdio.h>
#include <string.h>
#include "ladspa.h"
#include "plugin.h"

#define APPNAME "Plugin"

int plugin_init (state_t *state, const char * plugin_file, unsigned long plugin_no)
{
    IN ;
    /*	Load a LADSPA descriptor from shared library */
//    const char *error;

    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Going to load library\n");

    int err = 0;

    // This "handle" is used again to initiate the plugin later on.
    // Could *this* be the casue of the problem?
    state -> library = dlopen(plugin_file, RTLD_LAZY);
    if (state -> library == NULL) {
        __android_log_print(ANDROID_LOG_ERROR, APPNAME, "Failed to load library: %s\n", dlerror ());
        err = 1 ;
        return err;
    }

    LOGD("dlopen [ok]! Going to look for descriptor...\n");

    //~ *(void **)(&f) = dlsym (handle, "ladspa_descriptor") ;
    //~ (*f) ();

//    LADSPA_Descriptor *(* fDescriptorFunction)(unsigned long plugin_no);
//    fDescriptorFunction = (LADSPA_Descriptor *(*)(unsigned long))dlsym(state -> library,
//                                                            "ladspa_descriptor");

    LADSPA_Descriptor_Function fDescriptorFunction = dlsym(state -> library, "ladspa_descriptor");
//    const LADSPA_Descriptor * psDescriptor;
//    LADSPA_Descriptor_Function pfDescriptorFunction;

    if (fDescriptorFunction == NULL) {
        err = -1 ;
        LOGE("Failed to load shared library %s: %s\n", plugin_file, dlerror());
        return err ;
    } else {
        LOGD("LADSPA descriptor function [ok]! Going to look for descriptor ...\n");
    }

    state -> descriptor = fDescriptorFunction (plugin_no);
    if (state -> descriptor == NULL) {
        LOGE("Failed to load descriptor from plugin! Descriptor returned NULL at %lu. Possible error %s\n", plugin_no, dlerror());
        return 1 ;
    }

    LOGD("Descriptor loaded [ok]. Going to instantiate plugin...\n");

    if (state -> descriptor -> instantiate == NULL) {
        LOGE("Instantiate returned null. Something is wrong!\n[plugin] %s -> %s | %lu",
             plugin_file, state -> descriptor -> Label, plugin_no);
        return 1 ;
    }

    LOGV("descriptor %s, sample rate %lu\n", state -> descriptor -> Label, state -> sample_rate);
    state -> handle = state -> descriptor -> instantiate (state -> descriptor, state -> sample_rate) ;
    LOGV("instantiate [ok]\n");
    if (state -> handle == NULL) {
        LOGE("Unable to instantiate plugin: returned NULL\n");
        return 1 ;
    } else
        LOGD("Successfully instantiated %s at %lu of %s [ok]. Going to activate plugin ...\n",
             state -> descriptor -> Label,
             state -> sample_rate,
             plugin_file);

    //      activate is null for our sample plugin.
    if (state -> descriptor -> activate != NULL) {
        state -> descriptor -> activate (state -> handle);
        LOGD("Plugin activated: %s\n", state -> descriptor -> Name);
    }

    /// TODO: Free this memory!
    state -> plugin_file = strdup (plugin_file);
    state -> plugin_index = plugin_no ;

    // Some direct references
    state -> connect_port = state -> descriptor -> connect_port ;
    state -> run = state -> descriptor -> run ;
    state -> activate = state -> descriptor -> activate ;
    state -> Name = state -> descriptor -> Name ;
    state -> Label = state -> descriptor -> Label ;

    OUT ;
    return err;
}

int plugin_connect_ports (state_t * state) {
    int err = 0 ;
    char * error = NULL ;

    // Port names
    state -> port_names = (const char **)calloc(state->descriptor->PortCount, sizeof(char *));
    if (!state->port_names)
        err = 1, error = "memory allocation error";

    /* Allocate memory for the list of control port values. */
    state->control_port_values = (LADSPA_Data *)calloc
            ((size_t)state->descriptor->PortCount, sizeof(LADSPA_Data));
    if (!state->control_port_values)
        err = 1 ;

//    static int controlvout = 0;
//    static int controlvin = 0;
//    unsigned long p; /* loop variable for ports */
//    const LADSPA_PortDescriptor *port; /* loop variable for port descriptors */

    /* For our test plugin
     *  #define AMP_CONTROL 0
        #define AMP_INPUT1  1
        #define AMP_OUTPUT1 2
        #define AMP_INPUT2  3
        #define AMP_OUTPUT2 4
     */

    return err ;
}
