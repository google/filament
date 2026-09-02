///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// hctjs.js                                                                  //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//
// References:
//
// JScript Language Reference
// http://msdn2.microsoft.com/en-us/library/yek4tbz0
//
// Windows Script Host Object Model
// http://msdn2.microsoft.com/en-us/library/a74hyyw0
//
// Script Runtime
// http://msdn2.microsoft.com/en-us/library/hww8txat.aspx
//

// Include this file with:
// eval(new ActiveXObject("Scripting.FileSystemObject").OpenTextFile(new ActiveXObject("WScript.Shell").ExpandEnvironmentStrings("%HLSL_SRC_DIR%\\utils\\hct\\hctjs.js"), 1).ReadAll());

function ArrayAny(arr, callback) {
    /// <summary>Checks whether any element in an array satisfies a predicate.</summary>
    /// <param name="arr" type="Array">Array to operate on.</param>
    /// <param name="callback" type="Function">Function to test with element and index, returning true or false.</param>
    /// <returns type="Boolean">true if 'callback' returns true for any element; false otherwise.</returns>
    for (var i = 0; i < arr.length; i++) {
        if (callback(arr[i], i)) {
            return true;
        }
    }

    return false;
}

function ArrayDistinct(arr, comparer) {
    /// <summary>Returns each element in the array at-most-once.</summary>
    /// <param name="arr" type="Array">Array to operate on.</param>
    /// <param name="comparer" type="Function">Optional comparer.</param>
    /// <returns type="Array">Array of elements from arr, sorted as the comparer, with no repeating elements.</returns>

    var result = arr.slice(0);

    if (result.length === 0) {
        return result;
    }

    if (!comparer) {
        comparer = CompareValues;
    }

    result.sort(comparer);
    var readIndex = 1, lastWriteIndex = 0;
    while (readIndex < result.length) {
        if (comparer(result[readIndex], result[lastWriteIndex]) !== 0) {
            lastWriteIndex += 1;
            if (readIndex !== lastWriteIndex) {
                result[lastWriteIndex] = result[readIndex];
            }
        }
        readIndex += 1;
    }

    lastWriteIndex += 1;
    if (lastWriteIndex < result.length) {
        result = result.slice(0, lastWriteIndex);
    }

    return result;
}

function ArrayForEach(arr, callback) {
    /// <summary>Invokes a callback for each element in the array.</summary>
    /// <param name="arr" type="Array">Array to operate on.</param>
    /// <param name="callback" type="Function">Function to invoke with element and index.</param>
    for (var i = 0; i < arr.length; i++) {
        callback(arr[i], i);
    }
}

function ArrayIndexOf(arr, element) {
    /// <summary>Scans the specified array for the given element.</summary>
    /// <param name="arr" type="Array">Array to operate on.</param>
    /// <param name="element">Element to look for.</param>
    for (var i = 0; i < arr.length; i++) {
        if (arr[i] == element) {
            return i;
        }
    }
    return -1;
}

function ArrayMax(arr) {
    /// <summary>Gets the maximum value of the specified array.</summary>
    if (arr.length === 0) {
        return undefined;
    }

    var result = arr[0];
    for (var i = 1; i < arr.length; i++) {
        if (arr[i] > result) {
            result = arr[i];
        }
    }
    return result;
}

function ArraySelect(arr, selector) {
    /// <summary>Invokes a selector callback for each element in the array.</summary>
    /// <param name="arr" type="Array">Array to operate on.</param>
    /// <param name="callback" type="Function">Function to invoke with element and index.</param>
    /// <returns type="Array">Array of selections for the arr element.</returns>
    var result = [];
    for (var i = 0; i < arr.length; i++) {
        result.push(selector(arr[i], i));
    }
    return result;
}

function ArraySelectMany(arr, selector) {
    /// <summary>Invokes a selector callback for each element in the array to produce multiple results.</summary>
    /// <param name="arr" type="Array">Array to operate on.</param>
    /// <param name="callback" type="Function">Function to invoke with element and index.</param>
    /// <returns type="Array">Array of flattened selections for the arr element.</returns>
    var result = [];
    for (var i = 0; i < arr.length; i++) {
        var selectorArr = selector(arr[i], i);
        if (selectorArr) {
            for (var j = 0; j < selectorArr.length; j++) {
                result.push(selectorArr[j]);
            }
        }
    }
    return result;
}

