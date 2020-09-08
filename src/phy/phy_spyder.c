//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "timer.h"
#include "print.h"

#include "phydrv.h"
#include "phy_base.h"
#include "phy_family.h"

#define PHY_DEBUG (0)

#if VTSS_SPYDER

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

static void init_seq_common_1 (vtss_port_no_t port_no)
{
    phy_page_tp(port_no);
    phy_write_masked(port_no, 8, 0x0200, 0x0200);

    phy_page_tr(port_no);

    phy_write(port_no, 16, 0xaf8a);
    phy_write_masked(port_no, 17, 0x0008, 0x000c);
    phy_write_masked(port_no, 18, 0x0000, 0x0000);
    phy_write(port_no, 16, 0x8f8a);

    phy_write(port_no, 16, 0xaf86);
    phy_write_masked(port_no, 17, 0x0000, 0x0000);
    phy_write_masked(port_no, 18, 0x0008, 0x000c);
    phy_write(port_no, 16, 0x8f86);

    phy_write(port_no, 16, 0xaf82);
    phy_write_masked(port_no, 17, 0x0100, 0x0180);
    phy_write_masked(port_no, 18, 0x0000, 0x0000);
    phy_write(port_no, 16, 0x8f82);

    phy_page_tp(port_no);
    phy_write_masked(port_no, 8, 0x0000, 0x0200);

    phy_page_std(port_no);
}

void phy_immediate_init_seq_8538 (vtss_port_no_t port_no) {
    ushort reg_3;

    /* This sequence should only be applied to revision A. As this function is only
    ** called if PHY register 2 reads 0007h, we only need to read register 3 to
    ** determine the model and revision number.
    */
    reg_3 = phy_read(port_no, 3);
    if ((reg_3 == ((ushort) PHY_ID_VTSS_8538_A)) || (reg_3 == ((ushort) PHY_ID_VTSS_8558_A))) {
        phy_write(port_no, 31, 0x0010);
        phy_write(port_no,  0, 0x4c19);
        phy_write_masked(port_no, 0, 0x8000, 0x8000);
        phy_write(port_no, 31, 0x0000);
    }
}

/*
** Initialization sequence for 8538/8558 PHYs
*/
vtss_rc init_seq_8538(
    vtss_port_no_t          port_no,
    phy_id_t                *phy_id
) {
    init_seq_common_1(port_no);

    /* Turn off Carrier Extensions */
    phy_page_ext(port_no);
    phy_write_masked(port_no, 20, 0x8000, 0x8000);
    phy_page_std(port_no);

    if ((phy_id->model == PHY_MODEL_VTSS_8658) && (phy_id->revision == 0)) {
        /* 100-Base FX Clock Data Recovery Improvement */
        ushort reg17, reg18;
        phy_page_tr(port_no);
        phy_write(port_no, 16, 0xae0e);
        reg18 = phy_read(port_no, 18);
        reg17 = phy_read(port_no, 17);
        phy_write(port_no, 18, reg18);
        phy_write(port_no, 17, (reg17&0xffef));
        phy_write(port_no, 16, 0x8e0e);
        phy_page_std(port_no);
    } else if ((phy_id->model == PHY_MODEL_VTSS_8558) && (phy_id->revision == 0)) {
        phy_page_tr(port_no);
        phy_write(port_no, 18, 0x0000);
        phy_write(port_no, 17, 0x003d);
        phy_write(port_no, 16, 0x8606);
        phy_write(port_no, 18, 0x0000);
        phy_write(port_no, 17, 0x003d);
        phy_write(port_no, 16, 0x8e06);
        phy_page_std(port_no);


        phy_page_tp(port_no);
        phy_write_masked(port_no, 27, 0x8000, 0x8000);
        phy_write_masked(port_no, 19, 0x0300, 0x0f00);
        phy_page_std(port_no);

        phy_page_ext(port_no);
        phy_write_masked(port_no, 17, 0x0000, 0x0001);
        phy_page_std(port_no);

        if ((port_no == 0) || (port_no == 8) || (port_no == 16)) {
            phy_write(port_no, 31, 0x10);

            phy_write(port_no,  0, 0x7009);
            phy_write(port_no, 12, 0x5000);
            phy_write(port_no, 11, 0xffff);
            phy_write(port_no, 12, 0x5002);
            phy_write(port_no, 12, 0x5040);
            phy_write(port_no, 12, 0x500C);
            phy_write(port_no, 12, 0x5002);
            phy_write(port_no, 12, 0x5040);
            phy_write(port_no, 12, 0x5021);
            phy_write(port_no, 12, 0x5002);
            phy_write(port_no, 12, 0x5040);
            phy_write(port_no, 12, 0x5022);
            phy_write(port_no, 12, 0x5002);
            phy_write(port_no, 12, 0x5040);
            phy_write(port_no, 12, 0x5023);
            phy_write(port_no, 12, 0x50C2);
            phy_write(port_no, 12, 0x50AD);
            phy_write(port_no, 12, 0x50C2);
            phy_write(port_no, 12, 0x50CA);
            phy_write(port_no, 12, 0x5075);
            phy_write(port_no, 12, 0x50CB);
            phy_write(port_no, 12, 0x509A);
            phy_write(port_no, 12, 0x5075);
            phy_write(port_no, 12, 0x50CA);
            phy_write(port_no, 12, 0x5046);
            phy_write(port_no, 12, 0x5085);
            phy_write(port_no, 12, 0x50CB);
            phy_write(port_no, 12, 0x50CD);
            phy_write(port_no, 12, 0x5085);
            phy_write(port_no, 12, 0x50CA);
            phy_write(port_no, 12, 0x50CC);
            phy_write(port_no, 12, 0x50D2);
            phy_write(port_no, 12, 0x50CA);
            phy_write(port_no, 12, 0x50D2);
            phy_write(port_no, 12, 0x50AD);
            phy_write(port_no, 12, 0x5022);
            phy_write(port_no, 12, 0x5022);
            phy_write(port_no, 12, 0x5022);
            phy_write(port_no, 12, 0x5022);
            phy_write(port_no, 12, 0x0000);
            phy_write(port_no,  0, 0x4099);
            phy_write(port_no,  0, 0xc099);
        }
    }

    phy_page_std(port_no);

    phy_write_masked(port_no, 18, 0x0040, 0x00C0);

//#if PERFECT_REACH_LNK_DN
#if 0 //default actiphy is disabled
    phy_page_std(port_no);
    phy_write_masked(port_no, 28, 0x0040, 0x0040);
#endif  /* PERFECT_REACH_LNK_DN */


    /* 100M edge rate and transmitter amplitude control */
    phy_write_masked(port_no, 24, 0xa002, 0xe00e);

    phy_page_ext(port_no);
    phy_write_masked(port_no, 20, 0x1800, 0x1800);
    phy_page_std(port_no);

    vtss_phy_enab_smrt_premphasis(port_no);

    return VTSS_RC_OK;
}
#endif

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
