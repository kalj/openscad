// Parses (but does not evaluate) the input as an OpenSCAD source file.
#include <filesystem>

#include "core/SourceFile.h"
#include "core/parsersettings.h"
#include "fuzz_common.h"
#include "platform/PlatformUtils.h"

namespace fs = std::filesystem;

bool parse(SourceFile *& file, const std::string& text, const std::string& filename,
           const std::string& mainFile, int debug);

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
  fuzz_init();
  PlatformUtils::registerApplicationPath(fs::absolute(fs::path((*argv)[0])).parent_path().generic_string());
  parser_init();
  return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  const std::string text(reinterpret_cast<const char *>(data), size);
  SourceFile *file = nullptr;
  parse(file, text, "/fuzz.scad", "/fuzz.scad", 0);
  delete file;
  return 0;
}