function ArraySelectMember(arr, memberName) {
    /// <summary>Invokes a selector callback for each element in the array.</summary>
    /// <param name="arr" type="Array">Array to operate on.</param>
    /// <param name="memberName" type="String">Member name to select from each array element.</param>
    /// <returns type="Array">Array of members for the arr elements.</returns>
    var result = [];
    for (var i = 0; i < arr.length; i++) {
        result.push(arr[i][memberName]);
    }
    return result;
}

function ArraySort(arr, comparer) {
    /// <summary>Adds the values in the array.</summary>
    /// <param name="arr" type="Array">Array to operate on.</param>
    /// <returns type="Number">Sum of the arr elements.</returns>
    var result = arr.slice(0);
    result.sort(comparer);
    return result;
}

function ArraySum(arr) {
    /// <summary>Adds the values in the array.</summary>
    /// <param name="arr" type="Array">Array to operate on.</param>
    /// <returns type="Number">Sum of the arr elements.</returns>
    var result = 0;
    for (var i = 0; i < arr.length; i++) {
        if (arr[i]) {
            result += arr[i];
        }
    }
    return result;
}

function ArrayTake(arr, count) {
    /// <summary>Returns the first count elements (or the original array if it has less than count elements.</summary>
    if (arr.length < count) {
        return arr;
    }

    return arr.slice(0, count);
}

function ArrayWhere(arr, callback) {
    /// <summary>Returns the elements in an array that satisfy a predicate.</summary>
    /// <param name="arr" type="Array">Array to operate on.</param>
    /// <param name="callback" type="Function">Function to test with element and index, returning true or false.</param>
    /// <returns type="Array">Array of elements from arr that satisfy the predicate.</returns>

    var result = [];

    for (var i = 0; i < arr.length; i++) {
        if (callback(arr[i], i)) {
            result.push(arr[i]);
        }
    }
    return result;
}

function CheckScriptFlag(name) {
    /// <summary>Checks whether a script argument was given with true or false.</summary>
    /// <param name="name" type="String">Argument name to check.</param>
    /// <returns type="Boolean">
    /// true if the argument was given witha value of 'true' or 'True'; false otherwise.
    /// </returns>
    var flag = WScript.Arguments.Named(name);
    if (!flag) {
        return false;
    }

    return flag === "true" || flag === "True";
}

function CreateFolderIfMissing(path) {
    /// <summary>Creates a folder if it doesn't exist.</summary>
    /// <param name="path" type="String">Path to folder to create.</param>
    /// <remarks>This function will write out to the console on creation.</remarks>
    if (!path) return;
    var parent = PathGetDirectory(path);
    var fso = new ActiveXObject("Scripting.FileSystemObject");
    if (!fso.FolderExists(parent)) {
        CreateFolderIfMissing(parent);
    }
    if (!fso.FolderExists(path)) {
        WScript.Echo("Creating " + path + "...");
        fso.CreateFolder(path);
    }
}

function CompareValues(left, right) {
    if (left === right) return 0;
    if (left < right) return -1;
    return 1;
}

function CompareValuesAsStrings(left, right) {
    if (left === right) return 0;
    left = (left === null) ? "" : left.toString();
    right = (right === null) ? "" : right.toString();
    if (left === right) return 0;
    if (left < right) return -1;
    return 1;
}

function DbValueToString(value) {
    /// <summary>Returns a string representation for a database value.</summary>
    /// <param name="value">Value from database.</param>
    /// <returns type="String">A string representing the specified value.</returns>
    if (value === null || value === undefined) {
        return "NULL";
    }
    return value.toString();
}

function DeleteFile(path, force) {
    /// <summary>Deletes a file.</summary>
    /// <param name="path" type="String">Path to the file.</param>
    /// <param name="force" type="Boolean">Whether to delete the file even if it has the read-only attribute set.</param>

    var fso = new ActiveXObject("Scripting.FileSystemObject");
    fso.DeleteFile(path, force);
}

function DeleteFolder(path, force) {
    /// <summary>Deletes a folder.</summary>
    /// <param name="path" type="String">Path to the folder.</param>
    /// <param name="force" type="Boolean">Whether to delete the folder even if it has the read-only attribute set.</param>

    var fso = new ActiveXObject("Scripting.FileSystemObject");
    fso.DeleteFolder(path, force);
}

