/*
 * Copyright (C) 2018 The Android Open Source Project
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

import static android.provider.SearchIndexablesContract.COLUMN_INDEX_RAW_INTENT_ACTION;
import static android.provider.SearchIndexablesContract.COLUMN_INDEX_RAW_INTENT_TARGET_CLASS;
import static android.provider.SearchIndexablesContract.COLUMN_INDEX_RAW_INTENT_TARGET_PACKAGE;
import static android.provider.SearchIndexablesContract.COLUMN_INDEX_RAW_KEY;
import static android.provider.SearchIndexablesContract.COLUMN_INDEX_RAW_KEYWORDS;
import static android.provider.SearchIndexablesContract.COLUMN_INDEX_RAW_SUMMARY_ON;
import static android.provider.SearchIndexablesContract.COLUMN_INDEX_RAW_TITLE;
import static android.provider.SearchIndexablesContract.INDEXABLES_RAW_COLUMNS;
import static android.provider.SearchIndexablesContract.NON_INDEXABLES_KEYS_COLUMNS;

import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.provider.SearchIndexablesProvider;
import android.provider.Settings;

import com.android.angle.R;

public class SearchProvider extends SearchIndexablesProvider
{

    private static final String TAG = "AngleSearchProvider";

    @Override
    public boolean onCreate()
    {
        return true;
    }

    @Override
    public Cursor queryXmlResources(String[] projection)
    {
        return null;
    }

    @Override
    public Cursor queryRawData(String[] projection)
    {
        MatrixCursor cursor = new MatrixCursor(INDEXABLES_RAW_COLUMNS);
        Context context     = getContext();

        Object[] ref                        = new Object[INDEXABLES_RAW_COLUMNS.length];
        ref[COLUMN_INDEX_RAW_KEY]           = context.getString(R.string.angle_preferences);
        ref[COLUMN_INDEX_RAW_TITLE]         = context.getString(R.string.angle_preferences);
        ref[COLUMN_INDEX_RAW_SUMMARY_ON]    = context.getString(R.string.angle_preferences_summary);
        ref[COLUMN_INDEX_RAW_KEYWORDS]      = context.getString(R.string.keywords);
        ref[COLUMN_INDEX_RAW_INTENT_ACTION] = Intent.ACTION_MAIN;
        ref[COLUMN_INDEX_RAW_INTENT_TARGET_PACKAGE] = getContext().getApplicationInfo().packageName;
        ref[COLUMN_INDEX_RAW_INTENT_TARGET_CLASS]   = "com.android.angle.MainActivity";

        cursor.addRow(ref);
        return cursor;
    }

    @Override
    public Cursor queryNonIndexableKeys(String[] projection)
    {
        boolean developerOptionsIsEnabled =
                Settings.Global.getInt(getContext().getContentResolver(),
                        Settings.Global.DEVELOPMENT_SETTINGS_ENABLED, 0)
                != 0;

        // If developer options is not enabled, ANGLE shouldn't be searchable.
        if (!developerOptionsIsEnabled)
        {
            MatrixCursor cursor = new MatrixCursor(NON_INDEXABLES_KEYS_COLUMNS);
            Object[] row        = new Object[] {getContext().getString(R.string.angle_preferences)};
            cursor.addRow(row);
            return cursor;
        }
        return null;
    }
}
