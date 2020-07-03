//#include <sys/stat.h>
//#include <string.h>
#include <asm/errno.h>
#include <linux/kallsyms.h>
#include <linux/slab.h>

#include "driver_tester_detour_patching.h"

typedef struct DT_PATCH
{
     struct DT_PATCH *next;
     char *patch;
     char *replacedCode;
     unsigned int targetDriverRoutineAddress;
     int size;
};

static struct DT_PATCH *gDTPatches;
static char gDevPath[5]  = "/dev/";

static void dt_detour_patching_append_patch(struct DT_PATCH **patch)
{
    (*patch) = kmalloc(sizeof(**patch), GFP_KERNEL);
    (*patch)->patch = kmalloc((*patch)->size, GFP_KERNEL);
    (*patch)->replacedCode = kmalloc((*patch)->size, GFP_KERNEL);
    (*patch)->next = gDTPatches;
    return;
}

static void dt_detour_patching_remove_patch(struct DT_PATCH *patch)
{
    struct DT_PATCH *cursor;
    for(cursor = patch; patch != NULL; cursor = cursor->next)
    {
        if(cursor == patch)
        {
            kfree(cursor->patch);
            kfree(cursor->replacedCode);
            kfree(cursor);
            break;
        }
    }
    return;
}

static inline int is_driver_loaded(char *path, char *targetDriver)
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

static int dt_detour_patching_apply_patch(unsigned long targetDriverRoutineAddress,
                                          DT_PATCH_REQUEST *patchRequest)
{
    int i;
    char trampolineBytes[TRAMPOLINE_SIZE] = {0x68, 0x00, 0x00, 0x00, 0x00, 0xC3};
    struct DT_PATCH *patch;
    char *targetDriverRoutineAddressAsBytes = (char *)targetDriverRoutineAddress;
    char *userlandRoutineAddressAsBytes = (char *)patchRequest->userlandRoutineAddress;
    dt_detour_patching_append_patch(&patch);
    patch->size = patchRequest->bytesRequired;
    patch->targetDriverRoutineAddress = targetDriverRoutineAddress;

    for(i = 0; i < patch->size; ++i)
    {
        // save the original code so we can unapply the patch
        patch->replacedCode[i] = targetDriverRoutineAddressAsBytes[i];
        if (i >= TRAMPOLINE_SIZE) // copy noop to pad patch to align with instructions
        {
            patch->patch[i] = 0x90;
        }
        else if (i >= 1 && i <= 4) // write out adress of routine we are provided
        {
           patch->patch[i] = userlandRoutineAddressAsBytes[i];
        }
        else // copy bytes as they appear in original trampoline
        {
            patch->patch[i] = trampolineBytes[i];
        }
        // patch target routine
        targetDriverRoutineAddressAsBytes[i] = patch->patch[i];
    }
    return 0;
}

static int dt_detour_patching_unapply_patch(struct DT_PATCH *patch)
{
    int i;
    char * addressAsBytes = (char *)patch->targetDriverRoutineAddress;
    for(i = 0; i < patch->size; ++i)
    {
        addressAsBytes[i] = patch->replacedCode[i];
    }
    return 0;
}

static int dt_detour_patching_patch_inner(char *path,
                                          DT_PATCH_REQUEST *patchRequest)

{
    unsigned long targetDriverRoutineAddress;

    if (!is_driver_loaded(path, patchRequest->targetDriverName))
    {
        return EINVAL;
    }

    targetDriverRoutineAddress = kallsyms_lookup_name(patchRequest->targetRoutineName);
    if (targetDriverRoutineAddress == 0)
    {
        return EINVAL;
    }

    return dt_detour_patching_apply_patch(targetDriverRoutineAddress, patchRequest);
}

void dt_detour_patching_init(void)
{
    gDTPatches = NULL;
    return;
}

void dt_detour_patching_exit(void)
{
    struct DT_PATCH *cursor = gDTPatches;
    while (cursor != NULL)
    {
        struct DT_PATCH *toUnapply = cursor;
        cursor = cursor->next;
        dt_detour_patching_unapply_patch(toUnapply);
        dt_detour_patching_remove_patch(toUnapply);
    }
    return;
}

int dt_detour_patching_patch(DT_PATCH_REQUEST *patchRequest)
{
    return dt_detour_patching_patch_inner(gDevPath, patchRequest);
}
