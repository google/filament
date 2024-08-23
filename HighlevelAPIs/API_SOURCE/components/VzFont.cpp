#include "VzTexture.h"
#include "../VzEngineApp.h"
#include "../FIncludes.h"

#include <fstream>
#include <iostream>

extern Engine* gEngine;
extern vzm::VzEngineApp gEngineApp;

namespace vzm
{
    bool VzFont::ReadFont(const std::string& fileName, const uint32_t fontSize)
    {
        VzFontRes* font_res = gEngineApp.GetFontRes(GetVID());

        Path file_name(fileName);
        if (!file_name.exists()) {
            backlog::post("The input font does not exist: " + fileName, backlog::LogLevel::Error);
            return false;
        }

        font_res->path_ = fileName;
        font_res->size_ = fontSize > 0 ? fontSize : 10;

        if (!gEngineApp.ftLibrary) {
            backlog::post("FreeType library is not initialized!", backlog::LogLevel::Error);
            return false;
        }

        FT_Error error;

        error = FT_New_Face(gEngineApp.ftLibrary, fileName.c_str(), 0, &font_res->ftFace_);
        if (error) {
            backlog::post("Failed to load font: " + fileName, backlog::LogLevel::Error);
            return false;
        }

        error = FT_Set_Char_Size(font_res->ftFace_, font_res->size_ << 6, 0, 72, 72);
        if (error) {
            backlog::post("Failed to set font size: " + fileName, backlog::LogLevel::Error);
            return false;
        }

        return true;
    }

    std::string VzFont::GetFontFileName()
    {
        VzFontRes* font_res = gEngineApp.GetFontRes(GetVID());
        return font_res->path_;
    }
}