function CopyFolder(source, dest, overwrite) {
    /// <summary>Recursively copies a folder and its contents from source to dest.</summary>
    /// <param name="source" type="String">Path to the source folder location.</param>
    /// <param name="dest" type="String">Path to the destination folder location.</param>
    /// <param name="overwrite" type="Boolean">Whether to overwrite a folder in the destination location.</param>

    var fso = new ActiveXObject("Scripting.FileSystemObject");
    fso.CopyFolder(source, dest, overwrite);
}

function CopyFile(source, dest, overwrite) {
    /// <summary>Copies a file from source to dest.</summary>
    /// <param name="source" type="String">Path to the source file location.</param>
    /// <param name="dest" type="String">Path to the destination file location.</param>
    /// <param name="overwrite" type="Boolean">Whether to overwrite a file in the destination location.</param>

    var fso = new ActiveXObject("Scripting.FileSystemObject");

    if (overwrite && fso.FileExists(dest)) {
        var f = fso.getFile(dest);
        f.attributes = 0;
    }

    fso.CopyFile(source, dest, overwrite);
}

function ExpandEnvironmentVariables(name) {
    /// <summary>
    /// Replaces the name of each environment variable embedded in the specified string with the
    /// string equivalent of the value of the variable, then returns the resulting string.
    /// </summary>
    /// <param name="name" type="String">
    /// A string containing the names of zero or more environment variables. Each environment variable is quoted with the percent sign character (%).
    /// </param>
    /// <returns type="String">A string with each environment variable replaced by its value.</returns>
    var shell = new ActiveXObject("WScript.Shell");
    var result = shell.ExpandEnvironmentStrings(name);
    return result;
}

function ExtractContentsBetweenMarkers(path, contentOnly, isExclusion, startMarker, endMarker, callback) {
    /// <summary>
    /// Extracts the lines from the 'path' text file between the start and end markers.
    /// </summary>
    /// <param name="path" type="String">Path to file.</param>
    /// <param name="contentOnly" type="Boolean">
    /// true to skip everything until it's found between markers, false to start including everything from the start.
    /// </param>
    /// <param name="isExclusion" type="Boolean">
    /// false if the 'extraction' means keeping the content; true if it means not excluding it from the result.
    /// </param>
    /// <param name="startMarker" type="String">Line content to match for content start.</param>
    /// <param name="endMarker" type="String">Line content to match for content end.</param>
    /// <param name="callback" type="Function" mayBeNull="true">
    /// If true, then this function is called for every line along with the inContent flag
    /// before the line is added; the called function may return a line
    /// to be added in its place, null to skip processing.
    /// </param>
    /// <returns type="String">The string content of the file.</returns>

    var content = ReadAllTextFile(path);
    return ExtractContentsBetweenMarkersForText(content, contentOnly, isExclusion, startMarker, endMarker, callback);
}

function ExtractContentsBetweenMarkersForText(content, contentOnly, isExclusion, startMarker, endMarker, callback) {
    /// <summary>
    /// Extracts the lines from the specified text between the start and end markers.
    /// </summary>
    /// <param name="content" type="String">Text to process.</param>
    /// <param name="contentOnly" type="Boolean">
    /// true to skip everything until it's found between markers, false to start including everything from the start.
    /// </param>
    /// <param name="isExclusion" type="Boolean">
    /// false if the 'extraction' means keeping the content; true if it means not excluding it from the result.
    /// </param>
    /// <param name="startMarker" type="String">Line content to match for content start.</param>
    /// <param name="endMarker" type="String">Line content to match for content end.</param>
    /// <param name="callback" type="Function" mayBeNull="true">
    /// If true, then this function is called for every line along with the inContent flag
    /// before the line is added; the called function may return a line
    /// to be added in its place, null to skip processing.
    /// </param>
    /// <returns type="String">The extracted content.</returns>

    var inContent = contentOnly === false;
    var lines = StringSplit(content, "\r\n");
    var result = [];
    var i, len;
    for (i = 0, len = lines.length; i < len; i++) {
        var line = lines[i];
        var contentStartIndex = line.indexOf(startMarker);
        if (inContent === false && contentStartIndex !== -1) {
            inContent = true;
            continue;
        }

        var contentEndIndex = line.indexOf(endMarker);
        if (inContent === true && contentEndIndex !== -1) {
            inContent = false;
            continue;
        }

        if (callback) {
            var callbackResult = callback(line, inContent);
            if (callbackResult !== null && callbackResult !== undefined) {
                result.push(callbackResult);
            }
        } else {
            if (inContent && !isExclusion) {
                result.push(line);
            } else if (!inContent && isExclusion) {
                result.push(line);
            }
        }
    }

    return result.join("\r\n");
}

