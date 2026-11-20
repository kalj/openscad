#include "fuzz_common.h"
#include "geometry/Polygon2d.h"
#include "io/import.h"
FUZZ_INIT_AND_LOAD(".svg", import_svg(fuzz_discretizer(), filename, boost::none, boost::none,
                                      72.0, false, fuzz_location))
