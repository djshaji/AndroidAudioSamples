#include <logging_macros.h>
#include <string>
#include <vector>
#include "ladspa.h"

/*
class PluginControlInit {
    char with_ini;
    LADSPA_Data ini;
    char with_def;
    LADSPA_Data def;
    char with_min;
    LADSPA_Data min;
    char with_max;
    LADSPA_Data max;
};
 */

class PluginControl {
    unsigned long port;
//    unsigned long ctrl;
    const char *name;
    const LADSPA_PortDescriptor *desc;
    const LADSPA_PortRangeHint *hint;
    /* values selected in the interface */
    LADSPA_Data sel;
    /* value range */
    LADSPA_Data min;
    LADSPA_Data max;
    LADSPA_Data *def;
    struct { LADSPA_Data fine; LADSPA_Data coarse; } inc;
    unsigned long sample_rate = 48000;

     enum Type {
        FLOAT,
        INT,
        TOGGLE
    };
    Type type ;

    LADSPA_Data control_rounding(LADSPA_Data val)
    {
        if (type == INT || type == TOGGLE)
            return nearbyintf(val);
        return val;
    }

    void set_value (float value) {
        val = &value ;
    }

    void set_sample_rate (unsigned long rate) {
        sample_rate = rate ;
    }

public:
    PluginControl(const LADSPA_Descriptor *descriptor, int _port) {
        IN ;
        port = _port ;
//        ctrl = _control ;
        desc = & descriptor -> PortDescriptors [port] ;
        hint = & descriptor -> PortRangeHints [port] ;
        LADSPA_PortRangeHintDescriptor ladspaPortRangeHintDescriptor = hint -> HintDescriptor;
        LADSPA_Data lower_bound = hint -> LowerBound;
        LADSPA_Data upper_bound = hint -> UpperBound;
        name = descriptor -> PortNames [port] ;

        /* control->min, control->max */
        if (LADSPA_IS_HINT_SAMPLE_RATE(ladspaPortRangeHintDescriptor)) {
            lower_bound *= sample_rate;
            upper_bound *= sample_rate;
        }

        if ( LADSPA_IS_HINT_BOUNDED_BELOW(ladspaPortRangeHintDescriptor) &&
             LADSPA_IS_HINT_BOUNDED_ABOVE(ladspaPortRangeHintDescriptor) )
        {
            min = lower_bound;
            max = upper_bound;
        }
        else if (LADSPA_IS_HINT_BOUNDED_BELOW(ladspaPortRangeHintDescriptor)) {
            min = lower_bound;
            max = 1.0;
        }
        else if (LADSPA_IS_HINT_BOUNDED_ABOVE(ladspaPortRangeHintDescriptor)) {
            min = 0.0;
            max = upper_bound;
        }
        else {
            min = -1.0;
            max = 1.0;
        }
        /* control->def */
        if (LADSPA_IS_HINT_HAS_DEFAULT(ladspaPortRangeHintDescriptor)) {
            /// TODO: Free this memory
            def = (LADSPA_Data *)malloc(sizeof(LADSPA_Data));
            if (def == NULL) {
                LOGE("Failed to allocate memory!");
                return ;
                OUT;
            }
            switch (ladspaPortRangeHintDescriptor & LADSPA_HINT_DEFAULT_MASK) {
                case LADSPA_HINT_DEFAULT_MINIMUM:
                    *def = lower_bound;
                    break;
                case LADSPA_HINT_DEFAULT_LOW:
                    *def = lower_bound * 0.75 + upper_bound * 0.25;
                    break;
                case LADSPA_HINT_DEFAULT_MIDDLE:
                    *def = lower_bound * 0.5 + upper_bound * 0.5;
                    break;
                case LADSPA_HINT_DEFAULT_HIGH:
                    *def = lower_bound * 0.25 + upper_bound * 0.75;
                    break;
                case LADSPA_HINT_DEFAULT_MAXIMUM:
                    *def = upper_bound;
                    break;
                case LADSPA_HINT_DEFAULT_0:
                    *def = 0.0;
                    break;
                case LADSPA_HINT_DEFAULT_1:
                    *def = 1.0;
                    break;
                case LADSPA_HINT_DEFAULT_100:
                    *def = 100.0;
                    break;
                case LADSPA_HINT_DEFAULT_440:
                    *def = 440.0;
                    break;
                default:
                    free(def), def = NULL;
                    LOGV("[plugin] %s has no defaults", name);
            }
        }
        else
            def = NULL;

        /* Check the default */
        if (def) {
            if (*def < min) {
                LOGD("[plugin] %s: default smaller than the minimum", name);
                *def = min;
            }
            if (*def > max) {
                LOGD("[plugin] %s: default greater than the maximum\n", name);
                *def = max;
            }
        }

        /* control->inc & Overrides */
        if (LADSPA_IS_HINT_TOGGLED(ladspaPortRangeHintDescriptor)) {
            min = 0.0;
            max = 1.0;
            inc.fine = 1.0;
            inc.coarse = 1.0;
            type = TOGGLE;
            if (def) * def = nearbyintf(*def);
        }
        else if (LADSPA_IS_HINT_INTEGER(ladspaPortRangeHintDescriptor)) {
            min = nearbyintf(min);
            max = nearbyintf(max);
            inc.fine = 1.0;
            inc.coarse = 1.0;
            type = INT ;
            if (def) *def = nearbyintf(*def);
        }
        else {
            inc.fine = (max - min) / 500;
            inc.coarse = (max - min) / 50;
            type = FLOAT;
        }

        /* control->sel, control->val */
        if (def)
            sel = *def;
        else
            sel = min;
        val = &sel;

        if (! def)
            LOGD("[plugin] %s: found control %s <%f - %f> no default value", descriptor ->Name, name, lower_bound, upper_bound);
        else
            LOGD("[plugin] %s: found control %s <%f - %f> default value %f", descriptor ->Name, name, lower_bound, upper_bound, *def);
        OUT ;
    }

/* value in the plugin */
LADSPA_Data *val;
};

