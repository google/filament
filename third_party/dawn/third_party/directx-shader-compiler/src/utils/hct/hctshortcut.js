///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// hctshortcut.js                                                            //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//
// Use this script to create a shortcut on your desktop that will set up the
// right console environment.
//

var shell = WScript.CreateObject("WScript.Shell");
var linkName = "HLSL console";
var hctPath = WScript.ScriptFullName;
hctPath = hctPath.substr(0, hctPath.lastIndexOf("\\"));

var hctStartPath = hctPath + "\\hctstart.cmd";

var srcPath = hctPath; // root\utils\hct
srcPath = srcPath.substr(0, srcPath.lastIndexOf("\\")); // root\utils
srcPath = srcPath.substr(0, srcPath.lastIndexOf("\\")); // root

var binPath = srcPath; // somewhere\root
binPath = srcPath.substr(0, srcPath.lastIndexOf("\\")); // somewhere\
binPath = binPath + "\\hlsl.bin";

var desktopPath = shell.SpecialFolders("Desktop");
var shortcut = shell.CreateShortcut(desktopPath + "\\" + linkName + ".lnk");
shortcut.TargetPath = shell.ExpandEnvironmentStrings("%windir%\\System32\\cmd.exe");
shortcut.Arguments = "/k " + hctStartPath + " " + srcPath + " " + binPath;
shortcut.Save();

WScript.Echo("Shortcut '" + linkName + "' created on desktop.");
