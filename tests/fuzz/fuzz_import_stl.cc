#include "fuzz_common.h"
#include "geometry/PolySet.h"
#include "io/import.h"
FUZZ_INIT_AND_LOAD(".stl", import_stl(filename, fuzz_location))
