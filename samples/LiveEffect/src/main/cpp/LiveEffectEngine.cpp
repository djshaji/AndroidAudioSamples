/**
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

#include <cassert>
#include <logging_macros.h>

#include "LiveEffectEngine.h"

LiveEffectEngine::LiveEffectEngine() {
    IN ;
    assert(mOutputChannelCount == mInputChannelCount);

//    pluginState = loadPlugin();
//    LOGD("%s loadPlugin [ok]: %s %s\n", __PRETTY_FUNCTION__ , pluginState -> descriptor -> Label, pluginState -> descriptor -> Name);

    OUT ;
}

void LiveEffectEngine::setRecordingDeviceId(int32_t deviceId) {
    mRecordingDeviceId = deviceId;
}

void LiveEffectEngine::setPlaybackDeviceId(int32_t deviceId) {
    mPlaybackDeviceId = deviceId;
}

bool LiveEffectEngine::isAAudioRecommended() {
    return oboe::AudioStreamBuilder::isAAudioRecommended();
}

bool LiveEffectEngine::setAudioApi(oboe::AudioApi api) {
    if (mIsEffectOn) return false;
    mAudioApi = api;
    return true;
}

bool LiveEffectEngine::setEffectOn(bool isOn) {
    IN ;
    bool success = true;
    if (isOn != mIsEffectOn) {
        if (isOn) {
            success = openStreams() == oboe::Result::OK;
            if (success) {
                mFullDuplexPass.start();
                mIsEffectOn = isOn;
            }
        } else {
            mFullDuplexPass.stop();
            closeStreams();
            mIsEffectOn = isOn;
       }
    }

//    LOGV("plugin here: %s\n", pluginState -> descriptor -> Name);
    OUT ;
    return success;
}

state_t * LiveEffectEngine::loadPlugin() {
    IN ;
    //! TODO: Free this memory!
//    ptr = static_cast<state_t *>(malloc(sizeof(state_t)));
    state_t  * ptr = static_cast<state_t *>(malloc(sizeof(state_t)));
    ptr -> sample_rate = mSampleRate ; // am I devops or what
//    if (plugin_init(&ptr, "libnoise.so", 0))
    if (!plugin_init(ptr, "libsine.so", 0))
        LOGD("Loaded plugin %s\n", ptr -> descriptor->Name);

    OUT ;
    return ptr ;
}

void LiveEffectEngine::closeStreams() {
    IN ;
    /*
    * Note: The order of events is important here.
    * The playback stream must be closed before the recording stream. If the
    * recording stream were to be closed first the playback stream's
    * callback may attempt to read from the recording stream
    * which would cause the app to crash since the recording stream would be
    * null.
    */
    closeStream(mPlayStream);
    mFullDuplexPass.setOutputStream(nullptr);

    closeStream(mRecordingStream);
    mFullDuplexPass.setInputStream(nullptr);
}

oboe::Result  LiveEffectEngine::openStreams() {
    IN ;
    // Note: The order of stream creation is important. We create the playback
    // stream first, then use properties from the playback stream
    // (e.g. sample rate) to create the recording stream. By matching the
    // properties we should get the lowest latency path
    oboe::AudioStreamBuilder inBuilder, outBuilder;
    setupPlaybackStreamParameters(&outBuilder);
    oboe::Result result = outBuilder.openStream(mPlayStream);
    if (result != oboe::Result::OK) {
        LOGE("Failed to open output stream. Error %s", oboe::convertToText(result));
        mSampleRate = oboe::kUnspecified;
        OUT ;
        return result;
    } else {
        // The input stream needs to run at the same sample rate as the output.
        mSampleRate = mPlayStream->getSampleRate();
    }
    warnIfNotLowLatency(mPlayStream);

    setupRecordingStreamParameters(&inBuilder, mSampleRate);
    result = inBuilder.openStream(mRecordingStream);
    if (result != oboe::Result::OK) {
        LOGE("Failed to open input stream. Error %s", oboe::convertToText(result));
        closeStream(mPlayStream);
        OUT ;
        return result;
    }
    warnIfNotLowLatency(mRecordingStream);

    mFullDuplexPass.setInputStream(mRecordingStream);
    mFullDuplexPass.setOutputStream(mPlayStream);

    // Load LADSPA Plugin here

    /*
    LOGV("Checking if plugin state is null ...\n");
    if (mFullDuplexPass.duplexPluginState == NULL) {
        LOGV("Assinging plugin\n");
        mFullDuplexPass .duplexPluginState = loadPlugin();
        pluginState = mFullDuplexPass.duplexPluginState;
    } else {
        LOGV("It isn't ... already loaded %s\n", mFullDuplexPass.duplexPluginState -> descriptor -> Name);
    }

    LOGD("%s loadPlugin [ok]: %s %s\n", __PRETTY_FUNCTION__ , mFullDuplexPass .get_plugin() -> descriptor -> Label, mFullDuplexPass .get_plugin() -> descriptor -> Name);
     */
    OUT ;
    return result;
}

/**
 * Sets the stream parameters which are specific to recording,
 * including the sample rate which is determined from the
 * playback stream.
 *
 * @param builder The recording stream builder
 * @param sampleRate The desired sample rate of the recording stream
 */
oboe::AudioStreamBuilder *LiveEffectEngine::setupRecordingStreamParameters(
    oboe::AudioStreamBuilder *builder, int32_t sampleRate) {
    // This sample uses blocking read() because we don't specify a callback
    builder->setDeviceId(mRecordingDeviceId)
        ->setDirection(oboe::Direction::Input)
        ->setSampleRate(sampleRate)
        ->setChannelCount(mInputChannelCount);
    return setupCommonStreamParameters(builder);
}

