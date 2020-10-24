//#include <sys/stat.h>
//#include <string.h>
#include <asm/errno.h>
#include <linux/kallsyms.h>
#include <linux/slab.h>

#include "../../common/driver_tester_patch_request.h"
#include "../../common/common.h"
#include "driver_tester_detour_patching.h"
#include "cdecl_patch.h"

typedef void (*DT_PROLOG)(void *context);

struct _DT_PATCH
{
     // next link in the linked list
     struct _DT_PATCH *next;

     // actual code to replace target code
     char *patch;

     // replaced code is stored here
     char *replaced_code;

     // address of the target routine
     unsigned long target_driver_routine_address;

     // address of the userland routine
     unsigned long userland_routine_address;

     // size of the code to replace target code
     int patch_size;

     // where we want to restore control to
     unsigned long original_routine_restore_point;

     DT_ARGUMENTS arguments;

     // simulated prolog of patched function
     DT_PROLOG prolog;
};

static struct _DT_PATCH *g_dt_patches;
static char g_dev_path[5]  = "/dev/";

static void dt_detour_patching_remove_patch(struct _DT_PATCH *patch)
{
    struct _DT_PATCH *cursor;

    if (patch == NULL)
    {
        printk(KERN_WARNING "%s(): patch is null", __FUNCTION__);
        return;
    }

    for(cursor = patch; patch != NULL; cursor = cursor->next)
    {
        if (cursor == NULL)
        {
            printk(KERN_WARNING "%s(): cursor is null, did not find patch", __FUNCTION__);
            break;
        }

        if(cursor == patch)
        {
            int i;
            if (cursor->patch != NULL)
            {
                kfree(cursor->patch);
            }

            if (cursor->replaced_code != NULL)
            {
                kfree(cursor->replaced_code);
            }

            for(i = 0; i < (cursor->arguments).parameter_count; ++i)
            {
                kfree((cursor->arguments).arguments[i].data);
            }

            kfree(cursor);
            break;
        }
    }
    return;
}

static void dt_detour_patching_append_patch(struct _DT_PATCH **patch)
{
    if (patch == NULL)
    {
        printk(KERN_WARNING "%s(): patch is null", __FUNCTION__);
        return;
    }

    if (*patch != NULL)
    {
        printk(KERN_NOTICE "%s(): patch needs to be removed and reallocated", __FUNCTION__);
        dt_detour_patching_remove_patch(*patch);
    }

    (*patch) = kmalloc(sizeof(**patch), GFP_KERNEL);

    if (*patch != NULL)
    {
        (*patch)->patch         = kmalloc((*patch)->patch_size, GFP_KERNEL);
        (*patch)->replaced_code = kmalloc((*patch)->patch_size, GFP_KERNEL);
        (*patch)->next          = g_dt_patches;
        g_dt_patches            = *patch;
    }
    return;
}

static inline int is_driver_loaded(char *path, char *target_driver)
{
    /*char[256] targetDriverPath;
    struct stat st;

    strcpy(targetDriverPath, path);
    strcpy(targetDriverPath, targetDriver);

    if (stat(targeteDriverPath, &st) != 0)
    {
        printk(KERN_NOTICE "%s() could not stat module with name %s", __function__, targetDriver);
        return EINVAL;
    }
    else
    {
        return 0;
    }*/
    return 0;
}

#define TRAMPOLINE_SIZE 6

void dt_detour_patching_prolog_detour(void) __attribute__((naked));

void dt_detour_patching_prolog_detour(void)
{
    int i;
    struct _DT_PATCH *patch = g_dt_patches;
    DT_PROLOG prolog;
    USERLAND_CALL userland_call;
    void *out;

    int address;

    // get patch
    for(i = 0; patch != NULL; patch = patch->next)
    {
       if (patch->target_driver_routine_address == address)
       {
            prolog = patch->prolog;
            break;
       }
    }

    if(prolog != NULL)
    {
        prolog(patch);
    }

    // jump to patch
    /*__asm__
    (
       "call %0"
       : "r" (patch->userlandRoutineAddress)
    );*/
    userland_call = (USERLAND_CALL)patch->userland_routine_address;
    out = kmalloc(patch->arguments.out_size, GFP_KERNEL);
    if (out == NULL)
    {
        return;//return ENOMEM;
    }

    userland_call(patch->arguments, out);

    // jump to original code
    __asm__
    (
         "push %0\n\t"
         "ret"
         : :"r" (patch->original_routine_restore_point)
    );
}

static inline int is_valid_patch(struct _DT_PATCH *patch, DT_PATCH_REQUEST *patch_request)
{
    if(patch == NULL || patch->patch == NULL || patch->replaced_code)
    {
        printk(KERN_WARNING "%s(): allocation failed", __FUNCTION__);
        return 0;
    }

    if (patch_request == NULL)
    {
        printk(KERN_WARNING "%s(): patchRequest is null", __FUNCTION__);
        return 0;
    }

    return 1;
}