function FolderExists(path) {
    /// <summary>Checks whether the specified directory exists.</summary>
    var fso = new ActiveXObject("Scripting.FileSystemObject");
    if (fso.FolderExists(path)) {
        return true;
    }
    else {
        return false;
    }
}

function FileExists(path) {
    /// <summary>Checks whether the specified file exists.</summary>
    var fso = new ActiveXObject("Scripting.FileSystemObject");
    if (fso.FileExists(path)) {
        return true;
    }
    else {
        return false;
    }
}

function GetEnvironmentVariable(name) {
    /// <summary>Gets the value of the specified environment variable.</summary>
    /// <param name="name" type="String">Name of the variable value to get.</param>
    /// <returns type="String">Value for the given environment variable; null if undefined.</returns>
    var shell = new ActiveXObject("WScript.Shell");
    var result = shell.ExpandEnvironmentStrings("%" + name + "%");
    if (result == "%" + name + "%") {
        result = null;
    }

    return result;
}

function GetFilesRecursive(path) {
    /// <summary>Gets all file names under the specified directory path.</summary>
    /// <param name="path" type="String">Path to directory.</param>
    /// <returns type="Array">Array of all file names under path.</returns>

    var result = [];
    var fso = new ActiveXObject("Scripting.FileSystemObject");
    var pending = [path];

    while (pending.length) {
        var item = pending.pop();
        var folder = fso.GetFolder(item);
        for (var files = new Enumerator(folder.Files); !files.atEnd(); files.moveNext()) {
            result.push(files.item().Path);
        }

        for (var subFolders = new Enumerator(folder.SubFolders); !subFolders.atEnd(); subFolders.moveNext()) {
            pending.push(subFolders.item().Path);
        }
    }

    return result;
}

function GetRelativePathFrom(startPath, endPath) {
    if (startPath[startPath.length - 1] !== "\\") {
        startPath += "\\";
    }

    if (startPath.length > endPath.length) {
        throw { message: "traversing up NYI" };
    }

    return endPath.substr(startPath.length);
}

function MatchesMask(file, mask) {
    if (!mask) {
        return false;
    }

    if (file === mask) {
        return true;
    }

    if (mask.substr(0, 1) === "*") {
        var rest = mask.substr(1);
        return file.substr(file.length - rest.length) === rest;
    } else if (mask.substr(mask.length - 1) === "*") {
        var end = mask.substr(0, mask.length - 1);
        return file.substr(0, end.length) === end;
    }

    return false;
}

function OpenCsvConnection(path) {
    /// <summary>Opens an ADO Connection object to the directory path where CSV files are found.</summary>
    /// <param name="path" type="String">Directory path where .csv files are.</param>
    /// <returns type="ADODB.Connection">An open connection.</returns>
    var connectionString = "Provider=Microsoft.Jet.OLEDB.4.0;" +
              "Data Source=" + path + ";" +
              "Extended Properties=\"text;HDR=YES;FMT=Delimited\"";
    var objConnection = WScript.CreateObject("ADODB.Connection");
    objConnection.Open(connectionString);
    return objConnection;
}

function OpenSqlCeConnection(path) {
    /// <summary>Opens an ADO Connection object to a SQL CE file.</summary>
    /// <param name="path" type="String">File path for .sdf file.</param>
    /// <returns type="ADODB.Connection">An open connection.</returns>
    var connectionString = "Provider=Microsoft.SQLSERVER.CE.OLEDB.4.0;" +
              "Data Source=" + path + ";";
    var objConnection = WScript.CreateObject("ADODB.Connection");
    objConnection.Open(connectionString);
    return objConnection;
}

