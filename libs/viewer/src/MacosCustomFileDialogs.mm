#ifdef __APPLE__

#include <Availability.h>
#include <TargetConditionals.h>
#if TARGET_OS_OSX

#import <AppKit/Appkit.h>
#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>
#include <cctype>

#include <viewer/MacosCustomFileDialogs.h>

// Hacky extension extractor from Windows-specific filter format string
static NSArray<NSString*>* GetExtensionsFromFormatString(const char* formats) {
    const char* nextStringStart = formats;
    NSMutableArray<NSString*>* result = [[NSMutableArray<NSString*> alloc] init];

    while (*nextStringStart != '\0') {
        // Skip the description string
        while (*nextStringStart != '\0') ++nextStringStart;
        // Got to the beginning of the extension string
        ++nextStringStart;
        // We hit the end of the "list"
        if (*nextStringStart == '\0') {
            break;
        }
        // Skip "." or "*." at the beginning
        while (!std::isalnum(*nextStringStart) && *nextStringStart != '\0') ++nextStringStart;

        // We probably had "*.*", so do no filtering
        if (*nextStringStart == '\0') {
            return nil;
        }

        // This does not support multiple extensions like "Text docs\0*.txt;*.rtf;*.doc\0Spreadsheets\0*.xls;*.xlsx\0\0"
        // Please don't do that!
        [result addObject:[NSString stringWithUTF8String:nextStringStart]];
    }

    return result;
}

#if (__MAC_OS_X_VERSION_MIN_REQUIRED >= 110000)
static NSArray<UTType*>* ConvertFormatsToUTI(const char* formats)
{
    NSMutableArray<UTType*>* result = [[NSMutableArray<UTType*> alloc] init];
    NSArray<NSString*>* extensions = GetExtensionsFromFormatString(formats);

    if (extensions == nil) {
        return result;
    } else {
        for (NSString* ext in extensions) {
            UTType* utType = [UTType typeWithFilenameExtension:ext];
            if (utType != nil) {
                [result addObject:utType];
            }
        }
    }

    return result;
}
#endif // __MAC_OS_X_VERSION_MIN_REQUIRED >= 110000

bool SD_MacOpenFileDialog(char* browsedFile, const char* formats, bool folderOnly) {
    NSOpenPanel* openPanel = [NSOpenPanel openPanel];

    openPanel.canChooseDirectories = folderOnly ? YES : NO;
    openPanel.canChooseFiles = !folderOnly ? YES : NO;

    if (!folderOnly) {
        if (@available(macOS 11.0, *)) {
            openPanel.allowedContentTypes = ConvertFormatsToUTI(formats);
        } else {
            openPanel.allowedFileTypes = GetExtensionsFromFormatString(formats);
        }
    }
    NSModalResponse response = [openPanel runModal];
    
    if (response == NSModalResponseOK) {
        NSURL* theDoc = [[openPanel URLs] objectAtIndex:0];
        strcpy(browsedFile, theDoc.fileSystemRepresentation); // TODO insecure af
        return true;
    } else {
        return false;
    }
}

bool SD_MacSaveFileDialog(char* browsedFile, const char* formats) {
    NSSavePanel* savePanel = [NSSavePanel savePanel];
    NSModalResponse response = [savePanel runModal];

    if (response == NSModalResponseOK) {
        NSURL* theDoc = savePanel.URL;
        strcpy(browsedFile, theDoc.fileSystemRepresentation); // TODO insecure af
        return true;
    } else {
        return false;
    }
}

#else // TARGET_OS_OSX

bool SD_MacOpenFileDialog(char* browsedFile, const char* formats, bool folderOnly) {
    return false;
}

bool SD_MacSaveFileDialog(char* browsedFile, const char* formats) {
    return false;
}

#endif // TARGET_OS_OSX

#endif // __APPLE__
