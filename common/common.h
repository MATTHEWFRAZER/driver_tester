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
    // output size
    int out_size;

    // number of parameters
    int parameter_count;

    // actual arguments passed in to target routine
    DT_ARGUMENT *arguments;
} DT_ARGUMENTS;

typedef enum _DT_DECL_SPEC
{
   DECL_SPEC_CDECL = 0,
} DT_DECL_SPEC;

struct _DT_PATCH
{
     // next link in the linked list
     struct _DT_PATCH *next;

     // actual code to replace target code
     char *patch;

     // replaced code is stored here
     char *replaced_code;

     // address of the target routine
     unsigned int target_driver_routine_address;

     // address of the userland routine
     unsigned int userland_routine_address;

     // size of the code to replace target code
     int patch_size;

     // where we want to restore control to
     int original_routine_restore_point;

     DT_ARGUMENTS arguments;

     // simulated prolog of patched function
     //DT_PROLOG prolog;
     void (*prolog)(struct _DT_PATCH *patch);
};

typedef void USERLAND_CALL(DT_ARGUMENTS arguments, void *out);
