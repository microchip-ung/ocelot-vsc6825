//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "timer.h"
#include "print.h"

#include "phydrv.h"
#include "phy_base.h"
#include "phy_family.h"

#define PHY_DEBUG (0)

#if VTSS_TESLA

/************************************************************************/
/* Tesla family functions                                               */
/************************************************************************/


#if VTSS_TESLA_A
// Tesla  Rev. A. Internal 8051 patch.(From James M., 9-27-2011)
// (additional workaround for BZ #5545 and SPI output bug)
// In     : port_no - any port within the chip where to load the 8051 code.
// Return : VTSS_RC_OK if configuration done else error code.

// Download patch into PRAM for the internal 8051
static vtss_rc tesla_revA_8051_patch_9_27_2011(vtss_port_no_t port_no) {

    static code const u8 patch_arr[] = {
        0x44, 0xdd, 0x02, 0x41, 0xff, 0x02, 0x42, 0xc3, 0x02, 0x45,
        0x06, 0x02, 0x45, 0x07, 0x02, 0x45, 0x08, 0x8f, 0x27, 0x7b,
        0xbb, 0x7d, 0x0e, 0x7f, 0x04, 0x12, 0x3c, 0x20, 0xef, 0x4e,
        0x60, 0x03, 0x02, 0x41, 0xe6, 0xe4, 0xf5, 0x19, 0x74, 0x01,
        0x7e, 0x00, 0xa8, 0x19, 0x08, 0x80, 0x05, 0xc3, 0x33, 0xce,
        0x33, 0xce, 0xd8, 0xf9, 0xff, 0xef, 0x55, 0x27, 0x70, 0x03,
        0x02, 0x41, 0xda, 0x85, 0x19, 0xfb, 0x7b, 0xbb, 0xe4, 0xfd,
        0xff, 0x12, 0x3c, 0x20, 0xef, 0x4e, 0x60, 0x03, 0x02, 0x41,
        0xda, 0xe5, 0x19, 0x54, 0x02, 0x75, 0x55, 0x00, 0x25, 0xe0,
        0x25, 0xe0, 0xf5, 0x54, 0xe4, 0x78, 0x93, 0xf6, 0xd2, 0x02,
        0x12, 0x41, 0xe7, 0x7b, 0xff, 0x7d, 0x12, 0x7f, 0x07, 0x12,
        0x3c, 0x20, 0xef, 0x4e, 0x60, 0x03, 0x02, 0x41, 0xd4, 0xc2,
        0x02, 0x74, 0x95, 0x25, 0x19, 0xf9, 0x74, 0xb5, 0x25, 0x19,
        0xf8, 0xe6, 0x27, 0xf5, 0x28, 0xe5, 0x55, 0x24, 0x5b, 0x12,
        0x44, 0x8c, 0x12, 0x3d, 0x48, 0x7b, 0xfc, 0x7d, 0x11, 0x7f,
        0x07, 0x12, 0x3c, 0x20, 0x78, 0x9a, 0xef, 0xf6, 0x78, 0x8f,
        0xe6, 0xfe, 0xef, 0xd3, 0x9e, 0x40, 0x06, 0x78, 0x9a, 0xe6,
        0x78, 0x8f, 0xf6, 0x12, 0x41, 0xe7, 0x7b, 0xec, 0x7d, 0x12,
        0x7f, 0x07, 0x12, 0x3c, 0x20, 0x78, 0x99, 0xef, 0xf6, 0xbf,
        0x07, 0x06, 0x78, 0x91, 0x76, 0x1a, 0x80, 0x1f, 0x78, 0x93,
        0xe6, 0xff, 0x60, 0x0f, 0xc3, 0xe5, 0x28, 0x9f, 0xff, 0x78,
        0x99, 0xe6, 0x85, 0x28, 0xf0, 0xa4, 0x2f, 0x80, 0x07, 0x78,
        0x99, 0xe6, 0x85, 0x28, 0xf0, 0xa4, 0x78, 0x91, 0xf6, 0xe4,
        0x78, 0x90, 0xf6, 0x78, 0x90, 0xe6, 0xff, 0xc3, 0x08, 0x96,
        0x40, 0x03, 0x02, 0x41, 0xbe, 0xef, 0x54, 0x03, 0x60, 0x33,
        0x14, 0x60, 0x46, 0x24, 0xfe, 0x60, 0x42, 0x04, 0x70, 0x4b,
        0xef, 0x24, 0x02, 0xff, 0xe4, 0x33, 0xfe, 0xef, 0x78, 0x02,
        0xce, 0xa2, 0xe7, 0x13, 0xce, 0x13, 0xd8, 0xf8, 0xff, 0xe5,
        0x55, 0x24, 0x5c, 0xcd, 0xe5, 0x54, 0x34, 0xf0, 0xcd, 0x2f,
        0xff, 0xed, 0x3e, 0xfe, 0x12, 0x44, 0xaf, 0x7d, 0x11, 0x80,
        0x0b, 0x78, 0x90, 0xe6, 0x70, 0x04, 0x7d, 0x11, 0x80, 0x02,
        0x7d, 0x12, 0x7f, 0x07, 0x12, 0x3d, 0x08, 0x8e, 0x29, 0x8f,
        0x2a, 0x80, 0x03, 0xe5, 0x29, 0xff, 0x78, 0x93, 0xe6, 0x06,
        0x24, 0x9b, 0xf8, 0xa6, 0x07, 0x78, 0x90, 0x06, 0xe6, 0xb4,
        0x1a, 0x0a, 0xe5, 0x55, 0x24, 0x5c, 0x12, 0x44, 0x8c, 0x12,
        0x3d, 0x48, 0x78, 0x93, 0xe6, 0x65, 0x28, 0x70, 0x82, 0x75,
        0xdb, 0x20, 0x75, 0xdb, 0x28, 0x12, 0x44, 0xa4, 0x12, 0x44,
        0xa4, 0xe5, 0x19, 0x12, 0x44, 0x97, 0xe5, 0x19, 0xc3, 0x13,
        0x12, 0x44, 0x97, 0x78, 0x93, 0x16, 0xe6, 0x24, 0x9b, 0xf8,
        0xe6, 0xff, 0x7e, 0x08, 0x1e, 0xef, 0xa8, 0x06, 0x08, 0x80,
        0x02, 0xc3, 0x13, 0xd8, 0xfc, 0xfd, 0xc4, 0x33, 0x54, 0xe0,
        0xf5, 0xdb, 0xef, 0xa8, 0x06, 0x08, 0x80, 0x02, 0xc3, 0x13,
        0xd8, 0xfc, 0xfd, 0xc4, 0x33, 0x54, 0xe0, 0x44, 0x08, 0xf5,
        0xdb, 0xee, 0x70, 0xd8, 0x78, 0x93, 0xe6, 0x70, 0xc8, 0x75,
        0xdb, 0x10, 0x02, 0x40, 0xea, 0x78, 0x90, 0xe6, 0xc3, 0x94,
        0x17, 0x50, 0x0e, 0xe5, 0x55, 0x24, 0x62, 0x12, 0x41, 0xf5,
        0xe5, 0x55, 0x24, 0x5c, 0x12, 0x41, 0xf5, 0x20, 0x02, 0x03,
        0x02, 0x40, 0x63, 0x05, 0x19, 0xe5, 0x19, 0xc3, 0x94, 0x04,
        0x50, 0x03, 0x02, 0x40, 0x27, 0x22, 0xe5, 0x55, 0x24, 0x5c,
        0xff, 0xe5, 0x54, 0x34, 0xf0, 0xfe, 0x12, 0x44, 0xaf, 0x22,
        0xff, 0xe5, 0x54, 0x34, 0xf0, 0xfe, 0x12, 0x44, 0xaf, 0x22,
        0xd2, 0x00, 0x75, 0xfb, 0x03, 0xab, 0x59, 0xaa, 0x58, 0x7d,
        0x19, 0x7f, 0x03, 0x12, 0x3d, 0x48, 0xe5, 0x59, 0x54, 0x0f,
        0x24, 0xf1, 0x70, 0x03, 0x02, 0x42, 0xb8, 0x24, 0x02, 0x60,
        0x03, 0x02, 0x42, 0xad, 0x12, 0x44, 0xc8, 0x12, 0x44, 0xcf,
        0xd8, 0xfb, 0xff, 0x20, 0xe2, 0x2a, 0x13, 0x92, 0x04, 0xef,
        0xa2, 0xe1, 0x92, 0x03, 0x30, 0x04, 0x1f, 0xe4, 0xf5, 0x21,
        0xe5, 0x21, 0x24, 0x17, 0xfd, 0x7b, 0x54, 0x7f, 0x04, 0x12,
        0x3c, 0x20, 0x74, 0xbd, 0x25, 0x21, 0xf8, 0xa6, 0x07, 0x05,
        0x21, 0xe5, 0x21, 0xc3, 0x94, 0x02, 0x40, 0xe4, 0x12, 0x44,
        0xc8, 0x12, 0x44, 0xcf, 0xd8, 0xfb, 0x54, 0x05, 0x64, 0x04,
        0x70, 0x27, 0x78, 0x92, 0xe6, 0x78, 0x94, 0xf6, 0xe5, 0x58,
        0xff, 0x33, 0x95, 0xe0, 0xef, 0x54, 0x0f, 0x78, 0x92, 0xf6,
        0x12, 0x43, 0x85, 0x20, 0x04, 0x0c, 0x12, 0x44, 0xc8, 0x12,
        0x44, 0xcf, 0xd8, 0xfb, 0x13, 0x92, 0x05, 0x22, 0xc2, 0x05,
        0x22, 0x12, 0x44, 0xc8, 0x12, 0x44, 0xcf, 0xd8, 0xfb, 0x54,
        0x05, 0x64, 0x05, 0x70, 0x1e, 0x78, 0x92, 0x7d, 0xb8, 0x12,
        0x42, 0xb9, 0x78, 0x8f, 0x7d, 0x74, 0x12, 0x42, 0xb9, 0xe4,
        0x78, 0x8f, 0xf6, 0x22, 0x7b, 0x01, 0x7a, 0x00, 0x7d, 0xee,
        0x7f, 0x92, 0x12, 0x37, 0x1c, 0x22, 0xe6, 0xfb, 0x7a, 0x00,
        0x7f, 0x92, 0x12, 0x37, 0x1c, 0x22, 0x30, 0x04, 0x03, 0x12,
        0x44, 0x3d, 0x78, 0x92, 0xe6, 0xff, 0x60, 0x03, 0x12, 0x40,
        0x12, 0xe4, 0xf5, 0x19, 0x12, 0x44, 0xd6, 0x20, 0xe7, 0x03,
        0x02, 0x43, 0x78, 0x85, 0x19, 0xfb, 0x7d, 0x1c, 0xe4, 0xff,
        0x12, 0x3d, 0x08, 0x8e, 0x54, 0x8f, 0x55, 0xe5, 0x54, 0x13,
        0x13, 0x13, 0x54, 0x0c, 0x44, 0x80, 0xf5, 0x1b, 0xe5, 0x54,
        0x54, 0xc0, 0x70, 0x06, 0xe5, 0x55, 0x54, 0x18, 0x60, 0x12,
        0x7e, 0x00, 0xe5, 0x55, 0x54, 0x18, 0x78, 0x03, 0xce, 0xc3,
        0x13, 0xce, 0x13, 0xd8, 0xf9, 0x04, 0x42, 0x1b, 0xe5, 0x1b,
        0x54, 0x03, 0x64, 0x03, 0x70, 0x33, 0x12, 0x44, 0xd6, 0x54,
        0x03, 0x64, 0x03, 0x60, 0x21, 0x75, 0x14, 0xc4, 0x75, 0x15,
        0x06, 0x7d, 0x0a, 0xe4, 0xff, 0x12, 0x3d, 0x08, 0xee, 0x30,
        0xe6, 0x08, 0x43, 0x1b, 0x10, 0x75, 0x16, 0x02, 0x80, 0x2b,
        0x53, 0x1b, 0xef, 0x75, 0x16, 0x03, 0x80, 0x23, 0x12, 0x44,
        0xd6, 0x54, 0x10, 0x42, 0x1b, 0x80, 0x21, 0xe5, 0x1b, 0x54,
        0x03, 0x64, 0x03, 0x60, 0x19, 0x12, 0x44, 0xd6, 0x54, 0x03,
        0xff, 0xbf, 0x03, 0x10, 0x75, 0x14, 0xc4, 0x75, 0x15, 0x06,
        0xe4, 0xf5, 0x16, 0x7f, 0xfe, 0x7e, 0x0e, 0x12, 0x3c, 0x49,
        0x74, 0xb9, 0x25, 0x19, 0xf8, 0xa6, 0x1b, 0x05, 0x19, 0xe5,
        0x19, 0xc3, 0x94, 0x04, 0x50, 0x03, 0x02, 0x42, 0xd5, 0x22,
        0x78, 0x8f, 0xe6, 0xfb, 0x7a, 0x00, 0x7d, 0x74, 0x7f, 0x92,
        0x12, 0x37, 0x1c, 0xe4, 0x78, 0x8f, 0xf6, 0xf5, 0x22, 0x74,
        0x01, 0x7e, 0x00, 0xa8, 0x22, 0x08, 0x80, 0x05, 0xc3, 0x33,
        0xce, 0x33, 0xce, 0xd8, 0xf9, 0xff, 0x78, 0x92, 0xe6, 0xfd,
        0xef, 0x5d, 0x60, 0x44, 0x85, 0x22, 0xfb, 0xe5, 0x22, 0x54,
        0x02, 0x25, 0xe0, 0x25, 0xe0, 0xfe, 0xe4, 0x24, 0x5b, 0xfb,
        0xee, 0x12, 0x44, 0x8f, 0x12, 0x3d, 0x48, 0x7b, 0x40, 0x7d,
        0x11, 0x7f, 0x07, 0x12, 0x3c, 0x20, 0x74, 0x95, 0x25, 0x22,
        0xf8, 0xa6, 0x07, 0x7b, 0x11, 0x7d, 0x12, 0x7f, 0x07, 0x12,
        0x3c, 0x20, 0xef, 0x4e, 0x60, 0x09, 0x74, 0xb5, 0x25, 0x22,
        0xf8, 0x76, 0x04, 0x80, 0x07, 0x74, 0xb5, 0x25, 0x22, 0xf8,
        0x76, 0x0a, 0x05, 0x22, 0xe5, 0x22, 0xc3, 0x94, 0x04, 0x40,
        0x9a, 0x78, 0x94, 0xe6, 0x70, 0x15, 0x78, 0x92, 0xe6, 0x60,
        0x10, 0x75, 0xd9, 0x38, 0x75, 0xdb, 0x10, 0x7d, 0xfe, 0x12,
        0x44, 0x33, 0x7d, 0x76, 0x12, 0x44, 0x33, 0x79, 0x94, 0xe7,
        0x78, 0x92, 0x66, 0xff, 0x60, 0x03, 0x12, 0x40, 0x12, 0x78,
        0x92, 0xe6, 0x70, 0x09, 0xfb, 0xfa, 0x7d, 0xfe, 0x7f, 0x8e,
        0x12, 0x37, 0x1c, 0x22, 0x7b, 0x01, 0x7a, 0x00, 0x7f, 0x8e,
        0x12, 0x37, 0x1c, 0x22, 0xe4, 0xf5, 0x4b, 0x74, 0xbd, 0x25,
        0x4b, 0xf8, 0xe6, 0x64, 0x03, 0x60, 0x38, 0xe5, 0x4b, 0x24,
        0x17, 0xfd, 0x7b, 0xeb, 0x7f, 0x04, 0x12, 0x3c, 0x20, 0x8f,
        0x19, 0x85, 0x19, 0xfb, 0x7b, 0x22, 0x7d, 0x18, 0x7f, 0x06,
        0x12, 0x3c, 0x20, 0xef, 0x64, 0x01, 0x4e, 0x70, 0x07, 0x12,
        0x44, 0xed, 0x7b, 0x03, 0x80, 0x0a, 0x12, 0x44, 0xed, 0x74,
        0xbd, 0x25, 0x4b, 0xf8, 0xe6, 0xfb, 0x7a, 0x00, 0x7d, 0x54,
        0x12, 0x37, 0x1c, 0x05, 0x4b, 0xe5, 0x4b, 0xc3, 0x94, 0x02,
        0x40, 0xb5, 0x22, 0xfb, 0xe5, 0x54, 0x34, 0xf0, 0xfa, 0x7d,
        0x10, 0x7f, 0x07, 0x22, 0x54, 0x01, 0xc4, 0x33, 0x54, 0xe0,
        0xf5, 0xdb, 0x44, 0x08, 0xf5, 0xdb, 0x22, 0xf5, 0xdb, 0x75,
        0xdb, 0x08, 0xf5, 0xdb, 0x75, 0xdb, 0x08, 0x22, 0xab, 0x07,
        0xaa, 0x06, 0x7d, 0x10, 0x7f, 0x07, 0x12, 0x3d, 0x48, 0x7b,
        0xff, 0x7d, 0x10, 0x7f, 0x07, 0x12, 0x3c, 0x20, 0xef, 0x4e,
        0x60, 0xf3, 0x22, 0xe5, 0x59, 0xae, 0x58, 0x78, 0x04, 0x22,
        0xce, 0xa2, 0xe7, 0x13, 0xce, 0x13, 0x22, 0x74, 0xb9, 0x25,
        0x19, 0xf8, 0xe6, 0x22, 0x7f, 0x04, 0x78, 0xb9, 0x74, 0x88,
        0xf6, 0x08, 0xdf, 0xfc, 0x12, 0x45, 0x01, 0x02, 0x44, 0xf7,
        0xe5, 0x4b, 0x24, 0x17, 0x54, 0x1f, 0x44, 0x80, 0xff, 0x22,
        0xe4, 0x78, 0x92, 0xf6, 0xc2, 0x05, 0x78, 0x8f, 0xf6, 0x22,
        0xc2, 0x04, 0xc2, 0x03, 0x22, 0x22, 0x22, 0x22
    };

#if PHY_DEBUG
    println_str("tesla_revA_8051_patch_9_27_2011");
#endif

    VTSS_RC(atom_download_8051_code(port_no, &patch_arr[0], sizeof(patch_arr)));

    VTSS_RC(vtss_phy_wr        (port_no, 31, 0x0010));  // GPIO page
    VTSS_RC(vtss_phy_wr        (port_no,  0, 0x4098));  // Enable 8051 clock; indicate patch present; disable PRAM clock override and addr. auto-incr; operate at 125 MHz
    VTSS_RC(vtss_phy_wr        (port_no,  0, 0xc098));  // Release 8051 SW Reset

    // Check that code is downloaded correctly.
    VTSS_RC(vtss_phy_is_8051_crc_ok_private(port_no,
                                            FIRMWARE_START_ADDR,
                                            sizeof(patch_arr) + 1 /* Add one for the byte auto-added in the download function */,
                                            0x4EE4));

    return VTSS_RC_OK;
}
#endif // VTSS_TESLA_A