function PathGetDirectory(path) {
    /// <summary>
    /// Returns the directory of the specified path string (excluding the trailing "\\");
    /// empty if there is no path.
    /// </summary>

    var l = path.length;
    var startIndex = l;
    while (--startIndex >= 0) {
        var ch = path.substr(startIndex, 1);
        if (ch == "\\") {
            if (startIndex === 0) {
                return "";
            } else {
                return path.substr(0, startIndex);
            }
        }
    }

    return "";
}

function PathGetFileName(path) {
    /// <summary>
    /// Returns the file name for the specified path string; empty if there is no
    /// directory information.
    /// </summary>
    var l = path.length;
    var startIndex = l;
    while (--startIndex >= 0) {
        var ch = path.substr(startIndex, 1);
        if (ch == "\\" || ch == "/" || ch == ":") {
            return path.substr(startIndex + 1);
        }
    }
    return "";
}

function PathGetExtension(path) {
    /// <summary>
    /// Returns the extension of the specified path string (including the ".");
    /// empty if there is no extension.
    /// </summary>
    /// <returns type="String">The extension of the specified path, including the '.'.</returns>
    var l = path.length;
    var startIndex = l;
    while (--startIndex >= 0) {
        var ch = path.substr(startIndex, 1);
        if (ch == ".") {
            if (startIndex != (l - 1)) {
                return path.substr(startIndex, l - startIndex);
            }
            return "";
        }
        else if (ch == "\\" || ch == ":") {
            break;
        }
    }
    return "";
}

function ReadAllTextFile(path) {
    /// <summary>Reads all the content of the file into a string.</summary>
    /// <param name="path" type="String">File name to read from.</param>
    /// <returns type="String">File contents.</returns>
    var ForReading = 1, ForWriting = 2;
    var fso = new ActiveXObject("Scripting.FileSystemObject");
    var file = fso.OpenTextFile(path, ForReading);
    try {
        var result;
        if (file.AtEndOfStream) {
            result = "";
        } else {
            result = file.ReadAll();
        }
    } finally {
        file.Close();
    }

    return result;
}

function ReadXmlFile(path) {
    /// <summary>Reads an XML document from the specified path.</summary>
    /// <param name="path" type="String">Path to file on disk.</param>
    /// <returns>A DOMDocument with the contents of the given file.</returns>
    var result = new ActiveXObject("Msxml2.DOMDocument.6.0");
    result.async = false;
    result.load(path);
    if (result.parseError.errorCode !== 0) {
        throw { message: "Error reading '" + path + "': " + result.parseError.reason };
    }

    return result;
}

// Runs the specified function catching exceptions and quits the current script.
function RunAndQuit(f) {
    try {
        f();
    }
    catch (e) {
        // An error with 'statusCode' defined will avoid the usual error dump handling.
        if (e.statusCode !== undefined) {
            if (e.message) {
                WScript.Echo(e.message);
            }

            WScript.Quit(e.statusCode);
        }

        WScript.Echo("Error caught while running this function:");
        WScript.Echo(f.toString());
        WScript.Echo("Error details:");
        if (typeof (e) == "object" && e.toString() == "[object Object]" || e.toString() === "[object Error]") {
            for (var p in e) WScript.Echo(" " + p + ": " + e[p]);
        }
        else {
            WScript.Echo(e);
        }

        WScript.Quit(1);
    }
    WScript.Quit(0);
}

function RunConsoleCommand(strCommand, timeout, retry) {
    /// <summary>Runs a command and waits for it to exit.</summary>
    /// <param name="strCommand" type="String">Command to run.</param>
    /// <param name="timeout" type="int">Timeout in seconds.</param>
    /// <param name="timeout" type="bool">Boolean specifying whether to retry on timeout or not.</param>
    /// <returns type="Array">
    /// An array with stdout in 0, stderr in 1 and exit code in 2. Forced
    /// termination sets the exit code to 1.
    /// </returns>

    // WScript.Echo("running [" + strCommand + "]");

    var WshShell = new ActiveXObject("WScript.Shell");
    var result = new Array(3);
    var oExec = WshShell.Exec(strCommand);
    var counter = 0;

    if (timeout) {
        // Status of 0 means the process is still running
        while (oExec.Status === 0 && counter < timeout) {
            WScript.Sleep(1000);
            counter++;
        }

        if (timeout === counter && oExec.Status === 0) {
            WScript.Echo("Forcefully terminating " + strCommand + " after " + timeout + " seconds.");
            oExec.Terminate();
            result[2] = 1;
            if (retry) {
                return RunConsoleCommand(strCommand, timeout, false);
            }
        }
    }

    result[0] = oExec.StdOut.ReadAll();
    result[1] = oExec.StdErr.ReadAll();

    if (!result[2]) {
        result[2] = oExec.ExitCode;
    }

    // WScript.Echo(" stdout:    " + result[0]);
    // WScript.Echo(" stderr:    " + result[1]);
    // WScript.Echo(" exit code: " + result[2]);

    return result;
}

