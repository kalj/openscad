#include "core/EvaluationSession.h"
#include "core/Value.h"
#include "fuzz_common.h"
#include "io/import.h"
FUZZ_INIT_AND_LOAD(".json", EvaluationSession session{"/"}; import_json(filename, &session, fuzz_location))