#if VTSS_TESLA_B || VTSS_TESLA_D
// Tesla Rev B and D, so far share the same micro code.

// Tesla  Rev. B. Internal 8051 patch.
// In     : port_no - any port within the chip where to load the 8051 code.
// Return : VTSS_RC_OK if configuration done else error code.

// Download patch into PRAM for the internal 8051
// Date: 3Oct2013
static vtss_rc tesla_revB_8051_patch(vtss_port_no_t port_no)
{
    static code const u8 patch_arr[] = {
        0x46, 0x18, 0x02, 0x43, 0x37, 0x02, 0x45, 0xf4, 0x02, 0x46,
        0x45, 0x02, 0x45, 0xbe, 0x02, 0x45, 0x60, 0xed, 0xff, 0xe5,
        0xfc, 0x54, 0x38, 0x64, 0x20, 0x70, 0x08, 0x65, 0xff, 0x70,
        0x04, 0xed, 0x44, 0x80, 0xff, 0x22, 0x8f, 0x19, 0x7b, 0xbb,
        0x7d, 0x0e, 0x7f, 0x04, 0x12, 0x3d, 0xd7, 0xef, 0x4e, 0x60,
        0x03, 0x02, 0x41, 0xf9, 0xe4, 0xf5, 0x1a, 0x74, 0x01, 0x7e,
        0x00, 0xa8, 0x1a, 0x08, 0x80, 0x05, 0xc3, 0x33, 0xce, 0x33,
        0xce, 0xd8, 0xf9, 0xff, 0xef, 0x55, 0x19, 0x70, 0x03, 0x02,
        0x41, 0xed, 0x85, 0x1a, 0xfb, 0x7b, 0xbb, 0xe4, 0xfd, 0xff,
        0x12, 0x3d, 0xd7, 0xef, 0x4e, 0x60, 0x03, 0x02, 0x41, 0xed,
        0xe5, 0x1a, 0x54, 0x02, 0x75, 0x1d, 0x00, 0x25, 0xe0, 0x25,
        0xe0, 0xf5, 0x1c, 0xe4, 0x78, 0xc5, 0xf6, 0xd2, 0x0a, 0x12,
        0x41, 0xfa, 0x7b, 0xff, 0x7d, 0x12, 0x7f, 0x07, 0x12, 0x3d,
        0xd7, 0xef, 0x4e, 0x60, 0x03, 0x02, 0x41, 0xe7, 0xc2, 0x0a,
        0x74, 0xc7, 0x25, 0x1a, 0xf9, 0x74, 0xe7, 0x25, 0x1a, 0xf8,
        0xe6, 0x27, 0xf5, 0x1b, 0xe5, 0x1d, 0x24, 0x5b, 0x12, 0x45,
        0x9b, 0x12, 0x3e, 0xda, 0x7b, 0xfc, 0x7d, 0x11, 0x7f, 0x07,
        0x12, 0x3d, 0xd7, 0x78, 0xcc, 0xef, 0xf6, 0x78, 0xc1, 0xe6,
        0xfe, 0xef, 0xd3, 0x9e, 0x40, 0x06, 0x78, 0xcc, 0xe6, 0x78,
        0xc1, 0xf6, 0x12, 0x41, 0xfa, 0x7b, 0xec, 0x7d, 0x12, 0x7f,
        0x07, 0x12, 0x3d, 0xd7, 0x78, 0xcb, 0xef, 0xf6, 0xbf, 0x07,
        0x06, 0x78, 0xc3, 0x76, 0x1a, 0x80, 0x1f, 0x78, 0xc5, 0xe6,
        0xff, 0x60, 0x0f, 0xc3, 0xe5, 0x1b, 0x9f, 0xff, 0x78, 0xcb,
        0xe6, 0x85, 0x1b, 0xf0, 0xa4, 0x2f, 0x80, 0x07, 0x78, 0xcb,
        0xe6, 0x85, 0x1b, 0xf0, 0xa4, 0x78, 0xc3, 0xf6, 0xe4, 0x78,
        0xc2, 0xf6, 0x78, 0xc2, 0xe6, 0xff, 0xc3, 0x08, 0x96, 0x40,
        0x03, 0x02, 0x41, 0xd1, 0xef, 0x54, 0x03, 0x60, 0x33, 0x14,
        0x60, 0x46, 0x24, 0xfe, 0x60, 0x42, 0x04, 0x70, 0x4b, 0xef,
        0x24, 0x02, 0xff, 0xe4, 0x33, 0xfe, 0xef, 0x78, 0x02, 0xce,
        0xa2, 0xe7, 0x13, 0xce, 0x13, 0xd8, 0xf8, 0xff, 0xe5, 0x1d,
        0x24, 0x5c, 0xcd, 0xe5, 0x1c, 0x34, 0xf0, 0xcd, 0x2f, 0xff,
        0xed, 0x3e, 0xfe, 0x12, 0x45, 0xdb, 0x7d, 0x11, 0x80, 0x0b,
        0x78, 0xc2, 0xe6, 0x70, 0x04, 0x7d, 0x11, 0x80, 0x02, 0x7d,
        0x12, 0x7f, 0x07, 0x12, 0x3e, 0x9a, 0x8e, 0x1e, 0x8f, 0x1f,
        0x80, 0x03, 0xe5, 0x1e, 0xff, 0x78, 0xc5, 0xe6, 0x06, 0x24,
        0xcd, 0xf8, 0xa6, 0x07, 0x78, 0xc2, 0x06, 0xe6, 0xb4, 0x1a,
        0x0a, 0xe5, 0x1d, 0x24, 0x5c, 0x12, 0x45, 0x9b, 0x12, 0x3e,
        0xda, 0x78, 0xc5, 0xe6, 0x65, 0x1b, 0x70, 0x82, 0x75, 0xdb,
        0x20, 0x75, 0xdb, 0x28, 0x12, 0x45, 0xb3, 0x12, 0x45, 0xb3,
        0xe5, 0x1a, 0x12, 0x45, 0xa6, 0xe5, 0x1a, 0xc3, 0x13, 0x12,
        0x45, 0xa6, 0x78, 0xc5, 0x16, 0xe6, 0x24, 0xcd, 0xf8, 0xe6,
        0xff, 0x7e, 0x08, 0x1e, 0xef, 0xa8, 0x06, 0x08, 0x80, 0x02,
        0xc3, 0x13, 0xd8, 0xfc, 0xfd, 0xc4, 0x33, 0x54, 0xe0, 0xf5,
        0xdb, 0xef, 0xa8, 0x06, 0x08, 0x80, 0x02, 0xc3, 0x13, 0xd8,
        0xfc, 0xfd, 0xc4, 0x33, 0x54, 0xe0, 0x44, 0x08, 0xf5, 0xdb,
        0xee, 0x70, 0xd8, 0x78, 0xc5, 0xe6, 0x70, 0xc8, 0x75, 0xdb,
        0x10, 0x02, 0x40, 0xfd, 0x78, 0xc2, 0xe6, 0xc3, 0x94, 0x17,
        0x50, 0x0e, 0xe5, 0x1d, 0x24, 0x62, 0x12, 0x42, 0x08, 0xe5,
        0x1d, 0x24, 0x5c, 0x12, 0x42, 0x08, 0x20, 0x0a, 0x03, 0x02,
        0x40, 0x76, 0x05, 0x1a, 0xe5, 0x1a, 0xc3, 0x94, 0x04, 0x50,
        0x03, 0x02, 0x40, 0x3a, 0x22, 0xe5, 0x1d, 0x24, 0x5c, 0xff,
        0xe5, 0x1c, 0x34, 0xf0, 0xfe, 0x12, 0x45, 0xdb, 0x22, 0xff,
        0xe5, 0x1c, 0x34, 0xf0, 0xfe, 0x12, 0x45, 0xdb, 0x22, 0xe4,
        0xf5, 0x19, 0x12, 0x46, 0x11, 0x20, 0xe7, 0x1e, 0x7b, 0xfe,
        0x12, 0x42, 0xf9, 0xef, 0xc4, 0x33, 0x33, 0x54, 0xc0, 0xff,
        0xc0, 0x07, 0x7b, 0x54, 0x12, 0x42, 0xf9, 0xd0, 0xe0, 0x4f,
        0xff, 0x74, 0x2a, 0x25, 0x19, 0xf8, 0xa6, 0x07, 0x12, 0x46,
        0x11, 0x20, 0xe7, 0x03, 0x02, 0x42, 0xdf, 0x54, 0x03, 0x64,
        0x03, 0x70, 0x03, 0x02, 0x42, 0xcf, 0x7b, 0xcb, 0x12, 0x43,
        0x2c, 0x8f, 0xfb, 0x7b, 0x30, 0x7d, 0x03, 0xe4, 0xff, 0x12,
        0x3d, 0xd7, 0xc3, 0xef, 0x94, 0x02, 0xee, 0x94, 0x00, 0x50,
        0x2a, 0x12, 0x42, 0xec, 0xef, 0x4e, 0x70, 0x23, 0x12, 0x43,
        0x04, 0x60, 0x0a, 0x12, 0x43, 0x12, 0x70, 0x0c, 0x12, 0x43,
        0x1f, 0x70, 0x07, 0x12, 0x46, 0x07, 0x7b, 0x03, 0x80, 0x07,
        0x12, 0x46, 0x07, 0x12, 0x46, 0x11, 0xfb, 0x7a, 0x00, 0x7d,
        0x54, 0x80, 0x3e, 0x12, 0x42, 0xec, 0xef, 0x4e, 0x70, 0x24,
        0x12, 0x43, 0x04, 0x60, 0x0a, 0x12, 0x43, 0x12, 0x70, 0x0f,
        0x12, 0x43, 0x1f, 0x70, 0x0a, 0x12, 0x46, 0x07, 0xe4, 0xfb,
        0xfa, 0x7d, 0xee, 0x80, 0x1e, 0x12, 0x46, 0x07, 0x7b, 0x01,
        0x7a, 0x00, 0x7d, 0xee, 0x80, 0x13, 0x12, 0x46, 0x07, 0x12,
        0x46, 0x11, 0x54, 0x40, 0xfe, 0xc4, 0x13, 0x13, 0x54, 0x03,
        0xfb, 0x7a, 0x00, 0x7d, 0xee, 0x12, 0x38, 0xbd, 0x7b, 0xff,
        0x12, 0x43, 0x2c, 0xef, 0x4e, 0x70, 0x07, 0x74, 0x2a, 0x25,
        0x19, 0xf8, 0xe4, 0xf6, 0x05, 0x19, 0xe5, 0x19, 0xc3, 0x94,
        0x02, 0x50, 0x03, 0x02, 0x42, 0x15, 0x22, 0xe5, 0x19, 0x24,
        0x17, 0xfd, 0x7b, 0x20, 0x7f, 0x04, 0x12, 0x3d, 0xd7, 0x22,
        0xe5, 0x19, 0x24, 0x17, 0xfd, 0x7f, 0x04, 0x12, 0x3d, 0xd7,
        0x22, 0x7b, 0x22, 0x7d, 0x18, 0x7f, 0x06, 0x12, 0x3d, 0xd7,
        0xef, 0x64, 0x01, 0x4e, 0x22, 0x7d, 0x1c, 0xe4, 0xff, 0x12,
        0x3e, 0x9a, 0xef, 0x54, 0x1b, 0x64, 0x0a, 0x22, 0x7b, 0xcc,
        0x7d, 0x10, 0xff, 0x12, 0x3d, 0xd7, 0xef, 0x64, 0x01, 0x4e,
        0x22, 0xe5, 0x19, 0x24, 0x17, 0xfd, 0x7f, 0x04, 0x12, 0x3d,
        0xd7, 0x22, 0xd2, 0x08, 0x75, 0xfb, 0x03, 0xab, 0x7e, 0xaa,
        0x7d, 0x7d, 0x19, 0x7f, 0x03, 0x12, 0x3e, 0xda, 0xe5, 0x7e,
        0x54, 0x0f, 0x24, 0xf3, 0x60, 0x03, 0x02, 0x43, 0xe9, 0x12,
        0x46, 0x28, 0x12, 0x46, 0x2f, 0xd8, 0xfb, 0xff, 0x20, 0xe2,
        0x35, 0x13, 0x92, 0x0c, 0xef, 0xa2, 0xe1, 0x92, 0x0b, 0x30,
        0x0c, 0x2a, 0xe4, 0xf5, 0x10, 0x7b, 0xfe, 0x12, 0x43, 0xff,
        0xef, 0xc4, 0x33, 0x33, 0x54, 0xc0, 0xff, 0xc0, 0x07, 0x7b,
        0x54, 0x12, 0x43, 0xff, 0xd0, 0xe0, 0x4f, 0xff, 0x74, 0x2a,
        0x25, 0x10, 0xf8, 0xa6, 0x07, 0x05, 0x10, 0xe5, 0x10, 0xc3,
        0x94, 0x02, 0x40, 0xd9, 0x12, 0x46, 0x28, 0x12, 0x46, 0x2f,
        0xd8, 0xfb, 0x54, 0x05, 0x64, 0x04, 0x70, 0x27, 0x78, 0xc4,
        0xe6, 0x78, 0xc6, 0xf6, 0xe5, 0x7d, 0xff, 0x33, 0x95, 0xe0,
        0xef, 0x54, 0x0f, 0x78, 0xc4, 0xf6, 0x12, 0x44, 0x0a, 0x20,
        0x0c, 0x0c, 0x12, 0x46, 0x28, 0x12, 0x46, 0x2f, 0xd8, 0xfb,
        0x13, 0x92, 0x0d, 0x22, 0xc2, 0x0d, 0x22, 0x12, 0x46, 0x28,
        0x12, 0x46, 0x2f, 0xd8, 0xfb, 0x54, 0x05, 0x64, 0x05, 0x70,
        0x1e, 0x78, 0xc4, 0x7d, 0xb8, 0x12, 0x43, 0xf5, 0x78, 0xc1,
        0x7d, 0x74, 0x12, 0x43, 0xf5, 0xe4, 0x78, 0xc1, 0xf6, 0x22,
        0x7b, 0x01, 0x7a, 0x00, 0x7d, 0xee, 0x7f, 0x92, 0x12, 0x38,
        0xbd, 0x22, 0xe6, 0xfb, 0x7a, 0x00, 0x7f, 0x92, 0x12, 0x38,
        0xbd, 0x22, 0xe5, 0x10, 0x24, 0x17, 0xfd, 0x7f, 0x04, 0x12,
        0x3d, 0xd7, 0x22, 0x78, 0xc1, 0xe6, 0xfb, 0x7a, 0x00, 0x7d,
        0x74, 0x7f, 0x92, 0x12, 0x38, 0xbd, 0xe4, 0x78, 0xc1, 0xf6,
        0xf5, 0x11, 0x74, 0x01, 0x7e, 0x00, 0xa8, 0x11, 0x08, 0x80,
        0x05, 0xc3, 0x33, 0xce, 0x33, 0xce, 0xd8, 0xf9, 0xff, 0x78,
        0xc4, 0xe6, 0xfd, 0xef, 0x5d, 0x60, 0x44, 0x85, 0x11, 0xfb,
        0xe5, 0x11, 0x54, 0x02, 0x25, 0xe0, 0x25, 0xe0, 0xfe, 0xe4,
        0x24, 0x5b, 0xfb, 0xee, 0x12, 0x45, 0x9e, 0x12, 0x3e, 0xda,
        0x7b, 0x40, 0x7d, 0x11, 0x7f, 0x07, 0x12, 0x3d, 0xd7, 0x74,
        0xc7, 0x25, 0x11, 0xf8, 0xa6, 0x07, 0x7b, 0x11, 0x7d, 0x12,
        0x7f, 0x07, 0x12, 0x3d, 0xd7, 0xef, 0x4e, 0x60, 0x09, 0x74,
        0xe7, 0x25, 0x11, 0xf8, 0x76, 0x04, 0x80, 0x07, 0x74, 0xe7,
        0x25, 0x11, 0xf8, 0x76, 0x0a, 0x05, 0x11, 0xe5, 0x11, 0xc3,
        0x94, 0x04, 0x40, 0x9a, 0x78, 0xc6, 0xe6, 0x70, 0x15, 0x78,
        0xc4, 0xe6, 0x60, 0x10, 0x75, 0xd9, 0x38, 0x75, 0xdb, 0x10,
        0x7d, 0xfe, 0x12, 0x44, 0xb8, 0x7d, 0x76, 0x12, 0x44, 0xb8,
        0x79, 0xc6, 0xe7, 0x78, 0xc4, 0x66, 0xff, 0x60, 0x03, 0x12,
        0x40, 0x25, 0x78, 0xc4, 0xe6, 0x70, 0x09, 0xfb, 0xfa, 0x7d,
        0xfe, 0x7f, 0x8e, 0x12, 0x38, 0xbd, 0x22, 0x7b, 0x01, 0x7a,
        0x00, 0x7f, 0x8e, 0x12, 0x38, 0xbd, 0x22, 0xe4, 0xf5, 0xfb,
        0x7d, 0x1c, 0xe4, 0xff, 0x12, 0x3e, 0x9a, 0xad, 0x07, 0xac,
        0x06, 0xec, 0x54, 0xc0, 0xff, 0xed, 0x54, 0x3f, 0x4f, 0xf5,
        0x20, 0x30, 0x06, 0x2c, 0x30, 0x01, 0x08, 0xa2, 0x04, 0x72,
        0x03, 0x92, 0x07, 0x80, 0x21, 0x30, 0x04, 0x06, 0x7b, 0xcc,
        0x7d, 0x11, 0x80, 0x0d, 0x30, 0x03, 0x06, 0x7b, 0xcc, 0x7d,
        0x10, 0x80, 0x04, 0x7b, 0x66, 0x7d, 0x16, 0xe4, 0xff, 0x12,
        0x3d, 0xd7, 0xee, 0x4f, 0x24, 0xff, 0x92, 0x07, 0xaf, 0xfb,
        0x74, 0x26, 0x2f, 0xf8, 0xe6, 0xff, 0xa6, 0x20, 0x20, 0x07,
        0x39, 0x8f, 0x20, 0x30, 0x07, 0x34, 0x30, 0x00, 0x31, 0x20,
        0x04, 0x2e, 0x20, 0x03, 0x2b, 0xe4, 0xf5, 0xff, 0x75, 0xfc,
        0xc2, 0xe5, 0xfc, 0x30, 0xe0, 0xfb, 0xaf, 0xfe, 0xef, 0x20,
        0xe3, 0x1a, 0xae, 0xfd, 0x44, 0x08, 0xf5, 0xfe, 0x75, 0xfc,
        0x80, 0xe5, 0xfc, 0x30, 0xe0, 0xfb, 0x8f, 0xfe, 0x8e, 0xfd,
        0x75, 0xfc, 0x80, 0xe5, 0xfc, 0x30, 0xe0, 0xfb, 0x05, 0xfb,
        0xaf, 0xfb, 0xef, 0xc3, 0x94, 0x04, 0x50, 0x03, 0x02, 0x44,
        0xc5, 0xe4, 0xf5, 0xfb, 0x22, 0xe5, 0x7e, 0x30, 0xe5, 0x35,
        0x30, 0xe4, 0x0b, 0x7b, 0x02, 0x7d, 0x33, 0x7f, 0x35, 0x12,
        0x36, 0x29, 0x80, 0x10, 0x7b, 0x01, 0x7d, 0x33, 0x7f, 0x35,
        0x12, 0x36, 0x29, 0x90, 0x47, 0xd2, 0xe0, 0x44, 0x04, 0xf0,
        0x90, 0x47, 0xd2, 0xe0, 0x54, 0xf7, 0xf0, 0x90, 0x47, 0xd1,
        0xe0, 0x44, 0x10, 0xf0, 0x7b, 0x05, 0x7d, 0x84, 0x7f, 0x86,
        0x12, 0x36, 0x29, 0x22, 0xfb, 0xe5, 0x1c, 0x34, 0xf0, 0xfa,
        0x7d, 0x10, 0x7f, 0x07, 0x22, 0x54, 0x01, 0xc4, 0x33, 0x54,
        0xe0, 0xf5, 0xdb, 0x44, 0x08, 0xf5, 0xdb, 0x22, 0xf5, 0xdb,
        0x75, 0xdb, 0x08, 0xf5, 0xdb, 0x75, 0xdb, 0x08, 0x22, 0xe5,
        0x7e, 0x54, 0x0f, 0x64, 0x01, 0x70, 0x0d, 0xe5, 0x7e, 0x30,
        0xe4, 0x08, 0x90, 0x47, 0xd0, 0xe0, 0x44, 0x02, 0xf0, 0x22,
        0x90, 0x47, 0xd0, 0xe0, 0x54, 0xfd, 0xf0, 0x22, 0xab, 0x07,
        0xaa, 0x06, 0x7d, 0x10, 0x7f, 0x07, 0x12, 0x3e, 0xda, 0x7b,
        0xff, 0x7d, 0x10, 0x7f, 0x07, 0x12, 0x3d, 0xd7, 0xef, 0x4e,
        0x60, 0xf3, 0x22, 0x12, 0x44, 0xc2, 0x30, 0x0c, 0x03, 0x12,
        0x42, 0x12, 0x78, 0xc4, 0xe6, 0xff, 0x60, 0x03, 0x12, 0x40,
        0x25, 0x22, 0xe5, 0x19, 0x24, 0x17, 0x54, 0x1f, 0x44, 0x80,
        0xff, 0x22, 0x74, 0x2a, 0x25, 0x19, 0xf8, 0xe6, 0x22, 0x12,
        0x46, 0x40, 0x12, 0x46, 0x36, 0x90, 0x47, 0xfa, 0xe0, 0x54,
        0xf8, 0x44, 0x02, 0xf0, 0x22, 0xe5, 0x7e, 0xae, 0x7d, 0x78,
        0x04, 0x22, 0xce, 0xa2, 0xe7, 0x13, 0xce, 0x13, 0x22, 0xe4,
        0x78, 0xc4, 0xf6, 0xc2, 0x0d, 0x78, 0xc1, 0xf6, 0x22, 0xc2,
        0x0c, 0xc2, 0x0b, 0x22, 0x22
    };

#if PHY_DEBUG
    println_str("tesla_revB_8051_patch");
#endif

    VTSS_RC(atom_download_8051_code(port_no, &patch_arr[0], sizeof(patch_arr)));

    VTSS_RC(vtss_phy_wr        (port_no, 31, 0x0010));     // GPIO page
    VTSS_RC(vtss_phy_wr        (port_no,  3, 0x3eb7));     // Trap ROM at _MicroSmiRead+0x1d to spoof patch-presence
    VTSS_RC(vtss_phy_wr        (port_no,  4, 0x4012));     // Branch to starting address of SpoofPatchPresence
    VTSS_RC(vtss_phy_wr        (port_no, 12, 0x0100));     // Enable patch fram trap described in register 3-4
    VTSS_RC(vtss_phy_wr        (port_no,  0, 0x4018));     // Enable 8051 clock; no longer indicate patch present; disable PRAM clock override and addr. auto-incr; operate at 125 MHz
    VTSS_RC(vtss_phy_wr        (port_no,  0, 0xc018));     // Release 8051 SW Reset

    VTSS_RC(vtss_phy_wr        (port_no, 31, 0x0));        // STD page

    // Check that code is downloaded correctly.
    VTSS_RC(vtss_phy_is_8051_crc_ok_private(port_no,
                                            FIRMWARE_START_ADDR,
                                            sizeof(patch_arr) + 1, // Add one for the byte auto-added in the download function
                                            0x2BB0));
    return VTSS_RC_OK;
}
#endif // VTSS_TESLA_B || VTSS_TESLA_D


