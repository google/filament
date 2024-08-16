#ifndef VIEWER_SAVEFILE_IO
#define VIEWER_SAVEFILE_IO
#include "VizEngineAPIs.h"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#undef GetObject
namespace savefileIO {
std::string getRelativePath(std::string absolute_path);

void setResPath(std::string assetPath);

void importSettings(VID root, std::string filePath, vzm::VzRenderer* renderer,
                    vzm::VzScene* scene, vzm::VzLight* sunLight);

void exportSettings(VID root, vzm::VzRenderer* renderer, vzm::VzScene* scene,
                    vzm::VzLight* sunLight);

}  // namespace savefileIO
#endif
