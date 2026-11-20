#include "core/EvaluationSession.h"
#include "core/Value.h"
#include "fuzz_common.h"
#include "io/import.h"
static EvaluationSession session{"/"};
FUZZ_INIT_AND_LOAD(".json", import_json(filename, &session, fuzz_location))