#if VTSS_TESLA_B || VTSS_TESLA_D
static vtss_rc vtss_phy_pre_init_tesla_revB_1588(vtss_port_no_t port_no)
{
#if PHY_DEBUG
    println_str("vtss_phy_pre_init_tesla_revB_1588");
#endif

    /* Pass the cmd to Micro to initialize all 1588 analyzer registers to default */
    VTSS_RC(vtss_phy_wr        (port_no, 31, 0x0010));     // GPIO page
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x801A));
    /* Poll on 18G.15 to clear */
    VTSS_RC(vtss_phy_wait_for_micro_complete(port_no));
    /* return to standard page */
    VTSS_RC(vtss_phy_wr        (port_no, 31, 0x0));        // STD page

    return VTSS_RC_OK;
}
#endif // VTSS_TESLA_B || VTSS_TESLA_D


#if VTSS_TESLA_B
// Initialization needed for Tesla. (For whole chip and must be done before init. of the individual ports)
//
// In : port_no : Port number (MUST be the first port for the chip)..
//
// Return : VTSS_RC_OK if configuration done else error code.
// Based on matlab init_script.m,  Revision 1.13  2011/08/17 15:20:37  jimb
// Calls James McIntosh' latest patch for the 1588 SPI issue dated 9/12/2011
// On 27-09-2012 - updated to address bugzilla #9871 and #9886