function Say(text) {
    /// <summary>Uses the Speech API to speak to the user.</summary>
    var voice = new ActiveXObject("SAPI.SpVoice");
    try {
        voice.Speak(text);
    } catch (e) {
        // See http://msdn2.microsoft.com/en-us/library/ms717306.aspx for error codes.
        // SPERR_DEVICE_BUSY 0x80045006 -2147201018
        if (e.number == -2147201018) {
            WScript.Echo("The wave device is busy.");
            WScript.Echo("Happens sometimes over Terminal Services.");
        }
    }
}

function SaveTextToFile(content, path) {
    /// <summary>Saves text content into a file.</summary>
    /// <param name="content" type="String">Content to save.</param>
    /// <param name="path" type="String">Path of file to save into.</param>
    var ForReading = 1, ForWriting = 2;
    var fso = new ActiveXObject("Scripting.FileSystemObject");
    var file = fso.OpenTextFile(path, ForWriting, true);
    file.Write(content);
    file.Close();
}

function SelectLength(value) {
    return value.length;
}

function SendMicrosoftMail(subject, text, toAddress) {
    var shell = new ActiveXObject("WScript.Shell");
    var userName = shell.ExpandEnvironmentStrings("%username%");
    var fromAddress = userName + "@microsoft.com";
    var server = "smtphost.redmond.corp.microsoft.com";

    if (!toAddress) {
        toAddress = fromAddress;
    }

    var message = new ActiveXObject("CDO.Message");
    var configuration = message.Configuration;

    // 2 = cdoSendUsingPort; 2 = cdoNTLM
    configuration.Fields("http://schemas.microsoft.com/cdo/configuration/sendusing") = 2;
    configuration.Fields("http://schemas.microsoft.com/cdo/configuration/smtpserver") = server;
    configuration.Fields("http://schemas.microsoft.com/cdo/configuration/smtpauthenticate") = 2;
    configuration.Fields.Update();

    message.To = toAddress;
    message.From = fromAddress;
    message.Subject = subject;
    message.TextBody = text;
    message.Send();
}

function StringBetween(text, startMarker, endMarker) {
    /// <summary>Returns the text between startMarker and endMarker in text, null if not found.</summary>
    var startIndex = text.indexOf(startMarker);
    if (startIndex == -1) return null;
    var endIndex = text.indexOf(endMarker, startIndex + startMarker.length);
    if (endIndex == -1) return null;
    return text.substring(startIndex + startMarker.length, endIndex);
}

function StringPadRight(value, length, padString) {
    /// <summary>Returns a padded string.</summary>
    /// <param name="value" type="String">Value to pad.</param>
    /// <param name="length" type="Number" integer="true">Target length for string.</param>
    /// <param name="padString" type="String" optional="true">Optional string to pad with; defaults to space.</param>
    /// <returns type="String">The padded string.</returns>
    if (!padString) padString = " ";
    if (value.length < length) {
        value = value + Array(length + 1 - value.length).join(padString);
    }
    return value;
}

function StringToLower(text) {
    /// <summary>Returns the lowercase form of the specified string.</summary>
    /// <param name="text" type="String">Value to lower.</param>
    /// <returns type="String">The lowercase value.</returns>
    if (text) {
        return text.toLowerCase();
    } else {
        return text;
    }
}

function StringSplit(strLine, strSeparator) {
    /// <summary>Splits a string into a string array.</summary>
    var result = new Array();
    var startIndex = 0;
    var resultIndex = 0;
    while (startIndex < strLine.length) {
        var endIndex = strLine.indexOf(strSeparator, startIndex);
        if (endIndex == -1) {
            endIndex = strLine.length;
        }
        result[resultIndex] = strLine.substring(startIndex, endIndex);
        startIndex = endIndex + strSeparator.length;
        resultIndex++;
    }
    return result;
}

