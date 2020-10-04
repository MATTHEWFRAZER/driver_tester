#pragma once

typedef struct _DT_ARGUMENT
{
    // argument size
    int size;

    // argument data
    void *data;
} DT_ARGUMENT;

typedef struct _DT_ARGUMENTS
{
    // number of parameters
    int parameter_count;

    // actual arguments passed in to target routine
    DT_ARGUMENT *arguments;
} DT_ARGUMENTS;

typedef enum _DT_DECL_SPEC
{
   DECL_SPEC_CDECL = 0,
} DT_DECL_SPEC;

typedef void USERLAND_CALL(DT_ARGUMENTS arguments, void *out);