static vtss_rc vtss_phy_pre_init_seq_tesla_17_august_2011 (vtss_port_no_t port_no)
{
#if PHY_DEBUG
    println_str("vtss_phy_pre_init_seq_tesla_17_august_2011");
#endif

    // MII register writes and test-page register writes go here
    //using broadcast flag to speed things up
    VTSS_RC(vtss_phy_wr        (port_no, 31, 0x0));        // STD page
    VTSS_RC(vtss_phy_wr_masked (port_no, 22, 0x0001, 0x0001)); //turn on broadcast writes

    VTSS_RC(vtss_phy_wr        (port_no, 31, 0x2A30));        //Switch to test register page
    VTSS_RC(vtss_phy_wr_masked (port_no,  8, 0x8000, 0x8000)); //Enable token-ring during coma-mode
    VTSS_RC(vtss_phy_wr        (port_no, 31, 0x52B5));      // Switch to token-ring register page

    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x1));

    /* It is found that the settings commented out below can give a few frames with CRC (one every 1-2 hours), so
       they are left out. The settings are related to EEE, and this may give some EEE problems, but they are going to fixed in chip rev. C.
          VTSS_RC(vtss_phy_wr        (port_no, 17, 0x9fa2));
          VTSS_RC(vtss_phy_wr        (port_no, 16, 0x968a));

          VTSS_RC(vtss_phy_wr        (port_no, 18, 0xd2));
          VTSS_RC(vtss_phy_wr        (port_no, 17, 0x545a));
          VTSS_RC(vtss_phy_wr        (port_no, 16, 0x968c)); */

    // Fix for bugzilla#9871 (Atheros EEE interop) and bugzilla#9886 (1000BT too long to link up)
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0004));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x01bd));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8fae));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x000f));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x000f));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8fac));


    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x4));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0xe004));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x96a8));

    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x7200));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x96a2));

    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x5d));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0xdddd));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8ee0));

    VTSS_RC(vtss_phy_wr        (port_no, 18, 0xee));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0xffff));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x96a0));

    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x3f));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x5e48));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x96a6));

    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x2a));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x2d2e));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x9794));

    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x7));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x150));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8fe0));

    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x7000));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x96b2));

    // Improve 100BASE-TX link startup robustness to address interop issue
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0068));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x8980));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8f90));

    //Note that when Tesla is configured for 10BASE-T (not Te), it needs no 10BASE-T configuration as its pulse codes already match
#ifdef VTSS_10BASE_TE
    VTSS_RC(vtss_phy_65nm_10BASE_init(port_no, 0));
#endif
    VTSS_RC(vtss_phy_wr        (port_no, 31, 0x2A30));        //Switch to test register page
    VTSS_RC(vtss_phy_wr_masked (port_no,  8, 0x0000, 0x8000)); //Disable token-ring after complete


    VTSS_RC(vtss_phy_wr        (port_no, 31, 0x0));        // STD page
    VTSS_RC(vtss_phy_wr_masked (port_no, 22, 0x0000, 0x0001)); // Turn off broadcast writes

    VTSS_RC(tesla_revB_8051_patch(port_no)); //Load micro patch Tesla RevB

    VTSS_RC(vtss_phy_pre_init_tesla_revB_1588(port_no)); //Init 1588 register using Tesla RevB micro patch

    return VTSS_RC_OK;
}
#endif //  VTSS_TESLA_B


