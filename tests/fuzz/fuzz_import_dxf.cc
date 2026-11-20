#include "fuzz_common.h"
#include "geometry/Polygon2d.h"
#include "io/DxfData.h"
FUZZ_INIT_AND_LOAD(".dxf", DxfData(fuzz_discretizer(), filename).toPolygon2d())
