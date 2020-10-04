#pragma once

#include "common.h"

// need 6 bytes for signature. We need userland to give us
// bytes available
typedef struct _DT_PATCH_REQUEST
{
    char *target_routine_name;
    char *target_driver_name;
    int target_driver_routine_offset;
    unsigned long userland_routine_address;
    int *displaced_operands;
    int displaced_operand_count;
    int bytes_required;
    int use_displaced_operands;
    DT_DECL_SPEC decl_spec;
    int parameter_sizes;
} DT_PATCH_REQUEST;
