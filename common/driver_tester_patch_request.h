#pragma once

// need 6 bytes for signature. We need userland to give us
// bytes available

// offsets (from esp) and sizes of each argument
typedef struct _ARGUMENT_OFFSETS
{
    int offset;
    int size;
} ARGUMENT_OFFSETS;

typedef struct _DT_PATCH_REQUEST
{
    char *targetRoutineName;
    char *targetDriverName;
    unsigned long userlandRoutineAddress;
    ARGUMENT_OFFSETS *argmentOffsets;
    int bytesRequired;
} DT_PATCH_REQUEST;
