// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// StandaloneBenchmarkActivity:
//   A {@link android.app.NativeActivity} for running angle gtests.

package com.android.angle.test;

import android.app.NativeActivity;
import android.content.Intent;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.WindowManager;

import org.chromium.build.NativeLibraries;
import org.chromium.build.gtest_apk.NativeTestIntent;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;

public class StandaloneBenchmarkActivity extends NativeActivity
{

    private static final String TAG = "BenchmarkNativeTest";

    private static final String STDOUT_FILE           = "stdout.txt";
    private static final String FRAME_TIME_FILE       = "traces_fps.txt";
    private static final String TRACE_LIST_FILE       = "trace_list.json";
    private static final String IMAGE_COMPARISON_FILE = "traces_img_comp.txt";

    private String[] traceNames       = {};
    private String outDataDir         = "";
    private String externalStorageDir = "";
    private String traceListDir       = "";
    private String commonTraceDir     = "";
    private AngleNativeTest mTest     = new AngleNativeTest();

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        // For NativeActivity based tests,
        // dependency libraries must be loaded before NativeActivity::OnCreate,
        // otherwise loading android.app.lib_name will fail
        for (String library : NativeLibraries.LIBRARIES)
        {
            Log.i(TAG, "loading: " + library);
            System.loadLibrary(library);
            Log.i(TAG, "loaded: " + library);
        }
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        setupDirectories();
        clearPrevResults();
        setupTraceInfo();

        Intent myIntent    = this.getIntent();
        String gtestFilter = "";

        for (String trace : this.traceNames)
        {
            gtestFilter += "TraceTest." + trace + ":";
        }
        myIntent.putExtra(NativeTestIntent.EXTRA_GTEST_FILTER, gtestFilter);
        myIntent.putExtra(NativeTestIntent.EXTRA_STDOUT_FILE, this.outDataDir + STDOUT_FILE);

