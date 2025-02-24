#include "dxc/DxilPdbInfo/DxilPdbInfoWriter.h"

#include "dxc/Support/Global.h"
#include "dxc/Support/WinIncludes.h"

#include "dxc/DxilCompression/DxilCompressionHelpers.h"
#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/DxilContainer/DxilRuntimeReflection.h"

using namespace hlsl;

HRESULT hlsl::WritePdbInfoPart(IMalloc *pMalloc,
                               const void *pUncompressedPdbInfoData,
                               size_t size, std::vector<char> *outBuffer) {
  // Write to the output buffer.
  outBuffer->clear();

  hlsl::DxilShaderPDBInfo header = {};
  header.CompressionType =
      hlsl::DxilShaderPDBInfoCompressionType::Zlib; // TODO: Add option to do
                                                    // uncompressed version.
  header.UncompressedSizeInBytes = size;
  header.Version = hlsl::DxilShaderPDBInfoVersion::Latest;
  {
    const size_t lastSize = outBuffer->size();
    outBuffer->resize(outBuffer->size() + sizeof(header));
    memcpy(outBuffer->data() + lastSize, &header, sizeof(header));
  }

  // Then write the compressed RDAT data.
  hlsl::ZlibResult result = hlsl::ZlibCompressAppend(
      pMalloc, pUncompressedPdbInfoData, size, *outBuffer);

  if (result == hlsl::ZlibResult::OutOfMemory)
    IFTBOOL(false, E_OUTOFMEMORY);
  IFTBOOL(result == hlsl::ZlibResult::Success, E_FAIL);

  IFTBOOL(outBuffer->size() >= sizeof(header), E_FAIL);
  header.SizeInBytes = outBuffer->size() - sizeof(header);
  memcpy(outBuffer->data(), &header, sizeof(header));

  return S_OK;
}
