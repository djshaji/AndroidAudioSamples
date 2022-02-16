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

#include <jni.h>
#include <android/log.h>
#include <logging_macros.h>
#include "dlfcn.h"
#include "LiveEffectEngine.h"
#include "ladspa.h"
#include "plugin.h"

static const int kOboeApiAAudio = 0;
static const int kOboeApiOpenSLES = 1;
#define MODULE "JNI Bridge"

static LiveEffectEngine *engine = nullptr;

extern "C" {
JNIEXPORT jboolean JNICALL
Java_com_google_oboe_samples_liveEffect_LiveEffectEngine_create(JNIEnv *env,
                                                               jclass) {
    if (engine == nullptr) {
        engine = new LiveEffectEngine();
        engine->pluginManager = new PluginManager (engine -> mSampleRate) ;
        engine -> pluginManager -> loadLibraries();
        engine ->pluginManager -> addPluginToRack(1, 0);
    }

    return (engine != nullptr) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_com_google_oboe_samples_liveEffect_LiveEffectEngine_delete(JNIEnv *env,
                                                               jclass) {
    if (engine) {
        engine->setEffectOn(false);
        delete engine;
        engine = nullptr;
    }
}

JNIEXPORT jboolean JNICALL
Java_com_google_oboe_samples_liveEffect_LiveEffectEngine_setEffectOn(
    JNIEnv *env, jclass, jboolean isEffectOn) {
    IN ;
    if (engine == nullptr) {
        LOGE(
            "Engine is null, you must call createEngine before calling this "
            "method");
        return JNI_FALSE;
    }

    OUT ;
    return engine->setEffectOn(isEffectOn) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_com_google_oboe_samples_liveEffect_LiveEffectEngine_setRecordingDeviceId(
    JNIEnv *env, jclass, jint deviceId) {
    if (engine == nullptr) {
        LOGE(
            "Engine is null, you must call createEngine before calling this "
            "method");
        return;
    }

    engine->setRecordingDeviceId(deviceId);
}

JNIEXPORT void JNICALL
Java_com_google_oboe_samples_liveEffect_LiveEffectEngine_setPlaybackDeviceId(
    JNIEnv *env, jclass, jint deviceId) {
    if (engine == nullptr) {
        LOGE(
            "Engine is null, you must call createEngine before calling this "
            "method");
        return;
    }

    engine->setPlaybackDeviceId(deviceId);
}

JNIEXPORT jboolean JNICALL
Java_com_google_oboe_samples_liveEffect_LiveEffectEngine_setAPI(JNIEnv *env,
                                                               jclass type,
                                                               jint apiType) {
    if (engine == nullptr) {
        LOGE(
            "Engine is null, you must call createEngine "
            "before calling this method");
        return JNI_FALSE;
    }

    oboe::AudioApi audioApi;
    switch (apiType) {
        case kOboeApiAAudio:
            audioApi = oboe::AudioApi::AAudio;
            break;
        case kOboeApiOpenSLES:
            audioApi = oboe::AudioApi::OpenSLES;
            break;
        default:
            LOGE("Unknown API selection to setAPI() %d", apiType);
            return JNI_FALSE;
    }

    return engine->setAudioApi(audioApi) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jboolean JNICALL
Java_com_google_oboe_samples_liveEffect_LiveEffectEngine_isAAudioRecommended(
    JNIEnv *env, jclass type) {
    if (engine == nullptr) {
        LOGE(
            "Engine is null, you must call createEngine "
            "before calling this method");
        return JNI_FALSE;
    }
    return engine->isAAudioRecommended() ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_com_google_oboe_samples_liveEffect_LiveEffectEngine_native_1setDefaultStreamValues(JNIEnv *env,
                                               jclass type,
                                               jint sampleRate,
                                               jint framesPerBurst) {
    oboe::DefaultStreamValues::SampleRate = (int32_t) sampleRate;
    oboe::DefaultStreamValues::FramesPerBurst = (int32_t) framesPerBurst;
}
} // extern "C"

extern "C"
JNIEXPORT void JNICALL
Java_com_google_oboe_samples_liveEffect_LiveEffectEngine_load_1plugin(JNIEnv *env, jclass clazz) {
//    state_t state ;
//    plugin_init(&state, "libamp.so", 0);
//    LOGD("Loaded plugin %s\n", state.descriptor->Name);
    // TODO: implement load_plugin()
}