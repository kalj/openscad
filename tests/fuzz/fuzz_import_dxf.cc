#include "fuzz_common.h"
#include "geometry/Polygon2d.h"
#include "io/DxfData.h"
FUZZ_INIT_AND_LOAD(".dxf", DxfData(CurveDiscretizer(0.0, 2.0, 12.0), filename).toPolygon2d())
