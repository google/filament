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

import android.content.ContentResolver;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.PackageInfo;
import android.provider.Settings;
import android.util.Log;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import com.android.angle.R;

class GlobalSettings
{
    public static final String DRIVER_SELECTION_ANGLE = "angle";
    public static final String DRIVER_SELECTION_DEFAULT = "default";
    public static final String DRIVER_SELECTION_NATIVE = "native";

    private static final String TAG = "ANGLEGlobalSettings";
    private static final boolean DEBUG = false;

    private static final String SHOW_ANGLE_IN_USE_DIALOG_BOX = "show_angle_in_use_dialog_box";
    private static final String DRIVER_SELECTION_PACKAGES = "angle_gl_driver_selection_pkgs";
    private static final String DRIVER_SELECTION_VALUES = "angle_gl_driver_selection_values";
    private static final String ANGLE_DEBUG_PACKAGE = "angle_debug_package";

    private Context mContext;
    private SharedPreferences mSharedPreferences;
    private List<String> mDriverSelectionPackages = new ArrayList<>();
    private List<String> mDriverSelectionValues = new ArrayList<>();

    GlobalSettings(Context context, SharedPreferences sharedPreferences,
            List<PackageInfo> installedPackages)
    {
        mContext = context;
        mSharedPreferences = sharedPreferences;

        initGlobalSettings(installedPackages);
    }

    void initGlobalSettings(List<PackageInfo> installedPackages)
    {
        mDriverSelectionPackages.clear();
        mDriverSelectionValues.clear();

        for (PackageInfo packageInfo : installedPackages)
        {
            final String packageName = packageInfo.packageName;
            final String driverSelectionValue = mSharedPreferences.getString(packageName, DRIVER_SELECTION_DEFAULT);
            if (driverSelectionValue.equals(DRIVER_SELECTION_DEFAULT))
            {
                continue;
            }
            mDriverSelectionPackages.add(packageName);
            mDriverSelectionValues.add(driverSelectionValue);
        }
        if (!mDriverSelectionPackages.isEmpty())
        {
            writeGlobalSettings();
        }
    }

    static void clearGlobalSettings(Context context)
    {
        // show_angle_in_use_dialog_box
        updateShowAngleInUse(context, false);

        // clear angle_gl_driver_selection_pkgs, angle_gl_driver_selection_values
        ContentResolver contentResolver = context.getContentResolver();
        Settings.Global.putString(contentResolver, DRIVER_SELECTION_PACKAGES, "");
        Settings.Global.putString(contentResolver, DRIVER_SELECTION_VALUES, "");

        // For completeness, we'll clear the angle_debug_package, but we don't allow setting
        // it via the UI
        Settings.Global.putString(contentResolver, ANGLE_DEBUG_PACKAGE, "");
    }

    static void updateShowAngleInUse(Context context, Boolean showAngleInUse)
    {
        if (DEBUG)
        {
            Log.v(TAG, "Show angle in use: " + showAngleInUse);
        }
        ContentResolver contentResolver = context.getContentResolver();
        Settings.Global.putInt(contentResolver, SHOW_ANGLE_IN_USE_DIALOG_BOX, 
                showAngleInUse ? 1 : 0);
    }

    boolean getShowAngleInUse()
    {
        return Settings.Global.getInt(mContext.getContentResolver(),
                SHOW_ANGLE_IN_USE_DIALOG_BOX, 0) == 1;
    }

    void updatePackageDriverSelection(String packageName, String driverSelectionValue)
    {
        if (!isValidDriverSelectionValue(driverSelectionValue))
        {
            Log.v(TAG, "Attempting to update " + packageName
                    + " with an invalid driver selection value: '" + driverSelectionValue + "'");
            return;
        }

        updateSharedPreferences(packageName, driverSelectionValue);
        updatePackageDriverSelectionInternal(packageName, driverSelectionValue);
        writeGlobalSettings();
    }

    private void updateSharedPreferences(String packageName, String driverSelectionValue)
    {
        final SharedPreferences.Editor editor = mSharedPreferences.edit();
        if (driverSelectionValue.equals(DRIVER_SELECTION_DEFAULT))
        {
            editor.remove(packageName);
            editor.apply();
            return;
        }
        editor.putString(packageName, driverSelectionValue);
        editor.apply();
    }

    private void updatePackageDriverSelectionInternal(String packageName, String driverSelectionValue)
    {
        final int packageIndex = getPackageIndex(packageName);
        if (packageIndex < 0)
        {
            if (driverSelectionValue.equals(DRIVER_SELECTION_DEFAULT))
            {
                return;
            }
            mDriverSelectionPackages.add(packageName);
            mDriverSelectionValues.add(driverSelectionValue);
            return;
        }
        if (driverSelectionValue.equals(DRIVER_SELECTION_DEFAULT))
        {
            mDriverSelectionPackages.remove(packageIndex);
            mDriverSelectionValues.remove(packageIndex);
            return;
        }
        mDriverSelectionValues.set(packageIndex, driverSelectionValue);
    }

    String getDriverSelectionValue(String packageName)
    {
        final int packageIndex = getPackageIndex(packageName);

        return packageIndex >= 0
                ? mDriverSelectionValues.get(packageIndex) : DRIVER_SELECTION_DEFAULT;
    }

    private void writeGlobalSettings()
    {
        final String driverSelectionPackages   = String.join(",", mDriverSelectionPackages);
        final String driverSelectionValues = String.join(",", mDriverSelectionValues);

        final ContentResolver contentResolver = mContext.getContentResolver();
        Settings.Global.putString(contentResolver,
                DRIVER_SELECTION_PACKAGES, driverSelectionPackages);
        Settings.Global.putString(contentResolver, DRIVER_SELECTION_VALUES, driverSelectionValues);
    }

    static void writeGlobalSettings(Context context, String driverSelectionPackages,
            String driverSelectionValues)
    {
        final ContentResolver contentResolver = context.getContentResolver();
        Settings.Global.putString(contentResolver,
                DRIVER_SELECTION_PACKAGES, driverSelectionPackages);
        Settings.Global.putString(contentResolver, DRIVER_SELECTION_VALUES, driverSelectionValues);
    }

    private int getPackageIndex(String packageName)
    {
        for (int i = 0; i < mDriverSelectionPackages.size(); i++)
        {
            if (mDriverSelectionPackages.get(i).equals(packageName))
            {
                return i;
            }
        }

        return -1;
    }

    private boolean isValidDriverSelectionValue(String driverSelectionValue)
    {
        CharSequence[] drivers = mContext.getResources().getStringArray(R.array.driver_values);

        for (CharSequence driver : drivers)
        {
            if (driverSelectionValue.equals(driver.toString()))
            {
                return true;
            }
        }

        return false;
    }
}