#if VTSS_TESLA_A
// Initialization needed for Tesla. (For whole chip and must be done before init. of the individual ports)
//
// In : port_no - Any port with the chip.
//
// Return : VTSS_RC_OK if configuration done else error code.
static vtss_rc vtss_phy_pre_init_seq_tesla_4_april_2011 (vtss_port_no_t port_no) {
#if PHY_DEBUG
    println_str("vtss_phy_pre_init_seq_tesla_4_april_2011");
#endif

    // MII register writes and test-page register writes go here
    //using broadcast flag to speed things up
    VTSS_RC(vtss_phy_wr        (port_no, 31, 0)); //Switch to main register page
    VTSS_RC(vtss_phy_wr_masked (port_no, 22, 0x0001, 0x0001)); // turn on broadcast writes

    // Set 100BASE-TX edge rate to optimal setting
    VTSS_RC(vtss_phy_wr_masked (port_no, 24, 0x2000, 0xe000));

    // Set 100BASE-TX amplitude to optimal setting after MDI-cal tweak
    VTSS_RC(vtss_phy_wr        (port_no, 31, 2)); //Switch to extended register page 2
    VTSS_RC(vtss_phy_wr_masked (port_no, 16, 0x0200, 0x0f00));

    // Set MDI Cal offset 100 to 0
    // Set MDI Cal offset 1000 to 0
    VTSS_RC(vtss_phy_wr        (port_no, 31, 0x2a30)); //Switch to test register page
    VTSS_RC(vtss_phy_wr_masked (port_no, 20, 0x0000, 0x6030));

    //Switch to hardware energy-detector in 1000BASE-T mode
    VTSS_RC(vtss_phy_wr_masked (port_no,  9, 0x0000, 0x4000));

    VTSS_RC(vtss_phy_wr_masked (port_no,  8, 0x8000, 0x8000)); // Enable token-ring during coma-mode

    VTSS_RC(vtss_phy_wr        (port_no, 31, 0x52b5 )); // Switch to token-ring register page

    //TrWrite( base_phy+1, 'eee_TrKp1Short_1000', -1, 0);
    //TrWrite( base_phy+1, 'eee_TrKp2Long_1000', -1, 0);
    //TrWrite( base_phy+1, 'eee_TrKp2Short_1000', -1, 0);
    //TrWrite( base_phy+1, 'eee_TrKp3Long_1000', -1, 1);
    //TrWrite( base_phy+1, 'eee_TrKp3Short_1000', -1, 1);
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000 )); //eee_Tr... Kp1Long,Kp1Short ..._1000
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0011 )); //eee_Tr... Kp2Long,Kp2Short,Kp3Long,Kp3Short ..._1000
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x96a0 )); //Write eee_TrKp*_1000

    //TrWrite( base_phy+1, 'ph_shift_num1000_eee1', -1, 1); //Double-step when Kp1 selected
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000 )); //(no fields)
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x7100 )); //eee_TrKf1000,ph_shift_num1000_... eee1,eee2,std
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x96a2 )); //Write eee_TrKf1000,ph_shift_num1000_*

    //TrWrite( base_phy+1, 'SSTrKf1000Slv',-1, 7); //Disable frequency-offset updates at showtime
    //TrWrite( base_phy+1, 'SSTrKp100',-1, 13); //Speed up steady-state to 2*gain of 1 - UNH setting
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x00d2 )); //SSTrKp100,SSTrKf100
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x547f )); //SSTrKp1000Mas,SSTrKp1000Slv,SSTrKf1000Slv,Ss[EN]cUpdGain1000
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x968c )); //Write SSTrK*,SS[EN]cUpdGain1000

    //TrWrite( base_phy+1, 'eee_TrKp1Short_100',-1, 0);
    //TrWrite( base_phy+1, 'eee_TrKp2Long_100',-1, 15);
    //TrWrite( base_phy+1, 'eee_TrKp2Short_100',-1, 0);
    //TrWrite( base_phy+1, 'eee_TrKp3Short_100',-1, 13);
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x00f0 )); //eee_Tr... Kp1Long,Kp1Short ..._100
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0xf00d )); //eee_Tr... Kp2Long,Kp2Short,Kp3Long,Kp3Short ..._100
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x96b0 )); //Write eee_TrKp*_100

    //TrWrite( base_phy+1, 'ph_shift_num100_eee1', -1, 6); //Quad-step when Kp1 selected
    //TrWrite( base_phy+1, 'ph_shift_num100_eee2', -1, 2); //Double-step when Kp2 selected
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000 )); //(no fields)
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x7100 )); //eee_TrKf100,ph_shift_num100_... eee1,eee2,std
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x96b2 )); //Write eee_TrKf100,ph_shift_num100_*

    //TrWrite( base_phy+1, 'lpi_tr_tmr_val1',-1, '34'); //lengthen tmr1 by 1/2 us
    //TrWrite( base_phy+1, 'lpi_tr_tmr_val2',-1, '5f'); //lengthen tmr2 by 1/2 us
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000 )); //(no fields)
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x345f )); //lpi_tr_tmr_... val1,val2
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x96b4 )); //Write lpi_tr_tmr_val*

    //TrWrite( base_phy+1, 'nonzero_det_thr1000',-1, 15); // Increase from 8 to reduce noise
    //TrWrite( base_phy+1, 'zero_det_thr_nzd1000',-1, 31); // Increase from 15 to handle 1000BT scrambler length in case 31 zeroes OK
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000 )); //nonzero_det_thr1000[5:4]
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0xf7df )); //nonzero_det_thr1000[3:0],zero_det_thr_... nzd1000,zd1000
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8fd4 )); //Write non/zero_det_thr*1000

    //TrWrite( base_phy+1, 'nonzero_det_thr100',-1, 15); // Increase from 8 to reduce noise
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000 )); //nonzero_det_thr100[5:4]
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0xf3df )); //nonzero_det_thr100[3:0],zero_det_thr_... nzd100,zd100
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8fd2 )); //Write non/zero_det_thr*100

    //TrWrite( base_phy+1, 'zero_detect_thresh_zd',-1, 31); // Detect link-partner going quiet more quickly
    //TrWrite( base_phy+1, 'rem_lpi_req_thresh',-1, 15);
    //TrWrite( base_phy+1, 'rem_upd_done_thresh',-1, 15); // Speed up remote update-done detection faster than zero-detection
    //31, 15, and 15 are the default, thus no write

    //TrWrite( base_phy+1, 'LongVgaThresh100',-1, 21); //same as -11 -- UNH setting
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x000e )); //DSPreadyTime100[7:2]
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x2b00 )); //DSPreadyTime100[1:0],LongVgaThresh100,EnabRandUpdTrig,CMAforces(8b)
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8fb0 )); //Write DSPreadyTime100,LongVgaThresh100,EnabRandUpdTrig,CMAforces

    //10BT power level optimization for 10BT MAU

    //TrWrite( base_phy+1, 'ld10_pwrlvl_actiphy', -1, 1);
    //TrWrite( base_phy+1, 'ld10_pwrlvl_10tx', -1, 1);
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x000b )); //SwitchToLD10,PwrUpBothDAC_LD10/G,dac10_keepalive_en
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x05a0 )); //ld10_pwrlvl_... act_ct,g_ct,actiphy,10tx,10rx,100rx,10idle,g
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8fe0 )); //Write SwitchToLD10,PwrUpBoth*,dac10_keepalive_en,ld10_pwrlvl_*

    //TrWrite( base_phy+1, 'ld10_edge_ctrl1', -1, 3);
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000 )); //(no fields)
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x00ba )); //ld10_edge_ctrl... 0,1,2,3
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8fe2 )); //Write ld10_edge_ctrl*

    //TrWrite( base_phy+1, 'VgaGain10',-1, 4);
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000 )); //EnabIndpDfe/Dc/XcUpdTrig,SSFfeDecCntrl,SSEnabFfeUpdTapSel,VgaGain10[4]
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x4689 )); //VgaGain10[3:0],SSVgaSlowDecRate,SSVgaWindow1000/100,SSVgaSerialize
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8f92 )); //Write register containing VgaGain10

    // Improve 100BASE-TX link startup robustness to address interop issue
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0060));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0980));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8f90));

    //Note that when Tesla is configured for 10BASE-T (not Te), it needs no 10BASE-T configuration as its pulse codes already match
#ifdef VTSS_10BASE_TE
    VTSS_RC(vtss_phy_65nm_10BASE_init(iport, 0));
#endif

    VTSS_RC(vtss_phy_wr        (port_no, 31, 0x2a30)); //Switch to test register page
    VTSS_RC(vtss_phy_wr_masked (port_no,  8, 0x0000, 0x8000)); // Disable token-ring after complete


    VTSS_RC(vtss_phy_wr        (port_no, 31, 0 )); // STD page
    VTSS_RC(vtss_phy_wr_masked (port_no, 22, 0x0000, 0x0001)); // Turn off broadcast writes

    VTSS_RC(tesla_revA_8051_patch_9_27_2011(port_no)); //Load micro patch
    return VTSS_RC_OK;
}
#endif // VTSS_TESLA_A


#if VTSS_TESLA_D
// Initialization needed for Tesla. (For whole chip and must be done before init. of the individual ports)
//
// In : port_no : Port number (MUST be the first port for the chip)..
//
// Return : VTSS_RC_OK if configuration done else error code.
// Date : 27-09-2012 - updated to address bugzilla #9871 and #9886
static vtss_rc vtss_phy_pre_init_seq_tesla_rev_d(vtss_port_no_t port_no)
{
#if PHY_DEBUG
    println_str("vtss_phy_pre_init_seq_tesla_rev_d");
#endif

    VTSS_RC(vtss_phy_wr        (port_no, 31, 0 )); // STD page
    VTSS_RC(vtss_phy_wr_masked (port_no, 22, 0x0001, 0x0001));
    VTSS_RC(vtss_phy_wr        (port_no, 24, 0x0040));
    VTSS_RC(vtss_phy_wr        (port_no, 31, 2 )); // EXT2 page
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x02be));
    VTSS_RC(vtss_phy_wr        (port_no, 31, 0x2A30 )); // Test page
    VTSS_RC(vtss_phy_wr        (port_no, 20, 0x4420));
    VTSS_RC(vtss_phy_wr        (port_no, 24, 0x0c00));
    VTSS_RC(vtss_phy_wr        (port_no,  9, 0x18cc));
    VTSS_RC(vtss_phy_wr_masked (port_no,  8, 0x8000, 0x8000));
    VTSS_RC(vtss_phy_wr        (port_no,  5, 0x1320));
    VTSS_RC(vtss_phy_wr        (port_no, 31, 0x52B5 )); // Token ring page
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0004));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x01bd));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8fae));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x000f));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x000f));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8fac));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x00a0));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0xf147));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x97a0));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0005));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x2f54));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8fe4));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0027));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x303d));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x9792));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0704));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x87fe));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0006));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0150));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8fe0));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0012));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x480a));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8f82));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0034));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8f80));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0012));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x82e0));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0005));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0208));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x83a2));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x9186));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x83b2));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x000e));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x3700));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8fb0));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0004));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x9fa0));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x9688));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0xffff));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8fd2));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0003));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x9fa0));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x968a));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0020));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x640b));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x9690));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x2220));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8258));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x2a20));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x825a));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x3060));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x825c));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x3fa0));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x825e));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0xe0f0));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x83a6));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x4489));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8f92));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x7000));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x96a2));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0010));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x2048));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x96a6));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x00ff));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x96a0));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0091));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x9880));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8fe8));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0004));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0xd602));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8fea));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x00ef));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0xef00));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x96b0));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x7100));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x96b2));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x5064));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x96b4));

    // Improve 100BASE-TX link startup robustness to address interop issue
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0068));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x8980));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8f90));

#ifndef VTSS_10BASE_TE
    VTSS_RC(vtss_phy_wr        (port_no, 31, 0x52B5 )); // Token ring page
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0071));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0xf6d9));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8488));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0db6));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x848e));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0059));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x6596));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x849c));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0514));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x849e));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0041));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0280));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84a2));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84a4));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84a6));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84a8));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84aa));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x007d));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0xf7dd));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84ae));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x006d));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x95d4));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84b0));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0049));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x2410));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84b2));
#else //using 10BASE-Te
    VTSS_RC(vtss_phy_wr        (port_no, 31, 2 )); // EXT2 page
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x8000));
    VTSS_RC(vtss_phy_wr        (port_no, 31, 0x52B5 )); // Token ring page
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0008));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0xa499));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8486));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0075));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0xf759));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8488));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0914));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x848a));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x00f7));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0xff7b));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x848c));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0eb9));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x848e));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0061));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x85d6));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8490));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0055));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x44d2));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8492));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0044));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0xa8aa));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8494));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0cb9));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8496));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x00f7));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0xff79));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8498));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0caa));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x849a));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0061));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x8618));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x849c));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0618));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x849e));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0018));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84a0));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0061));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x848a));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84a2));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84a4));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84a6));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84a8));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84aa));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0029));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x265d));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84ac));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x007d));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0xd658));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84ae));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0061));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x8618));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84b0));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0061));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x8618));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84b2));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0061));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x8618));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84b4));
#endif //VTSS_10BASE_TE
    VTSS_RC(vtss_phy_wr        (port_no, 31, 0x2A30 )); // Test page
    VTSS_RC(vtss_phy_wr_masked (port_no,  8, 0x0000, 0x8000));
    VTSS_RC(vtss_phy_wr        (port_no, 31, 0 )); // STD page
    VTSS_RC(vtss_phy_wr_masked (port_no, 22, 0x0000, 0x0001));

    VTSS_RC(tesla_revB_8051_patch(port_no)); // Rev D. uses the same patch as rev B.

    VTSS_RC(vtss_phy_pre_init_tesla_revB_1588(port_no)); //Init 1588 register using Tesla RevB micro patch

    return VTSS_RC_OK;
}
#endif //VTSS_TESLA_D


// Function to be called at startup before the phy ports are reset.
// In : port_no - The phy port number stating from 0.
vtss_rc tesla_init_seq_pre(
        vtss_port_no_t      port_no,
        phy_id_t            *phy_id
) {
#if PHY_DEBUG
    println_str("tesla_init_seq_pre");
#endif

    switch (phy_id->revision)
    {
#if VTSS_TESLA_A
        case VTSS_PHY_TESLA_REV_A:
            VTSS_RC(vtss_phy_pre_init_seq_tesla_4_april_2011(port_no));
            break;
#endif //VTSS_TESLA_A

#if VTSS_TESLA_B
        case VTSS_PHY_TESLA_REV_B:
            VTSS_RC(vtss_phy_pre_init_seq_tesla_17_august_2011(port_no));
            break;
#endif //VTSS_TESLA_B

#if VTSS_TESLA_D
        case VTSS_PHY_TESLA_REV_D:
            VTSS_RC(vtss_phy_pre_init_seq_tesla_rev_d(port_no));
            break;
#endif //VTSS_TESLA_D

        default:
            print_str("Unsupported TESLA revision: ");
            print_dec(phy_id->revision);
            print_cr_lf();
            break;
    }

    return VTSS_RC_OK;
}

// Work-around for issue with Changing speed at one port gives CRC errors at other ports.

// In : Port_no - Port in question
//      Start   - TRUE to start work-around
static vtss_rc vtss_phy_bugzero_48512_workaround(vtss_port_no_t port_no, BOOL start)
{
#if PHY_DEBUG
    println_str("vtss_phy_bugzero_48512_workaround");
#endif

    VTSS_RC(vtss_phy_page_std(port_no));

    if (start) {
        VTSS_RC(vtss_phy_wr(port_no,  0, 0x4040));
    } else {
        VTSS_RC(vtss_phy_wr(port_no,  0, 0x0000));
    }
    return VTSS_RC_OK;
}


/**
 * Determine the MAC interface mode from PHY register.
 */
