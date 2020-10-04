#include <linux/slab.h>
#include "../../common/common.h"

// take arguments off the stack, we want our patch code to not directly pull arguments off the stack
__declspec(naked) void dt_cdecl_patch_prolog(void *context)
{
    int i;
    DT_PATCH *patch = (DT_PATCH*)context;

    if (arguments == NULL)
    {
        return;
    }

    for(i = patch->parameter_count; i >= 0; --i)
    {
       patch->arguments[i].data = (void *)kmalloc(patch->parameter_sizes[i]);
       patch->arguments[i].size = patch->parameters_sizes[i];
        __asm__
        (
           "movl -%1(%%rbp), %0"
           : "=r" (arguments[i].data) : "r" (arguments[i].size)
        );
    }
    return;
}
