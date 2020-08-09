//#include <sys/stat.h>
//#include <string.h>
#include <asm/errno.h>
#include <linux/kallsyms.h>
#include <linux/slab.h>

#include "../../common/driver_tester_patch_request.h"
#include "driver_tester_detour_patching.h"

typedef struct DT_PATCH
{
     // next link in the linked list
     struct DT_PATCH *next;

     // actual code to replace target code
     char *patch;

     // replaced code is stored here
     char *replacedCode;

     // address of the target routine
     unsigned int targetDriverRoutineAddress;

     // size of the code to replace target code
     int patchSize;

     // list of operands
     int *displacedOperands;

     // size of displaced operands
     int displacedOperandsSize;

     // where we want to restore control to
     int originalRoutineRestorePoint;

     // whether or not we care about the operands (if we fully circumvent the call, we dont)
     int useDisplacedOperands;
};

static struct DT_PATCH *gDTPatches;
static char gDevPath[5]  = "/dev/";

static void dt_detour_patching_remove_patch(struct DT_PATCH *patch)
{
    struct DT_PATCH *cursor;

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
            if (cursor->patch != NULL)
            {
                kfree(cursor->patch);
            }

            if (cursor->replacedCode != NULL)
            {
                kfree(cursor->replacedCode);
            }

            kfree(cursor);
            break;
        }
    }
    return;
}

static void dt_detour_patching_append_patch(struct DT_PATCH **patch)
{
    if (patch == NULL)
    {
        printk(KERN_WARNING "%s(): patch is null", __FUNCTION__);
        return;
    }

    if (*patch != NULL)
    {
        printk(KERN_NOTICE "%s(): patch needs to be removed and reallocated", __FUNCTION__);
        dt_detour_patching_remove_patch(*pactch);
    }

    (*patch) = kmalloc(sizeof(**patch), GFP_KERNEL);

    if (*patch != NULL)
    {
        (*patch)->patch = kmalloc((*patch)->size, GFP_KERNEL);
        (*patch)->replacedCode = kmalloc((*patch)->size, GFP_KERNEL);
        (*patch)->next = gDTPatches;
        gDTPatches = *patch;
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

__declspec(naked) dt_detour_patching_prolog_detour()
{
    int i;
    stuct DT_PATCH *patch;

    // jump to patch
    __asm
    {
        CALL patch->userlandRoutineAddress
    }

    // supplanted operands that needs to get pushed onto the stack
    for(i = 0; patch->useDisplacedOperands && i < patchRequest->displacedOperandsSize; ++i)
    {
        __asm
        {
            PUSH patch->displacedOperands[i]
        }
    }

    // jump to original code
    __asm
    {
         PUSH patch->originalRoutineRestorePoint
         RET
    }
}

static inline int is_valid_patch_request(struct DT_PATCH *patch, DT_PATCH_REQUEST *patchRequest)
{
    if(patch == NULL || patch->patch == NULL || patch->removedCode)
    {
        printk(KERN_WARNING "%s(): allocation failed", __FUNCTION__);
        return 0;
    }

    if (patchRequest == NULL)
    {
        printk(KERN_WARNING "%s(): patchRequest is null", __FUNCTION__);
        return 0;
    }

    if (patchRequest->displacedOperands == NULL)
    {
        printk(KERN_WARNING "%s(): displacedOperands is null", __FUNCTION__);
        return 0;
    }

    return 1;
}

static int dt_detour_patching_apply_patch(unsigned long targetDriverRoutineAddress,
                                          DT_PATCH_REQUEST *patchRequest)
{
    int i;
    char trampolineBytes[TRAMPOLINE_SIZE] = {0x68, 0x00, 0x00, 0x00, 0x00, 0xC3};
    struct DT_PATCH *patch;
    char *targetDriverRoutineAddressAsBytes = (char *)targetDriverRoutineAddress;
    char *userlandRoutineAddressAsBytes = (char *)patchRequest->userlandRoutineAddress;
    dt_detour_patching_append_patch(&patch);

    if(!is_valid_patch_request(patch, patchRequest))
    {
        printk(KERN_WARNING "%s(): need to remove patch", __FUNCTION__);
        dt_detour_patching_remove_patch(patch);
        return -ENOTTY;
    }

    patch->patchSize = patchRequest->bytesRequired;
    patch->displacedOperands = patchRequest->displacedOperands;
    patch->displacedOperandsSize = patchRequest->displacedOperandsSize;
    patch->targetDriverRoutineAddress = targetDriverRoutineAddress;

    for(i = 0; i < patch->patchSize; ++i)
    {
        // save the original code so we can unapply the patch
        patch->replacedCode[i] = targetDriverRoutineAddressAsBytes[i];
        if (i >= TRAMPOLINE_SIZE) // copy noop to pad patch to align with instructions
        {
            patch->patch[i] = 0x90;
        }
        else if (i >= 1 && i <= 4) // write out address of routine we are provided
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
    for(i = 0; i < patch->patchSize; ++i)
    {
        addressAsBytes[i] = patch->replacedCode[i];
    }
    return 0;
}

static inline int is_valid_patch_request(char *path,
                                         DT_PATCH_REQUEST *patchRequest)
{
    if (patch == NULL)
    {
        printk(KERN_WARNING "%s(): patch is null", __FUNCTION__);
        return 0;
    }

    if (patchRequest == NULL)
    {
        printk(KERN_WARNING "%s(): patchRequest is null", __FUNCTION__);
        return 0;
    }

    if (patchRequest->targetDriverName == NULL)
    {
        printk(KERN_WARNING "%s(): targetDriverName is null", __FUNCTION__);
        return 0;
    }

    if (patchRequest->targetRoutineName == NULL)
    {
        printk(KERN_WARNING "%s(): targetRoutineName is null", __FUNCTION__);
        return 0;
    }

    return 1;
}

static int dt_detour_patching_patch_inner(char *path,
                                          DT_PATCH_REQUEST *patchRequest)

{
    unsigned long targetDriverRoutineAddress;

    if (!is_valid_patch_request(patch, patchRequest))
    {
        return EINVAL;
    }

    if (!is_driver_loaded(path, patchRequest->targetDriverName))
    {
        printk(KERN_WARNING "%s(): driver is not loaded", __FUNCTION__);
        return EINVAL;
    }

    targetDriverRoutineAddress = kallsyms_lookup_name(patchRequest->targetRoutineName);
    if (targetDriverRoutineAddress == 0)
    {
        printk(KERN_WARNING "%s(): could not find target routine %s", __FUNCTION__, patchRequest->targetRoutineName);
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
