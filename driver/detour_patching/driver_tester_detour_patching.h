#pragma once

#include "../../common/driver_tester_patch_request.h"

void dt_detour_patching_init(void);
void dt_detour_patching_exit(void);
int dt_detour_patching_patch(DT_PATCH_REQUEST *patch_request);
