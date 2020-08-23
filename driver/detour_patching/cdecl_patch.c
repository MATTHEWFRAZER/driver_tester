__declspec(naked) void dt_detour_patching_prolog_load_stack(DT_PATCH_REQUEST)
{
    // use values passed into patch to construct stack
    return;
}

__declspec(naked) void dt_cdecl_patch_epilog(DT_PARAMETER *parameters, int size)
{
    int i;

    if (parameters == NULL)
    {
        return;
    }

    for(i = size - 1; i >= 0; --i)
    {
        __asm__
        (
           "movl -%1(%%rbp), %0"
           : "=r" (parameters[i].data) : "r" (parameters[i].size)
        );
    }
    return;
}
