/*
 * Copyright 2019 The Android Open Source Project
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
package com.android.angle.common;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.provider.Settings;
import android.text.TextUtils;
import android.util.Log;

import androidx.preference.PreferenceManager;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.IOException;
import java.io.InputStream;
import java.util.Collections;
import java.util.List;
import java.util.stream.Stream;

import com.android.angle.R;

public class Receiver extends BroadcastReceiver
{
    private static final String TAG = "AngleBroadcastReceiver";
    private static final boolean DEBUG = false;

    @Override
    public void onReceive(Context context, Intent intent)
    {
        final String action = intent.getAction();
        if (DEBUG)
        {
            Log.d(TAG, "Received intent: " + action);
        }

        if (action.equals(context.getString(R.string.intent_angle_for_android_toast_message)))
        {
            Bundle results = getResultExtras(true);
            results.putString(context.getString(R.string.intent_key_a4a_toast_message),
                    context.getString(R.string.angle_in_use_toast_message));
        }
        else if (action.equals(Intent.ACTION_BOOT_COMPLETED) || action.equals(Intent.ACTION_MY_PACKAGE_REPLACED))
        {
            AngleRuleHelper angleRuleHelper = new AngleRuleHelper(context);
            updateGlobalSettings(context, angleRuleHelper);
            updateDeveloperOptionsWatcher(context);
        }
    }

    /**
     * Consume the results of rule parsing to populate global settings
    */
    private static void updateGlobalSettings(Context context, AngleRuleHelper angleRuleHelper)
    {
        int count = 0;
        String packages = "";
        String choices = "";

        // Bring in the packages and choices and convert them to global settings format
        final List<String> anglePackages = angleRuleHelper.getPackageNamesForAngle();
        final List<String> nativePackages = angleRuleHelper.getPackageNamesForNative();

        // packages = anglePackage1,anglePackage2,nativePackage1,nativePackage2
        packages = String.join(",", Stream.concat(
                                    anglePackages.stream(),
                                    nativePackages.stream())
                                  .toList());

        // choices = angle,angle,native,native
        choices = String.join(",", Stream.concat(
                                    Collections.nCopies(
                                        anglePackages.size(), "angle")
                                    .stream(),
                                    Collections.nCopies(
                                        nativePackages.size(), "native")
                                    .stream())
                                 .toList());

        Log.v(TAG, "Updating ANGLE global settings with:" +
                " packages = " + packages +
                " choices = " + choices);
        GlobalSettings.writeGlobalSettings(context, packages, choices);
    }

    /**
     * When Developer Options are disabled, reset all of the global settings back to their defaults.
     */
    private static void updateDeveloperOptionsWatcher(Context context)
    {
        final Uri settingUri = Settings.Global.getUriFor(Settings.Global.DEVELOPMENT_SETTINGS_ENABLED);

        final ContentObserver developerOptionsObserver = new ContentObserver(new Handler()) {
            @Override
            public void onChange(boolean selfChange)
            {
                super.onChange(selfChange);

                final boolean developerOptionsEnabled =
                        (1
                                == Settings.Global.getInt(context.getContentResolver(),
                                        Settings.Global.DEVELOPMENT_SETTINGS_ENABLED, 0));

                Log.v(TAG, "Developer Options enabled value changed: "
                                   + "developerOptionsEnabled = " + developerOptionsEnabled);

                if (!developerOptionsEnabled)
                {
                    // Reset the necessary settings to their defaults.
                    SharedPreferences.Editor editor =
                            PreferenceManager.getDefaultSharedPreferences(context).edit();
                    editor.clear();
                    editor.apply();
                    GlobalSettings.clearGlobalSettings(context);
                }
            }
        };

        context.getContentResolver().registerContentObserver(
                settingUri, false, developerOptionsObserver);
    }
}
