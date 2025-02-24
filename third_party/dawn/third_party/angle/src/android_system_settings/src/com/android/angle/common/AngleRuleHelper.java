/*
 * Copyright 2023 The Android Open Source Project
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
import java.util.ArrayList;
import java.util.List;

/**
 * Helper that parses ANGLE rule JSON file and bookkeep the information.
 *
 * The format of the ANGLE rule JSON file as shown below.
 *
 * {
 *     "rules":[
 *         {
 *             "description": "Applications in this list will use ANGLE",
 *             "choice": "angle",
 *             "apps": [
 *                 {
 *                     "packageName": "com.android.example.a"
 *                 },
 *                 {
 *                     "packageName": "com.android.example.b"
 *                 }
 *             ]
 *         },
 *         {
 *             "description": "Applications in this list will not use ANGLE",
 *             "choice": "native",
 *             "apps":[
 *                 {
 *                     "packageName": "com.android.example.c"
 *                 }
 *             ]
 *         }
 *     ]
 * }
 */
public class AngleRuleHelper
{

    private static final String TAG = "AngleRuleHelper";
    private static final boolean DEBUG = false;
    private static final String ANGLE_RULES_FILE = "a4a_rules.json";

    private static final String ANGLE_JSON_RULES_KEY = "rules";
    private static final String ANGLE_JSON_DESCRIPTION_KEY = "description";
    private static final String ANGLE_JSON_CHOICE_KEY = "choice";
    private static final String ANGLE_JSON_APPS_KEY = "apps";
    private static final String ANGLE_JSON_PACKAGE_NAME_KEY = "packageName";

    private List<String> mPackageNamesForNative = new ArrayList<String>();
    private List<String> mPackageNamesForAngle = new ArrayList<String>();

    public AngleRuleHelper(Context context)
    {
        loadRules(context);
        storeRules(context);
    }

    List<String> getPackageNamesForNative()
    {
        return mPackageNamesForNative;
    }

    List<String> getPackageNamesForAngle()
    {
        return mPackageNamesForAngle;
    }

    private void storeRules(Context context)
    {
        final SharedPreferences sharedPreferences =
            PreferenceManager.getDefaultSharedPreferences(context);
        final SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.clear();
        for (String packageName : mPackageNamesForAngle) {
            editor.putString(packageName, GlobalSettings.DRIVER_SELECTION_ANGLE);
        }
        for (String packageName: mPackageNamesForNative) {
            editor.putString(packageName, GlobalSettings.DRIVER_SELECTION_NATIVE);
        }
        editor.apply();
    }

    private void loadRules(Context context)
    {
        String jsonStr = null;

        try
        {
            InputStream rulesStream = context.getAssets().open(ANGLE_RULES_FILE);
            int size = rulesStream.available();
            byte[] buffer = new byte[size];
            rulesStream.read(buffer);
            rulesStream.close();
            jsonStr = new String(buffer, "UTF-8");
        }
        catch (IOException ioe)
        {
            Log.e(TAG, "Failed to open " + ANGLE_RULES_FILE + ": ", ioe);
            return;
        }

        if (TextUtils.isEmpty(jsonStr))
        {
            Log.v(TAG, "Empty ANGLE rule in: " + ANGLE_RULES_FILE);
            return;
        }

        parseRules(jsonStr);
    }

    private void parseRules(String rulesJSON)
    {
        try
        {
            final JSONObject jsonObj = new JSONObject(rulesJSON);
            final JSONArray rules    = jsonObj.getJSONArray(ANGLE_JSON_RULES_KEY);
            if (rules == null)
            {
                Log.e(TAG, "No ANGLE Rules in " + ANGLE_RULES_FILE);
                return;
            }

            for (int i = 0; i < rules.length(); i++)
            {
                final JSONObject rule = rules.getJSONObject(i);
                Log.v (TAG, "Rule description: " + rule.optString(ANGLE_JSON_DESCRIPTION_KEY));

                final String choice = rule.optString(ANGLE_JSON_CHOICE_KEY);
                if (TextUtils.isEmpty(choice)
                        || (!choice.equals("native") && !choice.equals("angle")))
                {
                    Log.v(TAG, "Skipping rule entry due to unknown driver choice.");
                    continue;
                }

                final JSONArray apps  = rule.optJSONArray(ANGLE_JSON_APPS_KEY);
                if (apps == null || apps.length() == 0)
                {
                    Log.v(TAG, "Skipping rule entry with no apps");
                    continue;
                }

                if (choice.equals("native"))
                {
                    mPackageNamesForNative = parsePackageNames(apps);
                }
                else if (choice.equals("angle"))
                {
                    mPackageNamesForAngle = parsePackageNames(apps);
                }
            }
        }
        catch (JSONException je)
        {
            Log.e(TAG, "Error when parsing angle JSON: ", je);
        }
    }

    private List<String> parsePackageNames(JSONArray apps)
    {
        List<String> packageNames = new ArrayList<>();
        for (int j = 0; j < apps.length(); j++)
        {
            final JSONObject app = apps.optJSONObject(j);
            final String packageName = app.optString(ANGLE_JSON_PACKAGE_NAME_KEY);
            if (DEBUG)
            {
                Log.d(TAG, "parsed package name: " + packageNames);
            }
            if (TextUtils.isEmpty(packageName))
            {
                Log.v(TAG, "Skipping empty package name.");
                continue;
            }
            packageNames.add(packageName);
        }
        return packageNames;
    }
}
