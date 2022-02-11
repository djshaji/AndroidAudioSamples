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

#include "FullDuplexStream.h"
#include "plugin.h"

class FullDuplexPass : public FullDuplexStream {
public:
    state_t * state ;
    virtual oboe::DataCallbackResult
    onBothStreamsReady(
            std::shared_ptr<oboe::AudioStream> inputStream,
            const void *inputData,
            int   numInputFrames,
            std::shared_ptr<oboe::AudioStream> outputStream,
            void *outputData,
            int   numOutputFrames) {
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
        LADSPA_Data dry_wet = 0.1 ;
        LADSPA_Data delay_time = 1.0 ;
//        if (state -> descriptor == NULL) {
//            LOGE("Plugin descriptor is null\n");
//        }

//        LADSPA_Data cutoff = 60 ;
        state -> descriptor -> connect_port (state -> handle, 0, &delay_time);
        state -> descriptor -> connect_port (state -> handle, 1, &dry_wet);
        state -> descriptor -> connect_port (state -> handle, 2, (LADSPA_Data *) inputData);
        state -> descriptor -> connect_port (state -> handle, 3, (LADSPA_Data *) outputData);
//
        state -> descriptor -> run (state -> handle, samplesToProcess);

        // If there are fewer input samples then clear the rest of the buffer.
        int32_t samplesLeft = numOutputSamples - numInputSamples;
        for (int32_t i = 0; i < samplesLeft; i++) {
            *outputFloats++ = 0.0; // silence
        }

        return oboe::DataCallbackResult::Continue;
    }
};
#endif //SAMPLES_FULLDUPLEXPASS_H
