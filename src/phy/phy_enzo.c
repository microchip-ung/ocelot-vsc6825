//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "timer.h"
#include "print.h"

#include "phydrv.h"
#include "phy_base.h"
#include "phy_family.h"

#define PHY_DEBUG (0)

#if VTSS_ENZO

/****************************************************************************
 *
 *
 * Defines
 *
 *
 ****************************************************************************/

/****************************************************************************
 *
 *
 * Typedefs and enums
 *
 *
 ****************************************************************************/


/*****************************************************************************
 *
 *
 * Prototypes for local functions
 *
 *
 *
 ****************************************************************************/

/*****************************************************************************
 *
 *
 * Local data
 *
 *
 *
 ****************************************************************************/


/*****************************************************************************
 *
 *
 * PHY initialization sequences
 *
 *
 *
 ****************************************************************************/

vtss_rc init_seq_8634_8664(
    vtss_port_no_t          port_no,
    phy_id_t                *phy_id
) {
    ushort      reg;

    phy_page_ext(port_no);
    reg = phy_read(port_no, 23); /* PHY address at bit 11-15 */
    phy_page_std(port_no);

    /* Script runs only for the first port of each device in system */
    if ((reg & (0x3 << 11)) == 0) {
        /* Enable Broad-cast writes for this device */
        phy_page_std(port_no);
        phy_write_masked(port_no, 22, 0x0001, 0x0001);

        if (phy_id->revision == 0) {    /*- Rev A */

            /* BZ 2633 */
            /* Enable LED blinking after reset */
            phy_page_ext(port_no);
            phy_write_masked(port_no, 19, 0x0800, 0x0800);

            /* BZ 2637 */
            /* 100/1000 Base-T amplitude compenstation */
            phy_page_tp(port_no);
            phy_write_masked(port_no, 24, 0x0003, 0x000b);
            phy_page_std(port_no);
            phy_write_masked(port_no, 24, 0x0004, 0x0005);

            /* BZ 2639 / BZ 2642 */
            /* Improve robustness of 10Base-T performance */
            phy_page_tr(port_no);
            phy_write(port_no, 18, 0x0);
            phy_write(port_no, 17, 0x3f);
            phy_write(port_no, 16, 0x8794);

            phy_write(port_no, 18, 0xf7);
            phy_write(port_no, 17, 0xadb4);
            phy_write(port_no, 16, 0x879e);

            phy_write(port_no, 18, 0x0);
            phy_write(port_no, 17, 0x32);
            phy_write(port_no, 16, 0x87a0);

            phy_write(port_no, 18, 0x41);
            phy_write(port_no, 17, 0x410);
            phy_write(port_no, 16, 0x87a2);

            phy_write(port_no, 18, 0x41);
            phy_write(port_no, 17, 0x410);
            phy_write(port_no, 16, 0x87a4);

            phy_write(port_no, 18, 0x41);
            phy_write(port_no, 17, 0x284);
            phy_write(port_no, 16, 0x87a6);

            phy_write(port_no, 18, 0x92);
            phy_write(port_no, 17, 0xbcb8);
            phy_write(port_no, 16, 0x87a8);

            phy_write(port_no, 18, 0x3);
            phy_write(port_no, 17, 0xcfbf);
            phy_write(port_no, 16, 0x87aa);

            phy_write(port_no, 18, 0x49);
            phy_write(port_no, 17, 0x2451);
            phy_write(port_no, 16, 0x87ac);

            phy_write(port_no, 18, 0x1);
            phy_write(port_no, 17, 0x1410);
            phy_write(port_no, 16, 0x87c0);

            phy_write(port_no, 18, 0x10);
            phy_write(port_no, 17, 0xb498);
            phy_write(port_no, 16, 0x87e8);

            phy_write(port_no, 18, 0x71);
            phy_write(port_no, 17, 0xe7dd);
            phy_write(port_no, 16, 0x87ea);

            phy_write(port_no, 18, 0x69);
            phy_write(port_no, 17, 0x6512);
            phy_write(port_no, 16, 0x87ec);

            phy_write(port_no, 18, 0x49);
            phy_write(port_no, 17, 0x2451);
            phy_write(port_no, 16, 0x87ee);

            phy_write(port_no, 18, 0x45);
            phy_write(port_no, 17, 0x410);
            phy_write(port_no, 16, 0x87f0);

            phy_write(port_no, 18, 0x41);
            phy_write(port_no, 17, 0x410);
            phy_write(port_no, 16, 0x87f2);

            phy_write(port_no, 18, 0x0);
            phy_write(port_no, 17, 0x10);
            phy_write(port_no, 16, 0x87f4);

            phy_page_tp(port_no);
            phy_write_masked(port_no, 9, 0x0040, 0x0040);
            phy_write_masked(port_no, 22, 0x0010, 0x0010);
            phy_page_std(port_no);

            /* BZ 2643 */
            /* Performance optimization - 100Base-TX/1000Base-T slave */
            phy_page_tp(port_no);
            phy_write_masked(port_no, 0, 0x0060, 0x00e0);
            phy_page_tr(port_no);

            phy_write(port_no, 18, 0x12);
            phy_write(port_no, 17, 0x480a);
            phy_write(port_no, 16, 0x8f82);

            phy_write(port_no, 18, 0x0);
            phy_write(port_no, 17, 0x422);
            phy_write(port_no, 16, 0x8f86);

            phy_write(port_no, 18, 0x3c);
            phy_write(port_no, 17, 0x3800);
            phy_write(port_no, 16, 0x8f8a);

            phy_write(port_no, 18, 0x8);
            phy_write(port_no, 17, 0xe33f);
            phy_write(port_no, 16, 0x83ae);
            phy_page_std(port_no);
        }   /*- Rev A */

        /* BZ 2112 */
        /* Turn off Carrier Extensions */
        phy_page_ext(port_no);
        phy_write_masked(port_no, 20, 0x8000, 0x8000);
        phy_page_std(port_no);

        /* Turn-off broad-cast writes for this device */
        phy_page_std(port_no);
        phy_write_masked(port_no, 22, 0x0000, 0x0001);
    }

    return VTSS_RC_OK;
}
#endif

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
