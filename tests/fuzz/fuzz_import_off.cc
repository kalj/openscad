#include "fuzz_common.h"
#include "geometry/PolySet.h"
#include "io/import.h"
FUZZ_INIT_AND_LOAD(".off", import_off(filename, fuzz_location))