static vtss_port_interface_t phy_mac_mode_get (
    vtss_port_no_t  port_no
) {
    u16             value;

#if PHY_DEBUG
    println_str("phy_mac_mode_get");
#endif

    VTSS_RC(vtss_phy_page_gpio(port_no));
    VTSS_RC(vtss_phy_rd       (port_no, 19, &value));

    switch (value) {
    case 0x0000:
        return VTSS_PORT_INTERFACE_SGMII;  break;
    case 0x4000:
        return VTSS_PORT_INTERFACE_QSGMII; break;
    case 0x8000:
        return VTSS_PORT_INTERFACE_RGMII;  break;
    }
    return VTSS_PORT_INTERFACE_NO_CONNECTION;
}


/**
 * Detects current PHY MAC interface from PHY register.
 *
 * @retval TRUE     if the interface is different from current_mac_if.
 * @retval FALSE    if the interface is the same as current_mac_if.
 */
static BOOL tesla_mac_if_changed (
    vtss_port_no_t          port_no,
    vtss_port_interface_t   current_mac_if
) {
    return (phy_mac_mode_get(port_no) != current_mac_if) ? TRUE : FALSE;
}


// ** Register definitions -  MACSEC page**

/* PHY register pages */
#define VTSS_PHY_PAGE_STANDARD   0x0000 /**< Standard registers */
#define VTSS_PHY_PAGE_EXTENDED   0x0001 /**< Extended registers */
#define VTSS_PHY_PAGE_EXTENDED_2 0x0002 /**< Extended registers - page 2 */
#define VTSS_PHY_PAGE_EXTENDED_3 0x0003 /**< Extended registers - page 3 */
#define VTSS_PHY_PAGE_GPIO       0x0010 /**< GPIO registers */
#define VTSS_PHY_PAGE_MACSEC     0x0004 /**< MACSEC page */
#define VTSS_PHY_PAGE_TEST       0x2A30 /**< Test registers */
#define VTSS_PHY_PAGE_TR         0x52B5 /**< Token ring registers */

#define VTSS_PHY_PAGE_MACSEC_19 VTSS_PHY_PAGE_MACSEC, 19
#define VTSS_PHY_PAGE_MACSEC_20 VTSS_PHY_PAGE_MACSEC, 20
#define VTSS_PHY_F_PAGE_MACSEC_19_CMD_BIT  VTSS_PHY_BIT(15)
#define VTSS_PHY_F_PAGE_MACSEC_20_READY(value) VTSS_PHY_ENCODE_BITFIELD(value, 13, 2)

#define VTSS_PHY_F_PAGE_MACSEC_19_READ  VTSS_PHY_BIT(14)
#define VTSS_PHY_F_PAGE_MACSEC_19_TARGET(value) VTSS_PHY_ENCODE_BITFIELD(value, 12, 2)
#define VTSS_PHY_F_PAGE_MACSEC_20_TARGET(value) VTSS_PHY_ENCODE_BITFIELD(value, 0, 4)
#define VTSS_PHY_F_PAGE_MACSEC_19_CSR_REG_ADDR(value) VTSS_PHY_ENCODE_BITFIELD(value, 0, 14)

#define VTSS_PHY_PAGE_MACSEC_CSR_DATA_LSB VTSS_PHY_PAGE_MACSEC, 17
#define VTSS_PHY_PAGE_MACSEC_CSR_DATA_MSB VTSS_PHY_PAGE_MACSEC, 18

#define PHY_WR_PAGE(port_no, page_addr, value) vtss_phy_wr_page(port_no, page_addr, value)
#define PHY_RD_PAGE(port_no, page_addr, value) vtss_phy_rd_page(port_no, page_addr, value)

static vtss_rc vtss_phy_rd_wr_masked(BOOL                 read,
                                     const vtss_port_no_t port_no,
                                     const u32            addr,
                                     u16                  *const value,
                                     const u16            mask)
{
    vtss_rc           rc = VTSS_RC_OK;
    u16               reg, page, val;

    /* Page is encoded in address */
    page = (addr >> 5);
    reg = (addr & 0x1f);

    /* Change page */
    if (page)
        phy_write(port_no, 31, page);

    if (read) {
        /* Read */
        *value = phy_read(port_no, reg);
    } else if (mask != 0xffff) {
        /* Read-modify-write */
        val = phy_read(port_no, reg);
        phy_write(port_no, reg, (val & ~mask) | (*value & mask));
    } else {
        /* Write */
        phy_write(port_no, reg, *value);
    }

    /* Restore standard page */
    if (page)
        phy_write(port_no, 31, VTSS_PHY_PAGE_STANDARD);

    return rc;
}

/* Write PHY register */
// See comment at the do_page_chk
vtss_rc vtss_phy_wr_page(const vtss_port_no_t port_no,
                         const u16            page,
                         const u32            addr,
                         const u16            value)
{
    u16 val = value;
    return vtss_phy_rd_wr_masked(0, port_no, addr, &val, 0xFFFF);
}

// See comment at the do_page_chk
/* Read PHY register (include page) - Use this function in combination with the register defines for new code. */
static vtss_rc vtss_phy_rd_page(const vtss_port_no_t port_no,
                                const u16            page,
                                const u32            addr,
                                u16                  *const value)
{
    return vtss_phy_rd_wr_masked(1, port_no, addr, value, 0);
}

// #define vtss_phy_wr(port_no, reg_no, value) phy_write(port_no, reg_no, value)
// VTSS_PHY_PAGE_MACSEC

// Selecting MACSEC page registers
static vtss_rc vtss_phy_page_macsec(vtss_port_no_t port_no)
{
    vtss_phy_wr(port_no, 31, VTSS_PHY_PAGE_MACSEC);
    return VTSS_RC_OK;
}

// Function for checking if MACSEC registers are ready for new accesses. NOTE: **** Page register is left at 0xMACSEC, and not return to standard page  ****
// In - port_no : Any phy port with the chip
static vtss_rc vtss_phy_wait_for_macsec_command_busy(const vtss_port_no_t  port_no, u32 page)
{
    u16 val;
    u8 timeout = 255; // For making sure that we don't get stucked
    VTSS_RC(vtss_phy_page_macsec(port_no));

    // Wait for bit 15 to be set (or timeout)
    if (page == 19) {
        VTSS_RC(PHY_RD_PAGE(port_no, VTSS_PHY_PAGE_MACSEC_19, &val));
        while (!(val & VTSS_PHY_F_PAGE_MACSEC_19_CMD_BIT) && timeout)  {
            timeout--;
            VTSS_RC(PHY_RD_PAGE(port_no, VTSS_PHY_PAGE_MACSEC_19, &val));
        }
    } else if (page == 20) {
        VTSS_RC(PHY_RD_PAGE(port_no, VTSS_PHY_PAGE_MACSEC_20, &val));
        while (!(VTSS_PHY_F_PAGE_MACSEC_20_READY(val) == 0) && timeout)  {
            timeout--;
            VTSS_RC(PHY_RD_PAGE(port_no, VTSS_PHY_PAGE_MACSEC_20, &val));
        }
    }

    if (timeout == 0) {
        //VTSS_E("Unexpected timeout when accesing port:%d page:%d", port_no, page);
        return VTSS_RC_ERROR;
    } else {
        return VTSS_RC_OK;
    }
}

// See vtss_phy_csr_rd
static vtss_rc vtss_phy_macsec_csr_rd_private(const vtss_port_no_t port_no,
                                              const u16            target,
                                              const u32            csr_reg_addr,
                                              u32                  *value)
{
    u16 reg_value_lower;
    u16 reg_value_upper;
    u32 target_tmp = 0;

    if (target == 0x38 || target == 0x3C) {
        //VTSS_E("Port:%d, MACSEC to phy without MACSEC support, target:0x%X", port_no, target);
        return VTSS_RC_ERROR;
    }

    VTSS_RC(vtss_phy_wait_for_macsec_command_busy(port_no, 19)); // Wait for MACSEC register access
    VTSS_RC(PHY_WR_PAGE(port_no, VTSS_PHY_PAGE_MACSEC_20, VTSS_PHY_F_PAGE_MACSEC_20_TARGET((target >> 2))));

    if (target >> 2 == 1) {
        target_tmp = target & 3; // non-macsec access
    }
    VTSS_RC(vtss_phy_wait_for_macsec_command_busy(port_no, 19)); // Wait for MACSEC register access
    VTSS_RC(PHY_WR_PAGE(port_no, VTSS_PHY_PAGE_MACSEC_19,
                        VTSS_PHY_F_PAGE_MACSEC_19_CMD_BIT | VTSS_PHY_F_PAGE_MACSEC_19_TARGET(target_tmp) |
                        VTSS_PHY_F_PAGE_MACSEC_19_READ    | VTSS_PHY_F_PAGE_MACSEC_19_CSR_REG_ADDR(csr_reg_addr)));

    VTSS_RC(vtss_phy_wait_for_macsec_command_busy(port_no, 19)); // Wait for MACSEC register access
    VTSS_RC(PHY_RD_PAGE(port_no, VTSS_PHY_PAGE_MACSEC_CSR_DATA_LSB, &reg_value_lower));
    VTSS_RC(PHY_RD_PAGE(port_no, VTSS_PHY_PAGE_MACSEC_CSR_DATA_MSB, &reg_value_upper));
    VTSS_RC(vtss_phy_page_std(port_no));
    *value = (reg_value_upper << 16) | reg_value_lower;
    return VTSS_RC_OK;
}

// See vtss_phy_csr_wr
static vtss_rc vtss_phy_macsec_csr_wr_private(const vtss_port_no_t port_no,
                                              const u16            target,
                                              const u32            csr_reg_addr,
                                              const u32            value)
{

    /* Divide the 32 bit value to [15..0] Bits & [31..16] Bits */
    u16 reg_value_lower = (value & 0xffff);
    u16 reg_value_upper = (value >> 16);
    u32 target_tmp = 0;
    //    u16 val;

    // The only ones not accessible in non-MACsec devices are the MACsec
    // ingress and egress blocks at 0x38 and 0x3C (for each port).
    // Everything else is accessible using the so-called macsec_csr_wr/rd
    // functions using registers 17-20 in extended page 4 (as described in
    // PS1046).
    if (target == 0x38 || target == 0x3C) {
        //VTSS_E("Port:%d, MACSEC to phy without MACSEC support", port_no);
        return VTSS_RC_ERROR;
    }

    VTSS_RC(vtss_phy_wait_for_macsec_command_busy(port_no, 19)); // Wait for MACSEC register access
    VTSS_RC(PHY_WR_PAGE(port_no, VTSS_PHY_PAGE_MACSEC_20, VTSS_PHY_F_PAGE_MACSEC_20_TARGET((target >> 2))));

    //    VTSS_RC(PHY_RD_PAGE(port_no, VTSS_PHY_PAGE_MACSEC_20, &val));
    //    printf("reg20:%x\n",val);

    if (target >> 2 == 1 || target >> 2 == 3) {
        target_tmp = target; // non-macsec access
    }
    VTSS_RC(vtss_phy_wait_for_macsec_command_busy(port_no, 19)); // Wait for MACSEC register access
    VTSS_RC(PHY_WR_PAGE(port_no, VTSS_PHY_PAGE_MACSEC_CSR_DATA_LSB, reg_value_lower));
    VTSS_RC(vtss_phy_wait_for_macsec_command_busy(port_no, 19)); // Wait for MACSEC register access
    VTSS_RC(PHY_WR_PAGE(port_no, VTSS_PHY_PAGE_MACSEC_CSR_DATA_MSB, reg_value_upper));
    VTSS_RC(vtss_phy_wait_for_macsec_command_busy(port_no, 19)); // Wait for MACSEC register access
    VTSS_RC(PHY_WR_PAGE(port_no, VTSS_PHY_PAGE_MACSEC_19,  VTSS_PHY_F_PAGE_MACSEC_19_CMD_BIT |
                        VTSS_PHY_F_PAGE_MACSEC_19_TARGET(target_tmp) | VTSS_PHY_F_PAGE_MACSEC_19_CSR_REG_ADDR(csr_reg_addr)));

    VTSS_RC(vtss_phy_page_std(port_no));

    return VTSS_RC_OK;
}

//=============================================================================
// 6G Macro setup - Code comes from James McIntosh
//=============================================================================
static vtss_rc vtss_phy_sd6g_ib_cfg0_wr_private(const vtss_port_no_t port_no,
                                                const u8             ib_rtrm_adj,
                                                const u8             ib_sig_det_clk_sel,
                                                const u8             ib_reg_pat_sel_offset,
                                                const u8             ib_cal_ena)
{
    u32 base_val;
    u32 reg_val;

    // constant terms
    base_val = (1 << 30) | (1 << 29) | (5 << 21) | (1 << 19) | (1 << 14) | (1 << 12) | (2 << 10) | (1 << 5) | (1 << 4) | 7;
    // configurable terms
    reg_val = base_val | ((u32)(ib_rtrm_adj) << 25) | ((u32)(ib_sig_det_clk_sel) << 16) | ((u32)(ib_reg_pat_sel_offset) << 8) | ((u32)(ib_cal_ena) << 3);
    return vtss_phy_macsec_csr_wr_private(port_no, 7, 0x22, reg_val); // ib_cfg0
}

