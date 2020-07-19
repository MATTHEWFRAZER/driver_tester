#pragma once

// need 6 bytes for signature. We need userland to give us
// bytes available

typedef struct _DT_PATCH_REQUEST
{
    char *targetRoutineName;
    char *targetDriverName;
    unsigned long userlandRoutineAddress;
    int *displacedOperands;
    int displacedOperandsSize;
    int bytesRequired;
} DT_PATCH_REQUEST;
