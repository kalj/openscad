#include "fuzz_common.h"
#include "geometry/PolySet.h"
#include "io/import.h"
FUZZ_INIT_AND_LOAD(".3mf", import_3mf(filename, fuzz_location))
