#include <android/log.h>
#include <dlfcn.h>
#include <stddef.h>
#include <stdlib.h>
#include <logging_macros.h>
#include <stdio.h>
#include "ladspa.h"
#include "plugin.h"

#define APPNAME "Plugin"

int plugin_init (state_t *state, const char * plugin_file, int index)
{
    /*	Load a LADSPA descriptor from shared library */
//    const char *error;

    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, "Going to load library\n");

    int err = 0;

    void* handle = dlopen(plugin_file, RTLD_LAZY);
    if (handle == NULL) {
        __android_log_print(ANDROID_LOG_ERROR, APPNAME, "Failed to load library: %s\n", dlerror ());
        err = 1 ;
        return err;
    }

    //~ *(void **)(&f) = dlsym (handle, "ladspa_descriptor") ;
    //~ (*f) ();

    LADSPA_Descriptor_Function fDescriptorFunction;
    dlerror();

    fDescriptorFunction = (LADSPA_Descriptor_Function)dlsym(handle,
                                                            "ladspa_descriptor");

//    const LADSPA_Descriptor * psDescriptor;
//    LADSPA_Descriptor_Function pfDescriptorFunction;

    state -> descriptor = fDescriptorFunction (0);
    state -> handle = state -> descriptor -> instantiate (state -> descriptor, state -> sample_rate) ;
    if (state -> handle == NULL) {
        LOGE("Unable to instantiate plugin: returned NULL\n");
    } else
        LOGD("Successfully instantiated plugin at %lu, you bloody bastard!\n", state -> sample_rate);
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