        mTest.postCreate(this);
    }

    @Override
    public void onStart()
    {
        super.onStart();
        mTest.postStart(this);
    }

    @Override
    public void onStop()
    {
        super.onStop();

        for (String trace : this.traceNames)
        {
            String goldenImagePath  = this.commonTraceDir + trace + "/" + trace + "_golden.png";
            String currentImagePath = this.outDataDir + "angle_vulkan_" + trace + ".png";

            File goldenImageFile  = new File(goldenImagePath);
            File currentImageFile = new File(currentImagePath);

            if (goldenImageFile.exists() && currentImageFile.exists())
            {
                Bitmap golden  = BitmapFactory.decodeFile(goldenImagePath);
                Bitmap current = BitmapFactory.decodeFile(currentImagePath);
                compareImages(golden, current, trace);
            }
        }
    }

    // Method to set up trace information from assets folder to application data folder
    // Moves only .json, .angledata.gz, _golden.png in the respective trace folder
    private void setupTraceInfo()
    {
        // Get the AssetManager to access files in the assets folder
        AssetManager assetManager = getAssets();
        String[] files            = null;

        try
        {
            // Get a list of all files in the assets folder
            files = assetManager.list("");
        }
        catch (IOException e)
        {
            Log.e(TAG, "Failed to get asset file list.", e);
        }

        // Define the file suffixes to look for
        List<String> suffixes = List.of(".json", ".angledata.gz", "_golden.png");
        // Create a list to store the names of the trace names
        List<String> traceList = new ArrayList<>();

        for (String fileName : files)
        {
            InputStream in   = null;
            OutputStream out = null;
            String traceName;
            try
            {
                for (String suffix : suffixes)
                {
                    // Open the file from the assets folder
                    in = assetManager.open(fileName);
                    // If the file is the trace list file
                    // then we need to copy it in a different dir
                    if (fileName.equals(TRACE_LIST_FILE))
                    {
                        // Create a new file to store the trace list file
                        File outFile = new File(this.traceListDir, fileName);
                        if (!outFile.exists())
                        {
                            // Copy the trace list file to the new file
                            out = new FileOutputStream(outFile);
                            copyFile(in, out);
                            out.flush();
                            out.close();
                            out = null;
                        }
                        in.close();
                        in = null;
                        break;
                    }
                    // If the file is a trace data file
                    else if (fileName.endsWith(suffix))
                    {
                        // Extract the trace name from the file name
                        traceName = fileName.substring(0, fileName.length() - suffix.length());
                        // Add the trace name if not already in the list
                        if (!traceList.contains(traceName))
                        {
                            traceList.add(traceName);
                            this.traceNames = traceList.toArray(new String[0]);
                        }
                        // Create a directory to store the trace file
                        File dir = new File(this.commonTraceDir, traceName);
                        if (!dir.exists())
                        {
                            dir.mkdirs();
                        }
                        // Create a new file to store the trace file
                        File outFile = new File(dir, fileName);
                        if (!outFile.exists())
                        {
                            // Copy the trace file to the new file
                            out = new FileOutputStream(outFile);
                            copyFile(in, out);
                            out.flush();
                            out.close();
                            out = null;
                        }
                        in.close();
                        in = null;
                        break;
                    }
                }
            }
            catch (IOException e)
            {
                Log.i(TAG, "Failed to copy a file. Might not be fatal.");
            }
        }
    }

    // Method to copy the data from one file to another
    private void copyFile(InputStream in, OutputStream out) throws IOException
    {
        byte[] buffer = new byte[1024];
        int read;
        while ((read = in.read(buffer)) != -1)
        {
            out.write(buffer, 0, read);
        }
    }

    // Method to clear any previous data to avoid the data corruption
    private void clearPrevResults()
    {
        String[] filesPath = {this.outDataDir + STDOUT_FILE,
                this.externalStorageDir + FRAME_TIME_FILE,
                this.externalStorageDir + IMAGE_COMPARISON_FILE};

        for (String filePath : filesPath)
        {
            File file = new File(filePath);
            if (file.exists())
            {
                file.delete();
            }
        }
    }

    // Method to setup all the required directories for the app run.
    private void setupDirectories()
    {
        this.outDataDir   = getApplicationInfo().dataDir + "/files/";
        this.traceListDir = getApplicationInfo().dataDir + "/chromium_tests_root/gen/";
        this.commonTraceDir =
                getApplicationInfo().dataDir + "/chromium_tests_root/src/tests/restricted_traces/";
        this.externalStorageDir =
                Environment.getExternalStorageDirectory() + "/chromium_tests_root/";

        String[] dirsPath = {
                this.outDataDir, this.traceListDir, this.commonTraceDir, this.externalStorageDir};

        for (String dirPath : dirsPath)
        {
            File dir = new File(dirPath);
            if (!dir.exists())
            {
                dir.mkdirs();
            }
        }
    }

    // Method to compare current screenshot to provided golden images
    // It compares each component of the images pixel by pixel and stores the difference
    // It also calculates the histogram and dumps it into a file
    private void compareImages(Bitmap G, Bitmap C, String traceName)
    {
        int width  = G.getWidth();
        int height = G.getHeight();

        int[] pixelsGolden  = new int[width * height];
        int[] pixelsCurrent = new int[width * height];

        G.getPixels(pixelsGolden, 0, width, 0, 0, width, height);
        C.getPixels(pixelsCurrent, 0, width, 0, 0, width, height);

        int[] differences = new int[pixelsGolden.length];

        for (int i = 0; i < pixelsGolden.length; i++)
        {
            int p1 = pixelsGolden[i];
            int p2 = pixelsCurrent[i];

            int r1 = Color.red(p1);
            int g1 = Color.green(p1);
            int b1 = Color.blue(p1);

            int r2 = Color.red(p2);
            int g2 = Color.green(p2);
            int b2 = Color.blue(p2);

            differences[i] = Math.abs(r1 - r2) + Math.abs(g1 - g2) + Math.abs(b1 - b2);
        }

        // Calculate the histogram
        int[] histogram = calculateHistogram(differences);

        // Write the histogram to a file
        writeHistogramToFile(histogram, traceName);
    }

    // Method to calculate histogram
    private static int[] calculateHistogram(int[] differences)
    {
        int[] histogram = new int[6]; // 6 categories
        for (int diff : differences)
        {
            if (diff >= 0 && diff <= 20)
            {
                histogram[0]++;
            }
            else if (diff > 20 && diff <= 40)
            {
                histogram[1]++;
            }
            else if (diff > 40 && diff <= 70)
            {
                histogram[2]++;
            }
            else if (diff > 70 && diff <= 100)
            {
                histogram[3]++;
            }
            else if (diff > 100 && diff <= 150)
            {
                histogram[4]++;
            }
            else if (diff > 150 && diff <= 255)
            {
                histogram[5]++;
            }
        }
        return histogram;
    }

    // Method to write the histogram data to a file
    // Dumps the number of pixels having diff values in following 6 categories
    // 0-20, 20-40, 40-70, 70-100, 100-150, 150-255
    private void writeHistogramToFile(int[] histogram, String trace)
    {
        try
        {
            OutputStream out =
                    new FileOutputStream(this.externalStorageDir + IMAGE_COMPARISON_FILE, true);

            String categories =
                    "Diff in pixel value:\t0-20, 20-40, 40-70, 70-100, 100-150, 150-255\n";
            String traceName   = trace + ":\n";
            String newLine     = "\n\n";
            String numOfPixels = "Number of pixels:\t";

            out.write(traceName.getBytes(StandardCharsets.UTF_8));
            out.write(categories.getBytes(StandardCharsets.UTF_8));
            out.write(numOfPixels.getBytes(StandardCharsets.UTF_8));

            for (int i = 0; i < histogram.length; i++)
            {
                String line = histogram[i] + ", ";
                out.write(line.getBytes(StandardCharsets.UTF_8));
            }

            out.write(newLine.getBytes(StandardCharsets.UTF_8));
            out.close();
        }
        catch (IOException e)
        {
            Log.e(TAG, "Failed to write image comparison to the file", e);
        }
    }
}
