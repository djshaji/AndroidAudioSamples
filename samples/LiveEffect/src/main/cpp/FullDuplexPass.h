/*
 * Copyright 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SAMPLES_FULLDUPLEXPASS_H
#define SAMPLES_FULLDUPLEXPASS_H

#include <dlfcn.h>
#include "FullDuplexStream.h"
#include "plugin.h"

class FullDuplexPass : public FullDuplexStream {
public:
    state_t * duplexPluginState = NULL;
    void set_plugin (state_t * s) {
        this -> duplexPluginState = s ;
        return ;
    }

    state_t * get_plugin (void) {
        return this -> duplexPluginState ;
    }

    /*
    void loadPlugin1(state_t  * ptr) {
        IN ;
        //! TODO: Free this memory!
//    ptr = static_cast<state_t *>(malloc(sizeof(state_t)));
        ptr -> sample_rate = mSampleRate ; // am I devops or what
//    if (plugin_init(&ptr, "libamp.so", 0))
        if (!plugin_init(ptr, "libnoise.so", 0))
            LOGD("Loaded plugin %s\n", ptr -> descriptor->Name);

        OUT ;
    }
     */

    virtual oboe::DataCallbackResult
    onBothStreamsReady(
            std::shared_ptr<oboe::AudioStream> inputStream,
            const void *inputData,
            int   numInputFrames,
            std::shared_ptr<oboe::AudioStream> outputStream,
            void *outputData,
            int   numOutputFrames) {
//        IN ;

        /*
        LADSPA_Descriptor_Function fDescriptorFunction ;
        *(void**)(&fDescriptorFunction) = dlsym(duplexPluginState -> library, "ladspa_descriptor");
        const LADSPA_Descriptor *descriptor = fDescriptorFunction (duplexPluginState -> plugin_index);
        LOGE("[plugin] %s\n", descriptor -> Label);
        */

        /*
        if (duplexPluginState == NULL) {
            LOGE("Loading shared library\n");
            duplexPluginState = static_cast<state_t *>(malloc(sizeof(state_t)));
            duplexPluginState -> sample_rate = 48000 ; // am I devops or what
            if (!plugin_init(duplexPluginState, "libnoise.so", 0))
                LOGD("Loaded plugin %s\n", duplexPluginState -> descriptor->Name);

        }*/

//        LOGE("[%lu] plugin is %s\n", duplexPluginState -> sample_rate, duplexPluginState -> Label);
        // Copy the input samples to the output with a little arbitrary gain change.

        // This code assumes the data format for both streams is Float.
        const float *inputFloats = static_cast<const float *>(inputData);
        float *outputFloats = static_cast<float *>(outputData);

        // It also assumes the channel count for each stream is the same.
        int32_t samplesPerFrame = outputStream->getChannelCount();
        int32_t numInputSamples = numInputFrames * samplesPerFrame;
        int32_t numOutputSamples = numOutputFrames * samplesPerFrame;

        // It is possible that there may be fewer input than output samples.
        int32_t samplesToProcess = std::min(numInputSamples, numOutputSamples);
        for (int32_t i = 0; i < samplesToProcess; i++) {
            *outputFloats++ = *inputFloats++ * 0.95; // do some arbitrary processing
        }

        // Apply LADSPA Plugin
        /* For our test plugin
         *  #define AMP_CONTROL 0
            #define AMP_INPUT1  1
            #define AMP_OUTPUT1 2
            #define AMP_INPUT2  3
            #define AMP_OUTPUT2 4
         */
        // ugly hack, dunno will work or not
        /*  as I see it, LADSPA control port is essentially a float containing value for some
         *  "control". So We just supply a dummy value here.
         *  Our sample plugin refuses to run without it.
         */

        /*  Oh fucking hell
         *  it fucking works
         *  aaaargh
         */

        /*  so it means that if our "control" value is essentially a "slider" value for
         *  some control, it means that we should be able to adjust "gain" for our sample
         *  by playing with it
         */

        /*  Important lesson here
         *  So listen up folks.
         *  Apparently it is necessary to cast variable types to their *proper*
         *  data types, else segmentation faults happen.
         */

//        float *control = reinterpret_cast<float *>(1);
//        LADSPA_Data dry_wet = 0.1 ;
//        LADSPA_Data delay_time = 1.0 ;
//        if (state -> descriptor == NULL) {
//            LOGE("Plugin descriptor is null\n");
//        }

//        LADSPA_Data cutoff = 60 ;

//        LADSPA_Data  amplitude = 1 ;
//        duplexPluginState -> connect_port (duplexPluginState -> handle, 0, &delay_time);
//        duplexPluginState -> connect_port (duplexPluginState -> handle, 1, &dry_wet);
//        duplexPluginState -> connect_port (duplexPluginState -> handle, 2, (LADSPA_Data *) inputData);
//        duplexPluginState -> connect_port (duplexPluginState -> handle, 3, (LADSPA_Data *) outputData);

/*
        for (int i = 0 ; i < duplexPluginState -> number_of_ports ; i ++) {
            if (duplexPluginState -> input_port != -1)
                duplexPluginState -> connect_port (duplexPluginState -> handle, duplexPluginState -> input_port, (LADSPA_Data *) inputData);
            if (duplexPluginState -> output_port != -1)
                duplexPluginState -> connect_port (duplexPluginState -> handle, duplexPluginState -> output_port, (LADSPA_Data *) outputData);
//            if (LADSPA_IS_PORT_CONTROL(duplexPluginState -> portDescriptors [i]))
//                duplexPluginState -> connect_port (duplexPluginState -> handle, i, &duplexPluginState -> control_port_values [i]);
//            if (LADSPA_IS_PORT_INPUT(duplexPluginState->portDescriptors[i])) {
//                duplexPluginState->connect_port(duplexPluginState->handle, i,
//                                                (LADSPA_Data *) inputData);
//                LOGV("connected input port %d \n", i);
//            }
//            if (LADSPA_IS_PORT_OUTPUT(duplexPluginState->portDescriptors[i])) {
//                duplexPluginState->connect_port(duplexPluginState->handle, i,
//                                                (LADSPA_Data *) outputData);
//                LOGV("connected output port %d \n", i);
//            }
        }

        if (duplexPluginState->descriptor->activate)
            duplexPluginState->descriptor->activate(duplexPluginState->handle);
        duplexPluginState -> run (duplexPluginState -> handle, samplesToProcess);
*/
        // If there are fewer input samples then clear the rest of the buffer.
        int32_t samplesLeft = numOutputSamples - numInputSamples;
        for (int32_t i = 0; i < samplesLeft; i++) {
            *outputFloats++ = 0.0; // silence
        }

//        OUT ;
        return oboe::DataCallbackResult::Continue;
    }
};
#endif //SAMPLES_FULLDUPLEXPASS_H
