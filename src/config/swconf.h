//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __SWCONF_H__
#define __SWCONF_H__

#ifndef __INCLUDE_CONFIGS__
#error "swconf.h is for common.h only"
#endif

#if defined(PROJ_OPT)
    #if defined(FERRET_F11) || defined(FERRET_F10P) || defined(FERRET_F5) || defined(FERRET_F4P)
        #if (PROJ_OPT == 99998)
            #include "proj_opt_ferret_release.h"
        #elif (PROJ_OPT == 99999)
            #include "proj_opt_ferret_release_lacp.h"
        #else
            #include "proj_opt_ferret_develop.h"
        #endif
    #endif
#endif // PROJ_OPT

/* Default configuration */
#include "swconf_def.h"

#endif /* __SWCONF_H__ */
