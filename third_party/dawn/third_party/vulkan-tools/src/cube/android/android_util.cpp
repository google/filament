/*
 * Copyright (c) 2016 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Relicensed from the WTFPL (http://www.wtfpl.net/faq/).
 */

#include "android_util.h"
#include <android_native_app_glue.h>
#include <cassert>
#include <cstring>
#include <vector>
#include <string>
#include <sstream>
#include <stdlib.h>

extern "C" {

// Convert Intents to arg list, returning argc and argv
// Note that this C routine mallocs memory that the caller must free
char **get_args(struct android_app *app, const char *intent_extra_data_key, const char *appTag, int *count) {
    std::vector<std::string> args;
    JavaVM &vm = *app->activity->vm;
    JNIEnv *p_env;
    if (vm.AttachCurrentThread(&p_env, nullptr) != JNI_OK) return nullptr;

    JNIEnv &env = *p_env;
    jobject activity = app->activity->clazz;
    jmethodID get_intent_method = env.GetMethodID(env.GetObjectClass(activity), "getIntent", "()Landroid/content/Intent;");
    jobject intent = env.CallObjectMethod(activity, get_intent_method);
    jmethodID get_string_extra_method =
        env.GetMethodID(env.GetObjectClass(intent), "getStringExtra", "(Ljava/lang/String;)Ljava/lang/String;");
    jvalue get_string_extra_args;
    get_string_extra_args.l = env.NewStringUTF(intent_extra_data_key);
    jstring extra_str = static_cast<jstring>(env.CallObjectMethodA(intent, get_string_extra_method, &get_string_extra_args));

    std::string args_str;
    if (extra_str) {
        const char *extra_utf = env.GetStringUTFChars(extra_str, nullptr);
        args_str = extra_utf;
        env.ReleaseStringUTFChars(extra_str, extra_utf);
        env.DeleteLocalRef(extra_str);
    }

    env.DeleteLocalRef(get_string_extra_args.l);
    env.DeleteLocalRef(intent);
    vm.DetachCurrentThread();

    // split args_str
    std::stringstream ss(args_str);
    std::string arg;
    while (std::getline(ss, arg, ' ')) {
        if (!arg.empty()) args.push_back(arg);
    }

    // Convert our STL results to C friendly constructs
    assert(count != nullptr);
    *count = args.size() + 1;
    char **vector = (char **)malloc(*count * sizeof(char *));
    const char *appName = appTag ? appTag : (const char *)"appTag";

    vector[0] = (char *)malloc(strlen(appName) * sizeof(char));
    strcpy(vector[0], appName);

    for (uint32_t i = 0; i < args.size(); i++) {
        vector[i + 1] = (char *)malloc(strlen(args[i].c_str()) * sizeof(char));
        strcpy(vector[i + 1], args[i].c_str());
    }

    return vector;
}

}  // extern "C"
