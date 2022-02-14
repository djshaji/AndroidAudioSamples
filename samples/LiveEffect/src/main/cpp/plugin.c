#include <android/log.h>
#include <math.h>
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
        state -> activate = state -> descriptor -> activate ;
    } else {
        state -> activate = NULL ;
    }

    /// TODO: Free this memory!
    state -> plugin_file = strdup (plugin_file);
    state -> plugin_index = plugin_no ;

    // Some direct references
    state -> connect_port = state -> descriptor -> connect_port ;
    state -> run = state -> descriptor -> run ;
    state -> Name = state -> descriptor -> Name ;
    state -> Label = state -> descriptor -> Label ;

    plugin_connect_ports(state);
    OUT ;
    return err;
}

void plugin_connect_ports (state_t * state) {
    state -> input_port = state -> output_port = -1 ;

    state->portDescriptors = (LADSPA_PortDescriptor *)calloc
            ((size_t)state->descriptor->PortCount, sizeof(LADSPA_PortDescriptor));
    state->control_port_values = (LADSPA_Data *)calloc
            ((size_t)state->descriptor->PortCount, sizeof(LADSPA_Data));
    state->port_names =
            (const char **)calloc(state->descriptor->PortCount, sizeof(char *));
    state -> number_of_ports = state -> descriptor -> PortCount ;
    int i = 0 ;
    for (i = 0 ; i < state -> descriptor -> PortCount; i ++) {
        LOGD("\n-------------------------------------------------\n");
        LOGD("found port [%d]: %s\n", i, state -> descriptor -> PortNames [i]) ;
        if (LADSPA_IS_PORT_CONTROL(state->descriptor->PortDescriptors[i])) {
            LOGD("\t\t......... is a control port ... ");
            state -> control_port_values [i] = state -> descriptor -> PortRangeHints [i] . LowerBound / state -> descriptor -> PortRangeHints [i] . UpperBound ;
            // hack
            if (state -> control_port_values [i] == 0 || isnan (state -> control_port_values [i]))
                state -> control_port_values [i] = 1 ;
            LOGD("setting a sane default value of %f ", state -> control_port_values [i]);
            state -> connect_port (state -> handle, i, &state -> control_port_values [i]);
        }

        if (LADSPA_IS_PORT_INPUT(state->descriptor->PortDescriptors[i]) && LADSPA_IS_PORT_AUDIO(state->descriptor->PortDescriptors[i])) {
            LOGD("\t\t......... is an input port ");
            state ->input_port = i ;
        }
        if (LADSPA_IS_PORT_OUTPUT(state->descriptor->PortDescriptors[i]) && LADSPA_IS_PORT_AUDIO(state->descriptor->PortDescriptors[i])) {
            LOGD("\t\t......... is an output port ");
            state -> output_port = i ;
        }
        LOGD(" from %f to %f ", state -> descriptor -> PortRangeHints [i] . LowerBound, state -> descriptor -> PortRangeHints [i] . UpperBound);

        // set sane default control values

        state -> port_names [i] = state -> descriptor -> PortNames [i] ;
        state -> portDescriptors [i] = state -> descriptor -> PortDescriptors [i] ;
    }
}