static vtss_rc vtss_phy_sd6g_ib_cfg1_wr_private(const vtss_port_no_t port_no,
                                                const u8             ib_tjtag,
                                                const u8             ib_tsdet,
                                                const u8             ib_scaly,
                                                const u8             ib_frc_offset)
{
    u32 ib_filt_val;
    u32 ib_frc_val;
    u32 reg_val = 0;

    // constant terms
    ib_filt_val = (1 << 7) + (1 << 6) + (1 << 5) + (1 << 4);
    ib_frc_val = (0 << 3) + (0 << 2) + (0 << 1);
    // configurable terms
    reg_val  = ((u32)ib_tjtag << 17) + ((u32)ib_tsdet << 12) + ((u32)ib_scaly << 8) + ib_filt_val + ib_frc_val + ((u32)ib_frc_offset << 0);
    return vtss_phy_macsec_csr_wr_private(port_no, 7, 0x23, reg_val); // ib_cfg1
}

static vtss_rc vtss_phy_sd6g_ib_cfg2_wr_private(const vtss_port_no_t port_no,
                                                const u32            ib_cfg2_val)
{
    return vtss_phy_macsec_csr_wr_private(port_no, 7, 0x24, ib_cfg2_val); // ib_cfg2
}

static vtss_rc vtss_phy_sd6g_ib_cfg3_wr_private(const vtss_port_no_t port_no,
                                                const u8             ib_ini_hp,
                                                const u8             ib_ini_mid,
                                                const u8             ib_ini_lp,
                                                const u8             ib_ini_offset)
{
    u32 reg_val;

    reg_val  = ((u32)ib_ini_hp << 24) + ((u32)ib_ini_mid << 16) + ((u32)ib_ini_lp << 8) + ((u32)ib_ini_offset << 0);
    return vtss_phy_macsec_csr_wr_private(port_no, 7, 0x25, reg_val); // ib_cfg3
}

static vtss_rc vtss_phy_sd6g_ib_cfg4_wr_private(const vtss_port_no_t port_no,
                                                const u8             ib_max_hp,
                                                const u8             ib_max_mid,
                                                const u8             ib_max_lp,
                                                const u8             ib_max_offset)
{
    u32 reg_val;

    reg_val  = ((u32)ib_max_hp << 24) + ((u32)ib_max_mid << 16) + ((u32)ib_max_lp << 8) + ((u32)ib_max_offset << 0);
    return vtss_phy_macsec_csr_wr_private(port_no, 7, 0x26, reg_val); // ib_cfg4
}

static vtss_rc vtss_phy_sd6g_pll_cfg_wr_private(const vtss_port_no_t port_no,
                                                const u8             pll_ena_offs,
                                                const u8             pll_fsm_ctrl_data,
                                                const u8             pll_fsm_ena)
{
    u32 reg_val;

    reg_val  = ((u32)pll_ena_offs << 21) + ((u32)pll_fsm_ctrl_data << 8) + ((u32)pll_fsm_ena << 7);
    return vtss_phy_macsec_csr_wr_private(port_no, 7, 0x2b, reg_val); // pll_cfg
}

static vtss_rc vtss_phy_sd6g_common_cfg_wr_private(const vtss_port_no_t port_no,
                                                   const u8             sys_rst,
                                                   const u8             ena_lane,
                                                   const u8             ena_loop,
                                                   const u8             qrate,
                                                   const u8             if_mode)
{
//  ena_loop = 8 for eloop
//           = 4 for floop
//           = 2 for iloop
//           = 1 for ploop
//  qrate    = 1 for SGMII, 0 for QSGMII
//  if_mode  = 1 for SGMII, 3 for QSGMII

    u32 reg_val;

    reg_val  = ((u32)sys_rst << 31) + ((u32)ena_lane << 18) + ((u32)ena_loop << 8) + ((u32)qrate << 6) + ((u32)if_mode << 4);
    return vtss_phy_macsec_csr_wr_private(port_no, 7, 0x2c, reg_val); // common_cfg
}

static vtss_rc vtss_phy_sd6g_gp_cfg_wr_private(const vtss_port_no_t port_no,
                                               const u32            gp_cfg_val)
{
    return vtss_phy_macsec_csr_wr_private(port_no, 7, 0x2e, gp_cfg_val); // gp_cfg
}

static vtss_rc vtss_phy_sd6g_misc_cfg_wr_private(const vtss_port_no_t port_no,
                                                 const u8             lane_rst)
{
    // all other bits are 0 for now
    return vtss_phy_macsec_csr_wr_private(port_no, 7, 0x3b, (u32)lane_rst); // misc_cfg
}


// Macro for maing sure that we don't run forever
#define SD6G_TIMEOUT(timeout_var) if (timeout_var-- == 0) {goto macro_6g_err;} else {delay_1(1);}

static vtss_rc vtss_phy_sd6g_patch_private(
    phy_id_t                    *phy_id,
    const vtss_port_no_t        port_no
)
{
    BOOL viper_rev_a = FALSE;
    u8 rcomp;
    u8 ib_rtrm_adj;
    u8 iter;

    u8 pll_fsm_ctrl_data;
    u8 qrate;
    u8 if_mode;

    u8 ib_sig_det_clk_sel_cal;
    u8 ib_sig_det_clk_sel_mm  = 7;
    u8 ib_tsdet_cal = 16;
    u8 ib_tsdet_mm  = 5;
    ulong rd_dat;
    u8 timeout;
    u8 timeout2;

    u16 reg_val;
    u8 mac_if;

    vtss_port_no_t base_port_no = 0; // FIXED

    println_str("FIXME: base_port_no");

    switch (phy_id->family) {
    // These chips support the 6G macro setup
    case VTSS_PHY_FAMILY_VIPER:
        viper_rev_a = (phy_id->revision == VTSS_PHY_VIPER_REV_A);

    // Fall through on purpose
    case VTSS_PHY_FAMILY_ELISE:
        break;
    default:
        return VTSS_RC_OK;
    }

    ib_sig_det_clk_sel_cal = viper_rev_a ? 0 : 7; // 7 for Elise and Viper Rev. B+

    VTSS_RC(vtss_phy_page_gpio(port_no));
    VTSS_RC(vtss_phy_rd(port_no, 19, &reg_val));
    VTSS_RC(vtss_phy_page_std(port_no));

    mac_if = (reg_val >> 14) & 0x3;

    if (mac_if == MAC_IF_EXTERNAL) { // QSGMII, See data sheet register 19G
        pll_fsm_ctrl_data = 120;
        qrate   = 0;
        if_mode = 3;
    } else if (mac_if == MAC_IF_INTERNAL) { // SGMII, See data sheet register 19G
        pll_fsm_ctrl_data = 60;
        qrate   = 1;
        if_mode = 1;
    } else {
        return VTSS_RC_OK;
    }


#if PHY_DEBUG
    println_str("Setting 6G");
#endif

    //VTSS_RC(vtss_phy_id_get_private(port_no, &phy_id));
    VTSS_RC(vtss_phy_macsec_csr_wr_private(base_port_no, 7, 0x3f, 0x40000001)); // read 6G MCB into CSRs

    timeout = 200;
    do {
        VTSS_RC(vtss_phy_macsec_csr_rd_private(base_port_no, 7, 0x3f, &rd_dat)); // wait for MCB read to complete
        SD6G_TIMEOUT(timeout);
    } while (rd_dat & 0x4000000);

    if (viper_rev_a) {
        timeout = 200;
        // modify RComp value for Viper Rev. A
        VTSS_RC(vtss_phy_macsec_csr_wr_private(base_port_no, 7, 0x11, 0x40000001)); // read LCPLL MCB into CSRs
        do {
            VTSS_RC(vtss_phy_macsec_csr_rd_private(base_port_no, 7, 0x11, &rd_dat)); // wait for MCB read to complete
            SD6G_TIMEOUT(timeout);
        } while (rd_dat & 0x4000000);

        VTSS_RC(vtss_phy_macsec_csr_rd_private(base_port_no, 7, 0x0f, &rd_dat)); // rcomp_status
        rcomp = rd_dat & 0xf; //~10;
        ib_rtrm_adj = rcomp - 3;
    } else {
        // use trim offset for Viper Rev. B+ and Elise
        ib_rtrm_adj = 16 - 3;
    }

#if PHY_DEBUG
    println_str("Initializing 6G...");
#endif
    VTSS_RC(vtss_phy_sd6g_ib_cfg0_wr_private(base_port_no, ib_rtrm_adj, ib_sig_det_clk_sel_cal, 0, 0)); // ib_cfg0
    if (viper_rev_a) {
        VTSS_RC(vtss_phy_sd6g_ib_cfg1_wr_private(base_port_no, 8, ib_tsdet_cal, 0, 1)); // ib_cfg1
    } else {
        VTSS_RC(vtss_phy_sd6g_ib_cfg1_wr_private(base_port_no, 8, ib_tsdet_cal, 15, 0)); // ib_cfg1
    }
    VTSS_RC(vtss_phy_sd6g_ib_cfg2_wr_private(base_port_no, 0x3f878194)); // ib_cfg2
    VTSS_RC(vtss_phy_sd6g_ib_cfg3_wr_private(base_port_no,  0, 31, 1, 31)); // ib_cfg3
    VTSS_RC(vtss_phy_sd6g_ib_cfg4_wr_private(base_port_no, 63, 63, 2, 63)); // ib_cfg4
    VTSS_RC(vtss_phy_sd6g_pll_cfg_wr_private(base_port_no, 3, pll_fsm_ctrl_data, 0)); // pll_cfg
    VTSS_RC(vtss_phy_sd6g_common_cfg_wr_private(base_port_no, 0, 0, 0, qrate, if_mode)); // sys_rst, ena_lane
    VTSS_RC(vtss_phy_sd6g_misc_cfg_wr_private(base_port_no, 1)); // lane_rst
    VTSS_RC(vtss_phy_sd6g_gp_cfg_wr_private(base_port_no, 768)); // gp_cfg
    VTSS_RC(vtss_phy_macsec_csr_wr_private(base_port_no, 7, 0x3f, 0x80000001)); // write back 6G MCB
    timeout = 200;
    do {
        VTSS_RC(vtss_phy_macsec_csr_rd_private(base_port_no, 7, 0x3f, &rd_dat)); // wait for MCB write to complete
        SD6G_TIMEOUT(timeout);
    } while (rd_dat & 0x8000000);

#if PHY_DEBUG
    println_str("Calibrating PLL...");
#endif
    VTSS_RC(vtss_phy_sd6g_pll_cfg_wr_private(base_port_no, 3, pll_fsm_ctrl_data, 1)); // pll_fsm_ena
    VTSS_RC(vtss_phy_sd6g_common_cfg_wr_private(base_port_no, 1, 1, 0, qrate, if_mode)); // sys_rst, ena_lane
    VTSS_RC(vtss_phy_macsec_csr_wr_private(base_port_no, 7, 0x3f, 0x80000001)); // write back 6G MCB

    do {
        VTSS_RC(vtss_phy_macsec_csr_rd_private(base_port_no, 7, 0x3f, &rd_dat)); // wait for MCB write to complete
        SD6G_TIMEOUT(timeout);
    } while (rd_dat & 0x8000000);

    // wait for PLL cal to complete
    timeout = 200;
    do {
        VTSS_RC(vtss_phy_macsec_csr_wr_private(base_port_no, 7, 0x3f, 0x40000001)); // read 6G MCB into CSRs
        timeout2 = 200;
        do {
            VTSS_RC(vtss_phy_macsec_csr_rd_private(base_port_no, 7, 0x3f, &rd_dat)); // wait for MCB read to complete
            SD6G_TIMEOUT(timeout2);
        } while (rd_dat & 0x4000000);
        VTSS_RC(vtss_phy_macsec_csr_rd_private(base_port_no, 7, 0x31, &rd_dat)); // pll_status
        SD6G_TIMEOUT(timeout);
    } while (rd_dat & 0x0001000); // wait for bit 12 to clear

#if PHY_DEBUG
    println_str("Calibrating IB...");
#endif
    // only for Viper Rev. A
    if (viper_rev_a) {
        // one time with SW clock
        VTSS_RC(vtss_phy_sd6g_ib_cfg0_wr_private(base_port_no, ib_rtrm_adj, ib_sig_det_clk_sel_cal, 0, 1)); // ib_cal_ena
        //VTSS_RC(vtss_phy_sd6g_ib_cfg1_wr_private(base_port_no, 8, ib_tsdet_cal, 0, 1)); // ib_cfg1
        VTSS_RC(vtss_phy_sd6g_misc_cfg_wr_private(base_port_no, 0)); // lane_rst
        VTSS_RC(vtss_phy_macsec_csr_wr_private(base_port_no, 7, 0x3f, 0x80000001)); // write back 6G MCB
        timeout = 200;
        do {
            VTSS_RC(vtss_phy_macsec_csr_rd_private(base_port_no, 7, 0x3f, &rd_dat)); // wait for MCB write to complete
            SD6G_TIMEOUT(timeout);
        } while (rd_dat & 0x8000000);
        // 11 cycles w/ SW clock
        for (iter = 0; iter < 11; iter++) {
            VTSS_RC(vtss_phy_sd6g_gp_cfg_wr_private(base_port_no, 769)); // set gp(0)
            VTSS_RC(vtss_phy_macsec_csr_wr_private(base_port_no, 7, 0x3f, 0x80000001)); // write back 6G MCB
            timeout = 200;
            do {
                VTSS_RC(vtss_phy_macsec_csr_rd_private(base_port_no, 7, 0x3f, &rd_dat)); // wait for MCB write to complete
                SD6G_TIMEOUT(timeout);
            } while (rd_dat & 0x8000000);
            VTSS_RC(vtss_phy_sd6g_gp_cfg_wr_private(base_port_no, 768)); // clear gp(0)
            VTSS_RC(vtss_phy_macsec_csr_wr_private(base_port_no, 7, 0x3f, 0x80000001)); // write back 6G MCB
            timeout = 200;
            do {
                VTSS_RC(vtss_phy_macsec_csr_rd_private(base_port_no, 7, 0x3f, &rd_dat)); // wait for MCB write to complete
                SD6G_TIMEOUT(timeout);
            } while (rd_dat & 0x8000000);
        }
    }
    // auto. cal
    if (viper_rev_a) {
        VTSS_RC(vtss_phy_sd6g_ib_cfg0_wr_private(base_port_no, ib_rtrm_adj, ib_sig_det_clk_sel_cal, 0, 0)); // ib_cal_ena
        VTSS_RC(vtss_phy_sd6g_ib_cfg1_wr_private(base_port_no, 8, ib_tsdet_cal, 15, 0)); // ib_cfg1
        VTSS_RC(vtss_phy_macsec_csr_wr_private(base_port_no, 7, 0x3f, 0x80000001)); // write back 6G MCB
        timeout = 200;
        do {
            VTSS_RC(vtss_phy_macsec_csr_rd_private(base_port_no, 7, 0x3f, &rd_dat)); // wait for MCB write to complete
            SD6G_TIMEOUT(timeout);
        } while (rd_dat & 0x8000000);
    }
    VTSS_RC(vtss_phy_sd6g_ib_cfg0_wr_private(base_port_no, ib_rtrm_adj, ib_sig_det_clk_sel_cal, 0, 1)); // ib_cal_ena
    VTSS_RC(vtss_phy_sd6g_misc_cfg_wr_private(base_port_no, 0)); // lane_rst
    VTSS_RC(vtss_phy_macsec_csr_wr_private(base_port_no, 7, 0x3f, 0x80000001)); // write back 6G MCB
    timeout = 200;
    do {
        VTSS_RC(vtss_phy_macsec_csr_rd_private(base_port_no, 7, 0x3f, &rd_dat)); // wait for MCB write to complete
        SD6G_TIMEOUT(timeout);
    } while (rd_dat & 0x8000000);
    // wait for IB cal to complete
    timeout = 200;
    do {
        VTSS_RC(vtss_phy_macsec_csr_wr_private(base_port_no, 7, 0x3f, 0x40000001)); // read 6G MCB into CSRs
        timeout2 = 200;
        do {
            VTSS_RC(vtss_phy_macsec_csr_rd_private(base_port_no, 7, 0x3f, &rd_dat)); // wait for MCB read to complete
            SD6G_TIMEOUT(timeout2);
        } while (rd_dat & 0x4000000);
        VTSS_RC(vtss_phy_macsec_csr_rd_private(base_port_no, 7, 0x2f, &rd_dat)); // ib_status0
        SD6G_TIMEOUT(timeout);
    } while (~rd_dat & 0x0000100); // wait for bit 8 to set

#if PHY_DEBUG
    println_str("Final settings...");
#endif

    VTSS_RC(vtss_phy_sd6g_ib_cfg0_wr_private(base_port_no, ib_rtrm_adj, ib_sig_det_clk_sel_mm, 1, 1)); // ib_sig_det_clk_sel, ib_reg_pat_sel_offset
    VTSS_RC(vtss_phy_sd6g_ib_cfg1_wr_private(base_port_no, 8, ib_tsdet_mm, 15, 0)); // ib_tsdet
    VTSS_RC(vtss_phy_macsec_csr_wr_private(base_port_no, 7, 0x3f, 0x80000001)); // write back 6G MCB
    timeout = 200;
    do {
        VTSS_RC(vtss_phy_macsec_csr_rd_private(base_port_no, 7, 0x3f, &rd_dat)); // wait for MCB write to complete
        SD6G_TIMEOUT(timeout);
    } while (rd_dat & 0x8000000);

    return VTSS_RC_OK;

macro_6g_err:
    //VTSS_E("Viper 6G macro not configured correctly for port:%d", port_no);
    return VTSS_RC_ERR_PHY_6G_MACRO_SETUP;
}

