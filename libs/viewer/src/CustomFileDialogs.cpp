#include <viewer/CustomFileDialogs.h>

#if defined( WIN32 )
#include <Windows.h>
#include <ShlObj_core.h>
#include <cstring>
#else // defined( WIN32 )
#endif

#if defined( WIN32 )

bool SD_OpenFileDialog(char* browsedFile, const char* formats) {
    OPENFILENAME ofn;       // common dialog box structure
    TCHAR szFile[1024] = { 0 };       // if using TCHAR macros

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = formats;
    ofn.nFilterIndex = 0;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    BOOL result = GetOpenFileNameA(&ofn);
    if (result == TRUE)
    {
        lstrcpynA(browsedFile, ofn.lpstrFile, 1024);        
        // use ofn.lpstrFile
    }

    return result;
}

bool SD_SaveFileDialog(char* browsedFile, const char* formats) {
    OPENFILENAME ofn;       // common dialog box structure
    TCHAR szFile[1024] = { 0 };       // if using TCHAR macros

    // Initialize OPENFILENAME even though we will use it for a save file dialog
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = formats;
    ofn.nFilterIndex = 0;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = 0;

    BOOL result = GetSaveFileNameA(&ofn);
    if (result == TRUE)
    {
        lstrcpynA(browsedFile, ofn.lpstrFile, 1024);
        // use ofn.lpstrFile
    }

    return result;
}

bool SD_OpenFolderDialog(char* browsedFile) {
    BROWSEINFO browseInfo;
    ZeroMemory(&browseInfo, sizeof(BROWSEINFO));

    auto result = SHBrowseForFolder(&browseInfo);
    if (result) {
        if (SHGetPathFromIDList(result, browsedFile)) {
            return true;
        }
        else {
            return false;
        }
    }

    return false;
}

#else // defined( WIN32 )

#include <viewer/MacosCustomFileDialogs.h>

bool SD_OpenFileDialog(char* browsedFile, const char* formats) {
    return SD_MacOpenFileDialog(browsedFile, formats);
}

bool SD_SaveFileDialog(char* browsedFile, const char* formats) {
    return SD_MacSaveFileDialog(browsedFile, formats);
}

bool SD_OpenFolderDialog(char* browsedFile) {
    return SD_MacOpenFileDialog(browsedFile, nullptr, true);;
}

#endif

