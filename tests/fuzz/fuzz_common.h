// Shared helpers for the libFuzzer targets in this directory.
#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <optional>
#include <string>

#include <unistd.h>

#include "core/AST.h"
#include "core/CurveDiscretizer.h"
#include "utils/printutils.h"

static const Location fuzz_location{0, 0, 0, 0, nullptr};

// The importers read from a file, so write the input to a per-process temp file.
static std::string fuzz_write_file(const uint8_t *data, size_t size, const char *ext)
{
  static std::string path;
  if (path.empty()) {
    const char *tmp = getenv("TMPDIR");
    path = std::string(tmp ? tmp : "/tmp") + "/openscad-fuzz-" + std::to_string(getpid()) + ext;
  }
  FILE *f = fopen(path.c_str(), "wb");
  if (!f) abort();
  if (size > 0 && fwrite(data, 1, size, f) != size) abort();
  fclose(f);
  return path;
}

// Default $fn, $fs, $fa (and $fe).
static CurveDiscretizer fuzz_discretizer()
{
  return CurveDiscretizer([](const char *) { return std::optional<double>(); });
}

static void fuzz_quiet_output(const Message&, void *) {}

// Silence console output; set OPENSCAD_FUZZ_VERBOSE to see warnings.
static void fuzz_init()
{
  if (!getenv("OPENSCAD_FUZZ_VERBOSE")) set_output_handler(&fuzz_quiet_output, nullptr, nullptr);
}

#define FUZZ_INIT_AND_LOAD(ext, call)                                     \
  extern "C" int LLVMFuzzerInitialize(int *, char ***)                    \
  {                                                                       \
    fuzz_init();                                                          \
    return 0;                                                             \
  }                                                                       \
  extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) \
  {                                                                       \
    const std::string filename = fuzz_write_file(data, size, ext);        \
    (void)call;                                                               \
    return 0;                                                             \
  }