/**
 * Sets the stream parameters which are specific to playback, including device
 * id and the dataCallback function, which must be set for low latency
 * playback.
 * @param builder The playback stream builder
 */
oboe::AudioStreamBuilder *LiveEffectEngine::setupPlaybackStreamParameters(
    oboe::AudioStreamBuilder *builder) {
    builder->setDataCallback(this)
        ->setErrorCallback(this)
        ->setDeviceId(mPlaybackDeviceId)
        ->setDirection(oboe::Direction::Output)
        ->setChannelCount(mOutputChannelCount);

    return setupCommonStreamParameters(builder);
}

/**
 * Set the stream parameters which are common to both recording and playback
 * streams.
 * @param builder The playback or recording stream builder
 */
oboe::AudioStreamBuilder *LiveEffectEngine::setupCommonStreamParameters(
    oboe::AudioStreamBuilder *builder) {
    // We request EXCLUSIVE mode since this will give us the lowest possible
    // latency.
    // If EXCLUSIVE mode isn't available the builder will fall back to SHARED
    // mode.
    builder->setAudioApi(mAudioApi)
        ->setFormat(mFormat)
        ->setFormatConversionAllowed(true)
        ->setSharingMode(oboe::SharingMode::Exclusive)
        ->setPerformanceMode(oboe::PerformanceMode::LowLatency);
    return builder;
}

/**
 * Close the stream. AudioStream::close() is a blocking call so
 * the application does not need to add synchronization between
 * onAudioReady() function and the thread calling close().
 * [the closing thread is the UI thread in this sample].
 * @param stream the stream to close
 */
void LiveEffectEngine::closeStream(std::shared_ptr<oboe::AudioStream> &stream) {
    IN ;
    if (stream) {
        oboe::Result result = stream->stop();
        if (result != oboe::Result::OK) {
            LOGW("Error stopping stream: %s", oboe::convertToText(result));
        }
        result = stream->close();
        if (result != oboe::Result::OK) {
            LOGE("Error closing stream: %s", oboe::convertToText(result));
        } else {
            LOGW("Successfully closed streams");
        }
        stream.reset();
    }

    OUT ;
}

/**
 * Warn in logcat if non-low latency stream is created
 * @param stream: newly created stream
 *
 */
void LiveEffectEngine::warnIfNotLowLatency(std::shared_ptr<oboe::AudioStream> &stream) {
    if (stream->getPerformanceMode() != oboe::PerformanceMode::LowLatency) {
        LOGW(
            "Stream is NOT low latency."
            "Check your requested format, sample rate and channel count");
    }
}

/**
 * Handles playback stream's audio request. In this sample, we simply block-read
 * from the record stream for the required samples.
 *
 * @param oboeStream: the playback stream that requesting additional samples
 * @param audioData:  the buffer to load audio samples for playback stream
 * @param numFrames:  number of frames to load to audioData buffer
 * @return: DataCallbackResult::Continue.
 */
oboe::DataCallbackResult LiveEffectEngine::onAudioReady(
    oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) {
    return mFullDuplexPass.onAudioReady(oboeStream, audioData, numFrames);
}

/**
 * Oboe notifies the application for "about to close the stream".
 *
 * @param oboeStream: the stream to close
 * @param error: oboe's reason for closing the stream
 */
void LiveEffectEngine::onErrorBeforeClose(oboe::AudioStream *oboeStream,
                                          oboe::Result error) {
    LOGE("%s stream Error before close: %s",
         oboe::convertToText(oboeStream->getDirection()),
         oboe::convertToText(error));
}

/**
 * Oboe notifies application that "the stream is closed"
 *
 * @param oboeStream
 * @param error
 */
void LiveEffectEngine::onErrorAfterClose(oboe::AudioStream *oboeStream,
                                         oboe::Result error) {
    LOGE("%s stream Error after close: %s",
         oboe::convertToText(oboeStream->getDirection()),
         oboe::convertToText(error));
}

bool LiveEffectEngine::loadLibrary (std::string pluginfile) {
    IN ;
    SharedLibrary library = SharedLibrary (pluginfile);
    char * err = library . load() ;
    if (err == NULL) {
        LOGD("Loaded shared library %s\n", pluginfile.c_str());
        library.setSampleRate(mSampleRate);
        library.loadPlugins() ;
        LOGD("Loaded plugins for %s", pluginfile.c_str());
        libraries.push_front(library);
        OUT ;
       return true ;
    } else {
        LOGE("Failed to load shared library %s: %s", pluginfile.c_str(), err);
        OUT ;
        return false ;
    }
}

void LiveEffectEngine::loadLibraries () {
    IN ;
    // So I learnt this today
    // how cool is this: very
    for (std::string library : default_plugins) {
        loadLibrary(library);
        LOGV("loaded shared library: %s", library.c_str());
    }
    OUT ;
}

void LiveEffectEngine::addPluginToRack (SharedLibrary sharedLibrary, unsigned long index) {
    activePlugins.push_back(sharedLibrary.plugins.at (index));
}

void LiveEffectEngine::process (LADSPA_Data * inputData, LADSPA_Data * outputData, unsigned long samplesToProcess) {
    // How cool is this: very
    for (Plugin plugin: activePlugins) {
        if (plugin . inputPort != -1)
            plugin . descriptor -> connect_port (plugin . handle, plugin . inputPort, (LADSPA_Data *) inputData);
        if (plugin . outputPort != -1)
            plugin . descriptor -> connect_port (plugin . handle, plugin . outputPort, (LADSPA_Data *) outputData);
        plugin . descriptor -> run (plugin. handle, samplesToProcess);
    }
}