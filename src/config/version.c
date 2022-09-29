//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "version.h"
#include "code_revision.c"


/* 
 * Local Definition
 */
/* Software version suffix string */
#if defined(PROJ_OPT)
    #if (PROJ_OPT == 0)
        #define VER_SUFFIX_TXT      " - Engineer Developed"
    #elif (PROJ_OPT == 99998)
        #define VER_SUFFIX_TXT      " Ocelot-REL_Unmanaged"
    #elif (PROJ_OPT == 99999)
        #define VER_SUFFIX_TXT      " Ocelot-REL_Unmanaged_LACP"
    #endif
#else
    #define VER_SUFFIX_TXT          ""
#endif


/* 
 * Local Variables
 */
const char code *SW_COMPILE_DATE = __DATE__ " " __TIME__;

/* Notice that text 'TEST-N' should be excluded from the software version string */
const char code *SW_VERSION =
#if defined(FERRET_F11)
    "Ferret_F11" VER_SUFFIX_TXT
#elif defined(FERRET_F10P)
    "Ferret_F10P" VER_SUFFIX_TXT
#elif defined(FERRET_F5)
    "Ferret_F5" VER_SUFFIX_TXT
#elif defined(FERRET_F4P)
    "Ferret_F4P" VER_SUFFIX_TXT
#else
    "Uknown version"
#endif
;