static DT_PROLOG dt_detour_get_prolog(DT_DECL_SPEC decl_spec)
{
    switch(decl_spec)
    {
        case DECL_SPEC_CDECL:
            return dt_cdecl_patch_prolog;
        default:
            return NULL;
    }
}

static int dt_detour_patching_apply_patch(unsigned long target_driver_routine_address,
                                          DT_PATCH_REQUEST *patch_request)
{
    int i;
    char trampoline_bytes[TRAMPOLINE_SIZE] = {0x68, 0x00, 0x00, 0x00, 0x00, 0xC3}; // push <addr>; ret;
    struct _DT_PATCH *patch;
    char *target_driver_routine_address_as_bytes;
    char *detour_as_bytes;
    DT_PROLOG prolog;

    prolog = dt_detour_get_prolog(patch_request->decl_spec);
    if (prolog == NULL)
    {
        return ENOTTY;
    }

    target_driver_routine_address_as_bytes = ((char *)target_driver_routine_address) + patch_request->target_driver_routine_offset;
    detour_as_bytes = (char *)dt_detour_patching_prolog_detour;
    dt_detour_patching_append_patch(&patch);

    patch->patch_size = patch_request->bytes_required;
    patch->target_driver_routine_address = target_driver_routine_address;
    patch->userland_routine_address = patch_request->userland_routine_address;
    patch->prolog = *prolog;

    if(!is_valid_patch(patch, patch_request))
    {
        printk(KERN_WARNING "%s(): need to remove patch", __FUNCTION__);
        return ENOTTY;
    }

    for(i = 0; i < patch->patch_size ; ++i)
    {
        // save the original code so we can unapply the patch
        patch->replaced_code[i] = target_driver_routine_address_as_bytes[i];
        if (i >= TRAMPOLINE_SIZE) // copy noop to pad patch to align with instructions
        {
            patch->patch[i] = 0x90;
        }
        else if (i >= 1 && i <= 4) // write out address of our detour
        {
           patch->patch[i] = detour_as_bytes[i];
        }
        else // copy bytes as they appear in original trampoline
        {
            patch->patch[i] = trampoline_bytes[i];
        }
        // patch target routine
        target_driver_routine_address_as_bytes[i] = patch->patch[i];
    }
    return 0;
}

static int dt_detour_patching_unapply_patch(struct _DT_PATCH *patch)
{
    int i;
    char * address_as_bytes;

    if (patch == NULL)
    {
        return ENOTTY;
    }

    address_as_bytes = (char *)patch->target_driver_routine_address;
    for(i = 0; i < patch->patch_size; ++i)
    {
        address_as_bytes[i] = patch->replaced_code[i];
    }
    return 0;
}

static inline int is_valid_patch_request(char *path,
                                         DT_PATCH_REQUEST *patch_request)
{
    if (path == NULL)
    {
        printk(KERN_WARNING "%s(): patch is null", __FUNCTION__);
        return 0;
    }

    if (patch_request == NULL)
    {
        printk(KERN_WARNING "%s(): patchRequest is null", __FUNCTION__);
        return 0;
    }

    if (patch_request->target_driver_name == NULL)
    {
        printk(KERN_WARNING "%s(): targetDriverName is null", __FUNCTION__);
        return 0;
    }

    if (patch_request->target_routine_name == NULL)
    {
        printk(KERN_WARNING "%s(): targetRoutineName is null", __FUNCTION__);
        return 0;
    }

    return 1;
}

static int dt_detour_patching_patch_inner(char *path,
                                          DT_PATCH_REQUEST *patch_request)

{
    unsigned long target_driver_routine_address;

    if (!is_valid_patch_request(path, patch_request))
    {
        return EINVAL;
    }

    if (!is_driver_loaded(path, patch_request->target_driver_name))
    {
        printk(KERN_WARNING "%s(): driver is not loaded", __FUNCTION__);
        return EINVAL;
    }

    target_driver_routine_address = kallsyms_lookup_name(patch_request->target_routine_name);
    if (target_driver_routine_address == 0)
    {
        printk(KERN_WARNING "%s(): could not find target routine %s", __FUNCTION__, patch_request->target_routine_name);
        return EINVAL;
    }

    return dt_detour_patching_apply_patch(target_driver_routine_address, patch_request);
}

void dt_detour_patching_init(void)
{
    g_dt_patches = NULL;
    return;
}

void dt_detour_patching_exit(void)
{
    struct _DT_PATCH *cursor = g_dt_patches;
    while (cursor != NULL)
    {
        struct _DT_PATCH *to_unapply = cursor;
        cursor = cursor->next;
        dt_detour_patching_unapply_patch(to_unapply);
        dt_detour_patching_remove_patch(to_unapply);
    }
    return;
}

int dt_detour_patching_patch(DT_PATCH_REQUEST *patch_request)
{
    return dt_detour_patching_patch_inner(g_dev_path, patch_request);
}