class Plugin {
public:
    std::vector<PluginControl> pluginControls ;
    const LADSPA_Descriptor *descriptor ;
    // mono for now
    unsigned long inputPort = -1 ;
    unsigned long outputPort = -1 ;
    LADSPA_Handle handle ;
    int library_index = 0 ;
    unsigned long input_control_ports ;
    unsigned long output_control_ports ; // meter ports
    float * control_port_values; /* indexed by the LADSPA port index */
    unsigned long sample_rate = 48000 ; // default on my phone

    void setSampleRate (unsigned long _sample_rate) {
        sample_rate = _sample_rate ;
    }

    Plugin (LADSPA_Descriptor_Function descriptorFunction, int index) {
        IN
        descriptor = descriptorFunction (index) ;
        if (descriptor == NULL) {
            LOGE ("Failed to load plugin at index %d\n", index);
            return ;
        }

        LOGD ("Loaded plugin %s\n", descriptor -> Name) ;
        OUT
    }

    void activate (unsigned long _sample_rate) {
        IN
        sample_rate = _sample_rate ;
        handle = descriptor -> instantiate (descriptor, sample_rate);
        setupControls();
        if (descriptor -> activate)
            descriptor -> activate (handle) ;
        OUT
    }

    void setupControls () {
        IN ;
        for (int i = 0 ; i < descriptor -> PortCount ; i ++) {
            PluginControl pluginControl = PluginControl (descriptor, i);
            if (LADSPA_IS_PORT_AUDIO(descriptor -> PortDescriptors [i]) && LADSPA_IS_PORT_INPUT(descriptor -> PortDescriptors [i]))
                inputPort = i ;
            if (LADSPA_IS_PORT_AUDIO(descriptor -> PortDescriptors [i]) && LADSPA_IS_PORT_OUTPUT(descriptor -> PortDescriptors [i]))
                outputPort = i ;
            else {
                descriptor -> connect_port (handle, i, pluginControl .val);
            }

            pluginControls.push_back(pluginControl);
        }
        OUT ;
    }
};