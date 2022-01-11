#pragma once

static const char* kTextureDialogFilter = "All\0*.*\0Jpg\0*.JPG\0Png\0*.PNG\0Tga\0*.TGA\0Bmp\0*.bmp\0Psd\0*.PSD\0Gif\0*.GIF\0HDR\0*.HDR\0Pic\0*.PIC\0";

bool SD_OpenFileDialog(char* browsedFile, const char* formats = kTextureDialogFilter);
bool SD_SaveFileDialog(char* browsedFile, const char* formats = kTextureDialogFilter);

bool SD_OpenFolderDialog(char* browsedFile);
