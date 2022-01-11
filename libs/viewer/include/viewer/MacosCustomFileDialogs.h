#ifndef VIEWER_MACOSCUSTOMFILEDIALOGS
#define VIEWER_MACOSCUSTOMFILEDIALOGS

bool SD_MacOpenFileDialog(char* browsedFile, const char* formats, bool folderOnly = false);
bool SD_MacSaveFileDialog(char* browsedFile, const char* formats);

#endif

