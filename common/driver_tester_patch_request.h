#pragma once

#include "common.h"

// need 6 bytes for signature. We need userland to give us
// bytes available
typedef struct _DT_PATCH_REQUEST
{
    char *targetRoutineName;
    char *targetDriverName;
    int targetDriverRoutineOffset;
    unsigned long userlandRoutineAddress;
    int *displacedOperands;
    int displacedOperandsSize;
    int bytesRequired;
    int useDisplacedOperands;
    DECL_SPEC declSpec;
} DT_PATCH_REQUEST;
