/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "neonmask-filter.h"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-neonmask", "en-US")
OBS_MODULE_AUTHOR("Mas Ari and contributors")

bool obs_module_load(void)
{
    obs_register_source(&neonmask_filter_info);
    blog(LOG_INFO, "[NeonMask Studio] loaded v%s", NEONMASK_VERSION);
    return true;
}