function StringTrim(text) {
    var result = text.replace(/^\s*/, "").replace(/\s*$/, "");
    return result
}

function PathCombine(path1, path2) {
    if (path1.charAt(path1.length - 1) !== "\\") {
        return path1 + "\\" + path2;
    }
    return path1 + path2;
}

function RemoveReadOnlyAttribute(path) {
    /// <summary>Removes the read-only attribute on the specified file.</summary>
    /// <param name="path" type="String">Path to the file.</param>
    var fso = new ActiveXObject("Scripting.FileSystemObject");
    var f = fso.getFile(path);
    if (1 === (f.attributes & 1)) {
        f.attributes = (f.attributes & ~1);
    }
}

function RunWmiQuery(query) {
    /// <summary>Runs a WMI query and returns all objects.</summary>
    /// <param name="query" type="String">Query to run.</param>
    /// <returns type="Array">Array with results.</returns>
    var result = [];
    var wmiService = GetObject("winmgmts:\\\\.\\root\\cimv2");
    var items = wmiService.ExecQuery(query);
    var e = new Enumerator(items);
    while (!e.atEnd()) {
        result.push(e.item());
        e.moveNext();
    }
    return result;
}

function WriteRecordSetToTextStreamAsAsciiTable(objRecordSet, stream) {
    /// <summary>Writes all content of the specified ADO RecordSet into the given text stream.</summary>
    /// <param name="objRecordSet">RecordSet to write out to.</param>
    /// <param name="stream">Optional TextStream to write to; defaults to WScript.Out.</param>
    var outStream = (stream) ? stream : WScript.StdOut;
    var fields = objRecordSet.Fields;
    var columns = new Array(fields.Count);
    var rowCount = 1;

    for (var i = 0; i < fields.Count; i++) {
        columns[i] = [];
        columns[i].push(fields.Item(i).Name);
    }

    while (!objRecordSet.EOF) {
        for (var i = 0; i < fields.Count; i++) {
            columns[i].push(DbValueToString(fields.Item(i).Value));
        }
        ++rowCount;
        objRecordSet.MoveNext();
    }

    var columnSizes = new Array(columns.length);
    for (var i = 0; i < columns.length; ++i) {
        var valueLengths = ArraySelect(columns[i], SelectLength);
        columnSizes[i] = ArrayMax(valueLengths);
    }

    for (var rowIndex = 0; rowIndex < rowCount; ++rowIndex) {
        for (var i = 0; i < columnSizes.length; ++i) {
            outStream.Write(StringPadRight(columns[i][rowIndex], 1 + columnSizes[i]));
        }
        outStream.WriteLine();
        if (rowIndex === 0) {
            for (var i = 0; i < columnSizes.length; ++i) {
                outStream.Write(StringPadRight("", columnSizes[i], "-"));
                outStream.Write(" ");
            }
            outStream.WriteLine();
        }
    }
}

function WriteRecordSetToTextStream(objRecordSet, separator, stream) {
    /// <summary>Writes all content of the specified ADO RecordSet into the given text stream.</summary>
    /// <param name="objRecordSet">RecordSet to write out to.</param>
    /// <param name="separator" type="String">Text between fields.</param>
    /// <param name="stream">Optional TextStream to write to; defaults to WScript.Out.</param>
    var outStream = (stream) ? stream : WScript.StdOut;
    var fields = objRecordSet.Fields;
    for (var i = 0; i < fields.Count; i++) {
        if (i > 0) outStream.Write(separator);
        outStream.Write(fields.Item(i).Name);
    }
    outStream.WriteLine();

    while (!objRecordSet.EOF) {
        for (var i = 0; i < fields.Count; i++) {
            if (i > 0) outStream.Write(separator);
            outStream.Write(fields.Item(i).Value);
        }
        outStream.WriteLine();
        objRecordSet.MoveNext();
    }
}

function WriteXmlFile(document, path) {
    /// <summary>Write an XML document to the specified path.</summary>
    /// <param name="path" type="String">Path to file on disk.</param>
    document.save(path);
}
