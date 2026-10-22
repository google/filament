///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// hcttodo.js                                                                //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//
// Verifies that all TODO comments have some explanation.
//

eval(new ActiveXObject("Scripting.FileSystemObject").OpenTextFile(new ActiveXObject("WScript.Shell").ExpandEnvironmentStrings("%HLSL_SRC_DIR%\\utils\\hct\\hctjs.js"), 1).ReadAll());

// It would be nice to tag with issue numbers.
// var goodToDoLine = /TODO: HLSL #[0-9]+ - [0-9a-zA-Z]+/;
var goodToDoLine = /TODO: HLSL #[0-9]+ - [0-9a-zA-Z]+/;

var fileExts = [
  ".c", ".cpp", ".h", ".td", ".cs", ".hlsl", ".fx"
];

function CountBadTodoLines(files) {
    var badTodoCount = 0;
    for (var i = 0; i < files.length; i++) {
        var path = files[i];
        var extension = PathGetExtension(path).toLowerCase();
        if (ArrayIndexOf(fileExts, extension) >= 0) {
            var content = ReadAllTextFile(path);
            var lines = StringSplit(content, "\n");
            for (var j = 0; j < lines.length; j++) {
                var line = lines[j];
                var todoIndex = line.indexOf("TODO");
                if (todoIndex !== -1) {
//                    if (!goodToDoLine.exec(line)) {
                        WScript.Echo(path + "(" + (j+1) + ":" + (todoIndex+1) + "): " + lines[j]);
//                        badTodoCount += 1;
//                    }
                }
            }
        }
    }

    return badTodoCount;
}

function ExpandPath(path) {
    return new ActiveXObject("WScript.Shell").ExpandEnvironmentStrings(path);
}

var ignorePaths = [
  "%HLSL_SRC_DIR%\\lib\\target\\hexagon",
  "%HLSL_SRC_DIR%\\lib\\executionengine",
  "%HLSL_SRC_DIR%\\lib\\target\\arm",
  "%HLSL_SRC_DIR%\\lib\\target\\mips",
  "%HLSL_SRC_DIR%\\lib\\target\\nvptx",
  "%HLSL_SRC_DIR%\\lib\\target\\r600",
  "%HLSL_SRC_DIR%\\lib\\target\\powerpc",
  "%HLSL_SRC_DIR%\\lib\\target\\x86",
  "%HLSL_SRC_DIR%\\lib\\target\\xcore"
];
ignorePaths = ArraySelect(ignorePaths, function(path) { return StringToLower(ExpandPath(path)); });

var files = GetFilesRecursive(ExpandPath("%HLSL_SRC_DIR%"));
files = ArraySelectMany(files, function (path) {
    path = StringToLower(path);
    if (ArrayAny(ignorePaths, function (ignore) { return path.indexOf(ignore) === 0; })) {
        return [];
    } else {
        return [path];
    }
});

var counts = CountBadTodoLines(files);

// Fail on any bad TODO lines.
if (counts !== 0) {
    WScript.Echo("Badly formatted TODO comments found in " + files.length + " files: " + counts);
    WScript.Echo("\nFormat should be something like this, with the right number and description (spaces and symbols enforced for consistency).");
    WScript.Echo("  // TODO: HLSL #123 - description");
    WScript.Quit(1);
} else {
    WScript.Echo("No badly formatted TODO comments found in " + files.length + " files.");
    WScript.Quit(0);
}
