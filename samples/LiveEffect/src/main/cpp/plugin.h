#ifndef PLUGIN_H
#define PLUGIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ladspa.h"

typedef struct {
    char is_label;
    union { unsigned long uid; char *label; } id;
} plugin_id_t;

typedef struct {
    char * plugin_file ;
    const char * Name, * Label ;
    unsigned long plugin_index ;
    char *client_name;
    int input_port, output_port ; // mono for now
    LADSPA_PortDescriptor * portDescriptors ;
    const char **port_names; /* indexed by the LADSPA port index */
    unsigned long num_control_ports; /* input control ports */
    unsigned long num_meter_ports; /* output control ports */
    float *control_port_values; /* indexed by the LADSPA port index */
    LADSPA_Handle handle;
    const LADSPA_Descriptor *descriptor;
    void *library;
    unsigned long sample_rate ;
    int number_of_ports ;

    void (*connect_port)(LADSPA_Handle Instance,
                         unsigned long Port,
                         LADSPA_Data * DataLocation);
    void (*activate)(LADSPA_Handle Instance);
    void (*run)(LADSPA_Handle Instance,
                unsigned long SampleCount);
    void (*run_adding)(LADSPA_Handle Instance,
                       unsigned long SampleCount);

    // for passing data to JNI
    int * jni_control_ports ;
    float * jni_control_ports_minimums ;
    float * jni_control_ports_maximums ;
} state_t;

extern int plugin_init (state_t *state, const char * plugin_file, unsigned long plugin_no) ;
void plugin_connect_ports (state_t * state) ;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // PLUGIN_H