vtss_rc tesla_init_seq(
    vtss_port_no_t          port_no,
    phy_id_t                *phy_id
) {
    return atom12_init_seq(port_no, phy_id);
}


/**
 * VTSS_PHY_MAC_MODE_AND_FAST_LINK = 19G
 * VTSS_PHY_MICRO_PAGE             = 18G
 */
vtss_rc tesla_mac_media_if_setup (
    vtss_port_no_t          port_no,
    vtss_phy_reset_conf_t   *conf
) {
    u16                     micro_cmd_100fx = 0; // Use to signal to micro program if the fiber is 100FX (Bit 4). Default is 1000BASE-x
    u8                      media_operating_mode = 0;
    BOOL                    cu_prefered = FALSE;
    u16                     reg_val;
    u16                     reg_mask;
    BOOL                    if_changed;
    phy_id_t                phy_id;

#if PHY_DEBUG
    println_str("tesla_mac_media_if_setup");
#endif

    phy_read_id(port_no, &phy_id);

    if_changed = tesla_mac_if_changed(port_no, conf->mac_if);

    // Setup MAC Configuration
    VTSS_RC(vtss_phy_page_gpio(port_no));
    switch (conf->mac_if) {
    case VTSS_PORT_INTERFACE_SGMII:
        VTSS_RC(vtss_phy_wr_masked(port_no, 19,  0x0000, 0xC000));
        break;
    case VTSS_PORT_INTERFACE_QSGMII:
        VTSS_RC(vtss_phy_wr_masked(port_no, 19,  0x4000, 0xC000));
        break;
    case VTSS_PORT_INTERFACE_RGMII:
        VTSS_RC(vtss_phy_wr_masked(port_no, 19,  0x8000, 0xC000));
        break;
    case VTSS_PORT_INTERFACE_NO_CONNECTION:
        break;
    default:
        //VTSS_E("port_no %lu, Mac interface %d not supported",port_no, conf->mac_if);
        return VTSS_RC_ERROR;
    }

    // Setup media interface
    switch (conf->media_if) {
    case VTSS_PHY_MEDIA_IF_CU:
        media_operating_mode    = 0;
        cu_prefered             = TRUE;
        break;
    case VTSS_PHY_MEDIA_IF_SFP_PASSTHRU:
        media_operating_mode    = 1;
        cu_prefered             = TRUE;
        break;
    case VTSS_PHY_MEDIA_IF_FI_1000BX:
        media_operating_mode    = 2;
        cu_prefered             = FALSE;
        break;
    case VTSS_PHY_MEDIA_IF_FI_100FX:
        media_operating_mode    = 3;
        cu_prefered             = FALSE;
        micro_cmd_100fx         = 1 << 4;
        break;
    case VTSS_PHY_MEDIA_IF_AMS_CU_PASSTHRU:
        media_operating_mode    = 5;
        cu_prefered             = TRUE;
        break;
    case VTSS_PHY_MEDIA_IF_AMS_FI_PASSTHRU:
        media_operating_mode    = 5;
        cu_prefered             = FALSE;
        break;
    case VTSS_PHY_MEDIA_IF_AMS_CU_1000BX:
        media_operating_mode    = 6;
        cu_prefered             = TRUE;
        break;
    case VTSS_PHY_MEDIA_IF_AMS_FI_1000BX:
        media_operating_mode    = 6;
        cu_prefered             = FALSE;
        break;
    case VTSS_PHY_MEDIA_IF_AMS_CU_100FX:
        media_operating_mode    = 7;
        cu_prefered             = TRUE;
        micro_cmd_100fx         = 1 << 4;
        break;
    case VTSS_PHY_MEDIA_IF_AMS_FI_100FX:
        media_operating_mode    = 7;
        cu_prefered             = FALSE;
        micro_cmd_100fx         = 1 << 4;
        break;
    default:
        //VTSS_E("port_no %u, Media interface %d not supported", port_no, conf->media_if);
        return VTSS_RC_ERROR;
    }

    VTSS_RC(vtss_phy_bugzero_48512_workaround(port_no, TRUE)); //Bugzero#48512, Changing speed at one port gives CRC errors at other ports.
    VTSS_RC(vtss_phy_page_gpio(port_no));
    if (if_changed) {
#if defined(VTSS_FEATURE_SERDES_MACRO_SETTINGS)
        VTSS_RC(vtss_phy_warmstart_chk_micro_patch_mac_mode(port_no, conf));
#endif
        if (conf->mac_if ==  VTSS_PORT_INTERFACE_QSGMII) {
            VTSS_RC(vtss_phy_wr_masked (port_no, 18, 0x80E0, 0xFFFF)); // Configure SerDes macros for QSGMII MAC interface (See TN1080)
        } else {
            VTSS_RC(vtss_phy_wr_masked (port_no, 18, 0x80F0, 0xFFFF)); // Configure SerDes macros for 4xSGMII MAC interface (See TN1080)
        }
        VTSS_RC(vtss_phy_wait_for_micro_complete(port_no));

        VTSS_RC(vtss_phy_sd6g_patch_private(&phy_id, port_no));
    }

    VTSS_RC(vtss_phy_bugzero_48512_workaround(port_no, FALSE)); //Bugzero#48512, Changing speed at one port gives CRC errors at other ports.

    if (conf->media_if != VTSS_PHY_MEDIA_IF_CU) {
        VTSS_RC(vtss_phy_bugzero_48512_workaround(port_no, TRUE));//Bugzero#48512, Changing speed at one port gives CRC errors at other ports.

        // Setup media in micro program. Bit 8-11 is bit for the corresponding port (See TN1080)
        VTSS_RC(vtss_phy_page_gpio (port_no));
        // Should be warmstart checked, but not possible at the moment (Bugzilla#11826)
        VTSS_RC(vtss_phy_wr        (port_no, 18, 0x80C1 | ((1 << (port_no % 4)) << 8) | micro_cmd_100fx));
        VTSS_RC(vtss_phy_wait_for_micro_complete(port_no));
        VTSS_RC(vtss_phy_bugzero_48512_workaround(port_no, FALSE));//Bugzero#48512, Changing speed at one port gives CRC errors at other ports.
    }
    // Setup Media interface
    VTSS_RC(vtss_phy_page_std(port_no));
    reg_val  = media_operating_mode << 8 | (cu_prefered ? 0x0800 : 0x0000);

    reg_mask = 0x0F00;

    VTSS_RC(vtss_phy_wr_masked (port_no, 23, reg_val, reg_mask));

#if PHY_DEBUG
    print_str("media_operating_mode: ");
    print_dec(media_operating_mode);
    print_str(", media_if: ");
    print_dec(conf->media_if);
    print_cr_lf();
#endif

    // Port must be reset in order to update the media operating mode for register 23
    VTSS_RC(vtss_phy_soft_reset_port(port_no));

    return VTSS_RC_OK;
}


vtss_rc tesla_read_temp_reg(
    vtss_port_no_t  port_no,
    ushort          *temp
)
{
#if PHY_DEBUG
    println_str("tesla_read_temp_reg");
#endif
    *temp = vtss_phy_read_temp(port_no);
    return VTSS_RC_OK;
}


#endif // TESLA


/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
