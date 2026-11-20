#include "fuzz_common.h"
#include "geometry/PolySet.h"
#include "io/import.h"
FUZZ_INIT_AND_LOAD(".obj", import_obj(filename, fuzz_location))
