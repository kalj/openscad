#include "fuzz_common.h"
#include "geometry/Polygon2d.h"
#include "io/import.h"
// Default $fn, $fs, $fa.
FUZZ_INIT_AND_LOAD(".svg", import_svg(CurveDiscretizer(0.0, 2.0, 12.0), filename, boost::none, boost::none,
                                      72.0, false, fuzz_location))
