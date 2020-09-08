//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "vtss_api_base_regs.h"
#include "h2sdcfg.h"
#include "timer.h" // For dealy_1()
#include "misc2.h"
#include "h2io.h"
#ifndef NDEBUG
#include "print.h"
#endif

/*****************************************************************************
 *
 *
 * Defines
 *
 *
 *
 ****************************************************************************/
#if defined(H2_SD6G_DEBUG_ENABLE)
#include "print.h"
#endif /* H2_SD6G_DEBUG_ENABLE */

#define RCOMP_CFG0 VTSS_IOREG(VTSS_TO_MACRO_CTRL,0x8)



// Defines for better be able to share code from the managed system.
#define L26_WRM H2_WRITE_MASKED
#define L26_WR H2_WRITE
#define L26_RD(reg, value) H2_READ(reg, *value)
#define u32 ulong
#define l26_wr h2_write
#define VTSS_RC(x) x
#define l26_sd6g_write(addr, wait) h2_sd6g_write(addr)
#define l26_sd1g_write(addr, wait) h2_sd1g_write(addr)
#define L26_WRM_SET(reg, bit) H2_WRITE_MASKED(reg, bit, bit)
#define L26_WRM_CLR(reg, bit) H2_WRITE_MASKED(reg, 0, bit)

/*****************************************************************************
 *
 *
 * Typedefs and enums
 *
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

/* ************************************************************************ */
void h2_sd6g_write(ulong addr)
/* ------------------------------------------------------------------------ --
 * Purpose     : Serdes6G write data
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    ulong dat;

#if defined(VTSS_ARCH_LUTON26)
    ulong mask;

    mask = VTSS_F_MACRO_CTRL_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG_SERDES6G_WR_ONE_SHOT;

    /* Transfers data from MCB master (CSR target) to MCB slave */
    H2_WRITE(VTSS_MACRO_CTRL_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG,
            VTSS_F_MACRO_CTRL_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG_SERDES6G_ADDR(addr) | mask);

    do { /* Wait until write operation is completed  */
        H2_READ(VTSS_MACRO_CTRL_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG, dat);
    } while(dat & mask);
    delay_1(2);

#elif defined(VTSS_ARCH_OCELOT)
    // EA = 0; // Disable interrupt while writing the date via serial Macro Configuration Bus (MCB)
    H2_WRITE(VTSS_HSIO_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG,
             VTSS_F_HSIO_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG_SERDES6G_ADDR(addr) |
             VTSS_F_HSIO_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG_SERDES6G_WR_ONE_SHOT(1));

    /* Wait until write operation is completed  */
    do {
        H2_READ(VTSS_HSIO_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG, dat);
    } while(VTSS_X_HSIO_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG_SERDES6G_WR_ONE_SHOT(dat));
    // EA = 1; // Enable interrupt
#endif
}

/* ************************************************************************ */
void h2_sd6g_read(ulong addr)
/* ------------------------------------------------------------------------ --
 * Purpose     : Serdes6G write data
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    ulong dat;

#if defined(VTSS_ARCH_LUTON26)
    ulong mask;

    mask = VTSS_F_MACRO_CTRL_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG_SERDES6G_RD_ONE_SHOT;

    /* Transfers data from MCB master (CSR target) to MCB slave */
    H2_WRITE(VTSS_MACRO_CTRL_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG,
            VTSS_F_MACRO_CTRL_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG_SERDES6G_ADDR(addr) | mask);

    do { /* Wait until write operation is completed  */
        H2_READ(VTSS_MACRO_CTRL_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG, dat);
    } while(dat & mask);
    delay_1(2);

#elif defined(VTSS_ARCH_OCELOT)
    // EA = 0; // Disable interrupt while reading the date via serial Macro Configuration Bus (MCB)
    H2_WRITE(VTSS_HSIO_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG,
             VTSS_F_HSIO_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG_SERDES6G_ADDR(addr) |
             VTSS_F_HSIO_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG_SERDES6G_RD_ONE_SHOT(1));

    /* Wait until write operation is completed  */
    do {
        H2_READ(VTSS_HSIO_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG, dat);
    } while(VTSS_X_HSIO_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG_SERDES6G_RD_ONE_SHOT(dat));
    // EA = 1; // Enable interrupt
#endif
}

/* ************************************************************************ */
void h2_sd6g_cfg(vtss_serdes_mode_t mode, ulong addr)
/* ------------------------------------------------------------------------ --
 * Purpose     : Serdes6G setup (Disable/2G5/QSGMII/SGMII)
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
#if defined(VTSS_ARCH_LUTON26)
{
    u32 ib_rf=0, ctrl_data=0, if_mode=1, ob_ena_cas=0, ob_lev=0, ib_vbac=5, ib_vbcom=4, rcomp_val=0, ob_post_0=0, ib_ic_ac=0, ib_c=15, ib_chf=0;
    u32 ob_sr = 7;
    BOOL ena_lane=1, ena_rot=0, qrate=0, hrate=0, ob_ena1v=0, if_100fx=0, ib_cterm_ena=0;


//  VTSS_D("addr: 0x%lx, mode: %s", addr, l26_serdes_mode_txt(mode));
    ob_ena1v = 1; /* Based on Ref board design */
    ib_cterm_ena = 1; /* Based on Ref board design */

    switch (mode) {
    case VTSS_SERDES_MODE_2G5:
        /* Seredes6g_ob_cfg  */
        ob_post_0 = 2;
        /* Seredes6g_ob_cfg1 */
        ob_ena_cas = 1;
        ob_lev = ob_ena1v ? 48 : 63;
        /* Seredes6g_des_cfg --> see code */
        /* Seredes6g_ib_cfg */
        ib_ic_ac = ob_ena1v ? 2 : 0;
        ib_vbac  = ob_ena1v ? 4 : 5;
        ib_rf    = ob_ena1v ? 2 : 10;
        ib_vbcom = ob_ena1v ? 4 : 5;
        /* Seredes6g_ib_cfg1 */
        ib_c = ob_ena1v ? 6 : 10;
        ib_chf = ob_ena1v ? 1 : 0;
        /* Seredes6g_pll_cfg */
        ena_rot = 1;
        ctrl_data = 48;
        /* Seredes6g_common_cfg */
        qrate = 0;
        hrate = 1;
        break;
    case VTSS_SERDES_MODE_QSGMII:
        /* Seredes6g_ob_cfg  */
        ob_sr = 0; /* Based on Ref board design */
        ob_post_0 = 2;  /* Based on Ref board design */
        /* Seredes6g_ob_cfg1 */
        ob_ena_cas = 1;
        ob_lev = 24;
        /* Seredes6g_ib_cfg */
        ib_rf = 4;
        /* Seredes6g_ib_cfg1 */
        ib_c = 4;
        /* Seredes6g_pll_cfg */
        /* Seredes6g_pll_cfg */
        ctrl_data = 120;
        if_mode = 3;
        qrate = 0;
        hrate = 0;
        break;
    case VTSS_SERDES_MODE_SGMII:
        ob_lev = 48;
        ob_ena_cas = 2;
        ib_rf = 15;
        ctrl_data = 60;
        qrate = 1;
        hrate = 0;
        break;
    case VTSS_SERDES_MODE_100FX:
        ob_lev = 48;
        ob_ena_cas = 1;
        ib_rf = 15;
        ctrl_data = 60;
        qrate = 1;
        hrate = 0;
        if_100fx = 1;
        break;
    case VTSS_SERDES_MODE_1000BaseX:
        ob_lev = 48;
        ob_ena_cas = 2;
        ib_rf = 15;
        ctrl_data = 60;
        qrate = 1;
        hrate = 0;
        break;
    case VTSS_SERDES_MODE_DISABLE:
        ob_lev = 0;
        ob_ena_cas = 0;
        ib_rf = 0;
        ib_vbcom = 0;
        ena_rot = 0;
        ctrl_data = 0;
        qrate = 0;
        hrate = 0;
        break;
    default:
#if defined(H2_SD6G_DEBUG_ENABLE)
        println_str("%% Serdes6g mode %s not supported", l26_serdes_mode_txt(mode));
#endif /* H2_SD6G_DEBUG_ENABLE */
        return;
    }
    /* RCOMP_CFG0.MODE_SEL = 2 */
    VTSS_RC(l26_wr(RCOMP_CFG0,0x3<<8));

    /* RCOMP_CFG0.RUN_CAL = 1 */
    VTSS_RC(l26_wr(RCOMP_CFG0, 0x3<<8|1<<12));

    do { /* Wait for calibration to finish */
        L26_RD(VTSS_MACRO_CTRL_RCOMP_STATUS_RCOMP_STATUS, &rcomp_val);
    } while(rcomp_val & VTSS_F_MACRO_CTRL_RCOMP_STATUS_RCOMP_STATUS_BUSY);

    L26_RD(VTSS_MACRO_CTRL_RCOMP_STATUS_RCOMP_STATUS, &rcomp_val);
    rcomp_val = VTSS_X_MACRO_CTRL_RCOMP_STATUS_RCOMP_STATUS_RCOMP(rcomp_val);

    /* 1. Configure macro, apply reset */
    /* OB_CFG  */
    L26_WRM(VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG,
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_POL |
            VTSS_ENCODE_BITFIELD(rcomp_val+1,4,4) | /* RCOMP: bit 4-7 */
            VTSS_ENCODE_BITFIELD(ob_sr,0,4) |       /* SR:    bit 0-3 */
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_POST0(ob_post_0) |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_SR_H |
            (ob_ena1v ? VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_ENA1V_MODE : 0),
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_POL |
            VTSS_ENCODE_BITMASK(4,4) | /* RCOMP: bit 4-7 */
            VTSS_ENCODE_BITMASK(0,4) | /* SR:    bit 0-3 */
            VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_POST0 |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_SR_H |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_ENA1V_MODE);

    /* OB_CFG1 */
    L26_WRM(VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1,
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1_OB_ENA_CAS(ob_ena_cas) |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1_OB_LEV(ob_lev),
            VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1_OB_ENA_CAS |
            VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1_OB_LEV);

     /* IB_CFG */
    L26_WRM(VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG,
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_IC_AC(ib_ic_ac) |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_RT(15) |
            VTSS_ENCODE_BITFIELD(ib_vbac,7,3) |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_RESISTOR_CTRL(rcomp_val+2) |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_VBCOM(ib_vbcom) |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_RF(ib_rf),
            VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_IC_AC |
            VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_RT |
            VTSS_ENCODE_BITMASK(7,3) |
            VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_RESISTOR_CTRL |
            VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_VBCOM |
            VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_RF);

    /* IB_CFG1 */
    L26_WRM(VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1,
            (ib_cterm_ena ? VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_CTERM_ENA : 0) |
            (ib_chf ? VTSS_BIT(7) : 0 ) |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_C(ib_c) |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_DIS_EQ |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_ENA_OFFSAC |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_ENA_OFFSDC |
            (if_100fx ? VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_FX100_ENA : 0) |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_RST,
            VTSS_BIT(7) |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_CTERM_ENA |
            VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_C |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_DIS_EQ |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_ENA_OFFSAC |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_ENA_OFFSDC |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_FX100_ENA |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_RST);

    /* DES_CFG */
    L26_WRM(VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG,
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_PHS_CTRL(6) |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_MBTR_CTRL(2) |
           (if_100fx ? VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_CPMD_SEL(2) : 0) |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_BW_HYST(5) |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_BW_ANA(5),
            VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_PHS_CTRL |
            VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_MBTR_CTRL |
            VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_CPMD_SEL |
            VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_BW_HYST |
            VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_BW_ANA);

    /* PLL_CFG */
    L26_WRM(VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG,
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_FSM_CTRL_DATA(ctrl_data) |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_FSM_ENA |
            (ena_rot ? VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_ENA_ROT : 0),
            VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_FSM_CTRL_DATA |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_FSM_ENA |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_ENA_ROT);

    /* Write masked to avoid changing RECO_SEL_* fields used by SyncE */
    /* COMMON_CFG */
    L26_WRM(VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG,
            (ena_lane ? VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_ENA_LANE : 0) |
            (hrate ? VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_HRATE : 0) |
            (qrate ? VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_QRATE : 0) |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_IF_MODE(if_mode),
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_SYS_RST |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_ENA_LANE |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_HRATE |
            VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_QRATE |
            VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_IF_MODE);


    /* MISC_CFG */
    L26_WRM(VTSS_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG,
            (if_100fx ? VTSS_F_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_DES_100FX_CPMD_ENA : 0),
            VTSS_F_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_DES_100FX_CPMD_ENA);

    L26_WRM(VTSS_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG,
            (if_100fx ? VTSS_F_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_DES_100FX_CPMD_ENA : 0) |
            VTSS_F_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_LANE_RST,
            VTSS_F_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_DES_100FX_CPMD_ENA |
            VTSS_F_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_LANE_RST);

    VTSS_RC(l26_sd6g_write(addr, L26_SERDES_WAIT));

    /* 2. Release PLL reset */
    L26_WRM_SET(VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG,
                VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_SYS_RST);
    VTSS_RC(l26_sd6g_write(addr, L26_SERDES_WAIT));

    /* 3. Release digital reset */
    L26_WRM_CLR(VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1,
                VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_RST);

    L26_WRM(VTSS_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG, 0,
            VTSS_F_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_LANE_RST);
    VTSS_RC(l26_sd6g_write(addr, 0));
}

#elif defined(VTSS_ARCH_OCELOT)
{
    u32 ob_ena1v_mode = 0;
    u32 ob_post0 = 0;
    u32 ob_prec = 0;
    u32 ob_ena_cas = 0;
    u32 ob_lev = 24;
    u32 ob_idle = 0;
    u32 des_phs_ctrl = 2; // BZ#21826
    u32 des_cpmd_sel = 0;
    u32 des_bw_ana = 3; // as per BZ 17124
    u32 ib_sig_det_clk_sel = 7;
    u32 ser_alisel = 0;
    u32 ser_enali = 0;
    u32 pll_div4 = 0;
    u32 pll_ena_rot = 0;
    u32 pll_fsm_ctrl_data = 120;
    u32 pll_rot_dir = 0;
    u32 pll_rot_frq = 0;
    u32 hrate = 0;
    u32 qrate = 0;
    u32 if_mode = 3;
    u32 des_100fx_cpmd_ena = 0;
    u32 rx_lpi_mode_ena = 0;
    u32 tx_lpi_mode_ena = 0;
    u32 ib_concur = 1;
    u32 ena_lane = 1;
    u32 ib_sig_det_ena = 1;
    u32 ib_reg_ena = 1;
    u32 ib_sam_ena = 1;
    u32 ib_eqz_ena = 1;
    u32 ib_urge = 4;
    u32 test_mode = 0;
    u32 test_pattern = 0;
    u32 ib_filt_hp = 1, ib_filt_mid = 1, ib_filt_lp = 1, ib_filt_offset = 1;
    u32 ib_frc_hp = 0, ib_frc_mid = 0, ib_frc_lp = 0, ib_frc_offset = 0;
    u32 ib_ini_hp = 0, ib_ini_mid = 31, ib_ini_lp = 1, ib_ini_offset = 31;

    switch (mode) {
    case VTSS_SERDES_MODE_PCIE:
        ob_ena1v_mode = 0;
        ob_post0 = 0;
        ob_ena_cas = 0;
        ob_lev = 63;
        ser_alisel = 0;
        ser_enali = 0;
        pll_ena_rot = 0;
        pll_fsm_ctrl_data = 60;
        pll_rot_frq = 0;
        hrate = 1;
        qrate = 0;
        if_mode = 1;
        des_100fx_cpmd_ena = 0;
        des_bw_ana = 3;
        rx_lpi_mode_ena = 1;
        tx_lpi_mode_ena = 1;
        des_phs_ctrl = 3;  // BZ#21826
        break;
    case VTSS_SERDES_MODE_2G5:
        ob_ena1v_mode = 0;
        ob_post0 = 0;
        ob_ena_cas = 0;
        ob_lev = 63;
        ser_alisel = 0;
        ser_enali = 0;
        pll_ena_rot = 1;
        pll_fsm_ctrl_data = 48;
        pll_rot_frq = 1;
        hrate = 1;
        qrate = 0;
        if_mode = 1;
        des_100fx_cpmd_ena = 0;
        des_bw_ana = 4; // as per BZ 17124
        break;
    case VTSS_SERDES_MODE_QSGMII:
        ob_ena1v_mode = 0;
        ob_post0 = 0;
        ob_ena_cas = 0;
        ob_lev = 24;
        ser_alisel = 0;
        ser_enali = 0;
        pll_ena_rot = 0;
        pll_fsm_ctrl_data = 120;
        pll_rot_frq = 0;
        hrate = 0;
        qrate = 0;
        if_mode = 3;
        des_100fx_cpmd_ena = 0;
        des_bw_ana = 5; // as per BZ 17124
        break;
    case VTSS_SERDES_MODE_SGMII:
        ob_ena1v_mode = 1;
        ob_post0 = 0;
        ob_ena_cas = 2;
        ob_lev = 48;
        ser_alisel = 0;
        ser_enali = 0;
        pll_ena_rot = 0;
        pll_fsm_ctrl_data = 60;
        pll_rot_frq = 0;
        hrate = 0;
        qrate = 1;
        if_mode = 1;
        des_100fx_cpmd_ena = 0;
        break;
    case VTSS_SERDES_MODE_100FX:
        ob_ena1v_mode = 1;
        ob_post0 = 1;
        ob_ena_cas = 0;
        ob_lev = 48;
        ser_alisel = 0;
        ser_enali = 0;
        pll_ena_rot = 0;
        pll_fsm_ctrl_data = 60;
        pll_rot_frq = 0;
        hrate = 0;
        qrate = 1;
        if_mode = 1;
        des_100fx_cpmd_ena = 1;

        /* BZ#21826 - Observe the CRC frames while receiving frame content is all Zeros/Ones with 1518 bytes frame length
         *            when 100Base SFP modules is plugged.
         *
         * Observation:
         * Some bits(for example, PCS_ERROR_STICKY and SYNC_LOST_STICK) are raised in register
         * DEV:PCS_FX100_STATUS:PCS_FX100_STATUS. These error bits will occur the link-down
         * event in h2_pcs1g_100fx_status_get().
         *
         * Background:
         * SFP rate selector pin is an input to the SFP plug, controlled by the host.
         * On PCB121, SFP1_RATESEL is controlled by Ferret GPIO18 and SFP2_RATESEL is
         * controlled by Ferret GPIO19.
         * These Ferret GPIO pins are also used for strapping VCORE_CFG mode during
         * reset, so has a weak pulldown (4K7, SFP1_RATESEL) and a weak pullup
         * (4K7, SFP2_RATESEL) respectively, on PCB121 - at least until a PCIe module
         * connected to the NPI/PCIe connector overrules this (the NPI module leaves the signals as-is).
         * The SFP MSA states that if RATESEL is used by a plug, then the plug must
         * have an internal pulldown (>30K) on the signal.  If the signal is low then
         * the plug must operate at reduced bandwidth, if the signal is high then the
         * plug must operate at full bandwidth.
         * Hence if you do not deliberately control GPIO18 and GPIO19, then SFP1_RATESEL
         * will most likely be low (due to the PCB121 pulldown on the signal) and a
         * RATESEL-aware SFP plug in the SFP1 slot will thus operate at reduced bandwidth,
         * while SFP2_RATESEL will most likely be high (due to the PCB121 pullup on the signal)
         * and a RATESEL-aware SFP plug in the SFP2 slot will thus operate at full bandwidth.
         *
         * In FX100-mode cp/md are coming from the core logic, so bit3 in register 'DES_PHS_CTRL:bit-16'
         * must be set. If it is not set, the CDR is controlled from the core-logic but the
         * integrator gets its information from the DES directly, so both fight against each other.
         * And the input equalizer regulation(IB_REG_ENA) doesn't need for EQ in 100Mbit mode.
         * And bit2 in register 'DES_PHS_CTRL:bit-15' must set to 0.
         *   - 1: Integrative part of CDR loop is enabled if PCS logic is in sync
         *   - 0: Integrative part of CDR loop is always enabled.
         * Because the feedback from the PCS to the SerDes is not connected in Ferret.
         * The same setting need to apply for all the 1G-Serdes and 6G-Serdes interfaces.
         *
         * Theory:
         * The AC coupling caps inside the SFP module has a too small value, so that
         * the long frames introduce baseline wander on the serdes lanes (creates a DC offset).
         * This can cause the RX macro to loose track and lock.
         *
         * Solution: (Suit to Ferret/Ocelot/Serval-T/JR2)
         * Add the following new setting (based on the original configuration).
         * 1 .Drive SFP ports rate select (GPIO 18/19) to high
         * 2. 1G-Serdes:
         *      -- Change the default value from 1g_des_phs_ctrl=6 to 1g_des_phs_ctrl=2        
         *      -- For FX100_Mode: 
         *          . SERDES6G_ANA_CFG:SERDES1G_DES_CFG.DES_PHS_CTRL (16-13) = 10
         *          . SERDES1G_ANA_CFG:SERDES1G_IB_CFG.IB_ENA_CMV_TERM(13) = 0
         * 3. 6G-Serdes:
         *      -- Change the default value from 6g_des_phs_ctrl=6 to 6g_des_phs_ctrl=2
         *      -- For FX100_Mode:
         *          . SERDES6G_ANA_CFG:SERDES6G_DES_CFG.DES_PHS_CTRL (16-13) = 10
         *          . SERDES6G_ANA_CFG:SERDES6G_IB_CFG2.IB_UREG = 4 (default)
         *          . SERDES6G_ANA_CFG:SERDES6G_IB_CFG.IB_EQZ_ENA (1) = 0
         *          . SERDES6G_ANA_CFG:SERDES6G_IB_CFG.IB_REG_ENA (0) = 0
         *          . SERDES6G_ANA_CFG:SERDES6G_IB_CFG1.IB_FRC_<HP,MID,LP,OFFSET> = 1
         *          . SERDES6G_ANA_CFG:SERDES6G_IB_CFG1.IB_FILT_<HP,MID,LP,OFFSET> = 0
         *          . SERDES6G_ANA_CFG:SERDES6G_IB_CFG3.IB_INI_<HP,MID,LP,OFFSET> = <0,31,63,31>
         */
        des_cpmd_sel = 2;
        des_phs_ctrl = 10;
        ib_reg_ena = 0;
        ib_eqz_ena = 0;

        ib_frc_hp = 1;
        ib_frc_mid = 1;
        ib_frc_lp = 1;
        ib_frc_offset = 1;

        ib_filt_hp = 0;
        ib_filt_mid = 0;
        ib_filt_lp = 0;
        ib_filt_offset = 0;

        ib_ini_hp = 0;
        ib_ini_mid = 31;
        ib_ini_lp = 63;
        ib_ini_offset = 31;
        break;
    case VTSS_SERDES_MODE_1000BaseX:
        ob_ena1v_mode = 0;
        ob_post0 = 1;
        ob_ena_cas = 2;
        ob_lev = 48;
        ser_alisel = 0;
        ser_enali = 0;
        pll_ena_rot = 0;
        pll_fsm_ctrl_data = 60;
        pll_rot_frq = 0;
        qrate = 1;
        if_mode = 1;
        des_100fx_cpmd_ena = 0;
        break;
    case VTSS_SERDES_MODE_XAUI:
        ob_ena1v_mode = 0;
        ob_post0 = 0;
        ob_ena_cas = 0;
        ob_lev = 63;
        ser_alisel = 1;
        ser_enali = 1;
        pll_ena_rot = 1;
        pll_fsm_ctrl_data = 48;
        pll_rot_frq = 1;
        hrate = 1;
        qrate = 0;
        if_mode = 1;
        des_100fx_cpmd_ena = 0;
        break;
    case VTSS_SERDES_MODE_IDLE:
        ob_idle = 1;
        break;
    case VTSS_SERDES_MODE_TEST_MODE:
        test_mode = 2; // Fixed pattern
        test_pattern = 0x5f289;
        break;
    case VTSS_SERDES_MODE_DISABLE:
        ib_concur = 0;
        ena_lane = 0;
        ib_sig_det_ena = 0;
        ib_reg_ena = 0;
        ib_sam_ena = 0;
        ib_eqz_ena = 0;
        break;
    default:
#ifndef NDEBUG
        print_str("%% Calling h2_sd6g_cfg(): mode is unsupported, mode=0x");
        print_hex_b(mode);
        print_cr_lf();
#endif
        return;
    }

    h2_sd6g_read(addr);

    // prepare for initial or new configuration

    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_IDLE(ob_idle),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_IDLE);

    if (mode == VTSS_SERDES_MODE_IDLE) {
        /* Apply the idle mode (port down) and return */
        h2_sd6g_write(addr);
        return;
    }

    // Set test mode and test pattern
    H2_WRITE(VTSS_HSIO_SERDES6G_DIG_CFG_SERDES6G_TP_CFG0, test_pattern);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_DIG_CFG_SERDES6G_DFT_CFG0,
                    VTSS_F_HSIO_SERDES6G_DIG_CFG_SERDES6G_DFT_CFG0_TEST_MODE(test_mode),
                    VTSS_M_HSIO_SERDES6G_DIG_CFG_SERDES6G_DFT_CFG0_TEST_MODE);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_SYS_RST(0),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_SYS_RST);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_FSM_ENA(0),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_FSM_ENA);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_SIG_DET_ENA(ib_sig_det_ena) |
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_REG_ENA(ib_reg_ena) |
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_SAM_ENA(ib_sam_ena) |
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_EQZ_ENA(ib_eqz_ena) |
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_CONCUR(ib_concur) |
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_CAL_ENA(0),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_SIG_DET_ENA |
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_REG_ENA |
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_SAM_ENA |
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_EQZ_ENA |
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_CONCUR |
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_CAL_ENA);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG,
                    VTSS_F_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_LANE_RST(1),
                    VTSS_M_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_LANE_RST);
    h2_sd6g_write(addr);

    // apply configuration
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_ENA1V_MODE(ob_ena1v_mode),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_ENA1V_MODE);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_POST0(ob_post0),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_POST0);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_PREC(ob_prec),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_PREC);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1_OB_ENA_CAS(ob_ena_cas),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1_OB_ENA_CAS);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1_OB_LEV(ob_lev),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1_OB_LEV);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_DES_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_PHS_CTRL(des_phs_ctrl),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_PHS_CTRL);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_DES_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_CPMD_SEL(des_cpmd_sel),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_CPMD_SEL);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_DES_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_BW_ANA(des_bw_ana),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_BW_ANA);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_SIG_DET_CLK_SEL(0),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_SIG_DET_CLK_SEL);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_REG_PAT_SEL_OFFSET(0),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_REG_PAT_SEL_OFFSET);

#if 1 // BZ#21826
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_FRC_HP(ib_frc_hp) |
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_FRC_MID(ib_frc_mid) |
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_FRC_LP(ib_frc_lp) |
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_FRC_OFFSET(ib_frc_offset),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_FRC_HP |
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_FRC_MID |
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_FRC_LP |
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_FRC_OFFSET);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_FILT_HP(ib_filt_hp) |
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_FILT_MID(ib_filt_mid) |
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_FILT_LP(ib_filt_lp) |
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_FILT_OFFSET(ib_filt_offset),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_FILT_HP |
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_FILT_MID |
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_FILT_LP |
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_FILT_OFFSET);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG2,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG2_IB_UREG(ib_urge),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG2_IB_UREG);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG3,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG3_IB_INI_HP(ib_ini_hp) |
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG3_IB_INI_MID(ib_ini_mid) |
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG3_IB_INI_LP(ib_ini_lp) |
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG3_IB_INI_OFFSET(ib_ini_offset),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG3_IB_INI_HP |
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG3_IB_INI_MID |
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG3_IB_INI_LP |
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG3_IB_INI_OFFSET);
#endif

    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_TSDET(16),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_TSDET);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_SER_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_SER_CFG_SER_ALISEL(ser_alisel),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_SER_CFG_SER_ALISEL);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_SER_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_SER_CFG_SER_ENALI(ser_enali),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_SER_CFG_SER_ENALI);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_DIV4(pll_div4),
                     VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_DIV4);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_ENA_ROT(pll_ena_rot),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_ENA_ROT);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_FSM_CTRL_DATA(pll_fsm_ctrl_data),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_FSM_CTRL_DATA);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_ROT_DIR(pll_rot_dir),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_ROT_DIR);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_ROT_FRQ(pll_rot_frq),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_ROT_FRQ);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG,
                    VTSS_F_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_LANE_RST(1),
                    VTSS_M_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_LANE_RST);

#if 1 // Peter, 2016/07/18
    /* Configure Serdes6g TX/RX path to normal mode since the default is power down mode */
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_PWD_TX(0) |
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_PWD_RX(0),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_PWD_TX |
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_PWD_RX);
#endif

    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_SYS_RST(1),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_SYS_RST);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_ENA_LANE(ena_lane),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_ENA_LANE);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_HRATE(hrate),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_HRATE);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_QRATE(qrate),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_QRATE);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_IF_MODE(if_mode),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_IF_MODE);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG,
                    VTSS_F_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_DES_100FX_CPMD_ENA(des_100fx_cpmd_ena),
                    VTSS_M_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_DES_100FX_CPMD_ENA);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG,
                    VTSS_F_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_RX_LPI_MODE_ENA(rx_lpi_mode_ena),
                    VTSS_M_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_RX_LPI_MODE_ENA);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG,
                    VTSS_F_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_TX_LPI_MODE_ENA(tx_lpi_mode_ena),
                    VTSS_M_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_TX_LPI_MODE_ENA);
    h2_sd6g_write(addr);

    // Step 2: Set pll_fsm_ena=1
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_FSM_ENA(1),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_FSM_ENA);
    h2_sd6g_write(addr);

    // Step 3: Wait for 20 ms for PLL bringup
    delay_1(20);

    // Step 4: Start IB calibration by setting ib_cal_ena and clearing lane_rst
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_CAL_ENA(1),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_CAL_ENA);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG,
                    VTSS_F_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_LANE_RST(0),
                    VTSS_M_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_LANE_RST);
    h2_sd6g_write(addr);

    // Step 5: Wait for 60 ms for calibration
    delay_1(60);

    // Step 6:Set ib_tsdet and ib_reg_pat_sel_offset back to correct values
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_REG_PAT_SEL_OFFSET(0),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_REG_PAT_SEL_OFFSET);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_SIG_DET_CLK_SEL(ib_sig_det_clk_sel),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_SIG_DET_CLK_SEL);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_TSDET(3),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_TSDET);
    h2_sd6g_write(addr);
}
#endif

/**
 * Configures the Serdes1G/Serdes6G blocks based on mux mode
 * and Target.
 */
void h2_serdes_macro_config (void)
{
    /* Mux ports together with serdes macros */
    /*  00 - mode 0 - 3*QSGMII, 1*2G5, 1*SGMII */
    /*  01 - mode 1 - 2*2G5, 10*SGMII          */
    /*  10 - mode 2 - 2*QSGMII, 8*SGMII        */

#if defined(LUTON26_L25)
    H2_WRITE_MASKED(VTSS_DEVCPU_GCB_MISC_MISC_CFG,
                    VTSS_F_DEVCPU_GCB_MISC_MISC_CFG_SW_MODE(0UL),
                    VTSS_M_DEVCPU_GCB_MISC_MISC_CFG_SW_MODE);
    h2_sd6g_cfg(VTSS_SERDES_MODE_QSGMII, 0xE);  /* Enable QSGMII, Serdes6g (3-1) */
    h2_sd6g_cfg(VTSS_SERDES_MODE_SGMII, 0x1);   /* Enable SGMII, Serdes6g (0) */

#elif defined(LUTON26_L16)
#ifdef LUTON26_L16_QSGMII_EXT_PHY
    H2_WRITE_MASKED(VTSS_DEVCPU_GCB_MISC_MISC_CFG,
                    VTSS_F_DEVCPU_GCB_MISC_MISC_CFG_SW_MODE(0UL),
                    VTSS_M_DEVCPU_GCB_MISC_MISC_CFG_SW_MODE);
    h2_sd6g_cfg(VTSS_SERDES_MODE_QSGMII, 0xF);  /* Enable QSGMII, Serdes6g (3-0) */
#else
    H2_WRITE_MASKED(VTSS_DEVCPU_GCB_MISC_MISC_CFG,
                    VTSS_F_DEVCPU_GCB_MISC_MISC_CFG_SW_MODE(1UL),
                    VTSS_M_DEVCPU_GCB_MISC_MISC_CFG_SW_MODE);
    h2_sd6g_cfg(VTSS_SERDES_MODE_SGMII, 0xF);   /* Enable SGMII, Serdes6g (3-0) */
#endif /* LUTON26_L16_QSGMII_EXT_PHY */

#elif defined(LUTON26_L10)
    H2_WRITE_MASKED(VTSS_DEVCPU_GCB_MISC_MISC_CFG,
                    VTSS_F_DEVCPU_GCB_MISC_MISC_CFG_SW_MODE(1UL),
                    VTSS_M_DEVCPU_GCB_MISC_MISC_CFG_SW_MODE);
    h2_sd6g_cfg(VTSS_SERDES_MODE_SGMII, 0x3);   /* Enable SGMII, Serdes1g (1-0) */

#elif defined(VTSS_ARCH_OCELOT)
#if defined(FERRET_F11)
    h2_sd6g_cfg(VTSS_SERDES_MODE_QSGMII, 0x1);  /* DEV[4-7] - Configure SerDes6G_0 for QSGMII (mask 0x1) */
    h2_sd1g_cfg(VTSS_SERDES_MODE_SGMII, 0x10);  /* DEV[9]   - Configure interface macro: SerDes1G_4 for SGMII (mask 0x10) */
    h2_sd6g_cfg(VTSS_SERDES_MODE_2G5, 0x6);     /* DEV[8,10] - Configure SerDes6G_1 and SerDes6G_2 for 2G5 SGMII (mask 0x6) */
#endif // FERRET_F11

#if defined(FERRET_F10P)
    h2_sd6g_cfg(VTSS_SERDES_MODE_QSGMII, 0x1);  /* DEV[4-7] - Configure SerDes6G_0 for QSGMII (mask 0x1) */
    h2_sd6g_cfg(VTSS_SERDES_MODE_PCIE, 0x4);    /* DEV[10]  - Configure SerDes6G_2 for PCIe (mask 0x4) */
    h2_sd6g_cfg(VTSS_SERDES_MODE_2G5,  0x2);  /* DEV[8] - Configure SerDes6G_1 for 2G5 SGMII (mask 0x2) */
    h2_sd1g_cfg(VTSS_SERDES_MODE_SGMII, 0x10);  /* DEV[9]   - Configure interface macro: SerDes1G_4 for SGMII (mask 0x10) */
#endif // FERRET_F10P

#if defined(FERRET_F5)
    h2_sd6g_cfg(VTSS_SERDES_MODE_2G5, 0x4);     /* DEV[10] - Configure SerDes6G_2 for 2G5 SGMII (mask 0x4) */
#endif // FERRET_F5

#if defined(FERRET_F4P)
    h2_sd6g_cfg(VTSS_SERDES_MODE_PCIE, 0x4);    /* DEV[10]  - Configure SerDes6G_2 for PCIe (mask 0x4) */
#endif // FERRET_F4P

#endif
}

#if defined(VTSS_ARCH_LUTON26)
/* ************************************************************************ */
static void _h2_sd6g_cfg_100fx(ulong addr, vtss_cport_no_t chip_port)
/* ------------------------------------------------------------------------ --
 * Purpose     : Configures the sd6g 100fx mode
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    h2_sd6g_read(addr);

#if defined(VTSS_ARCH_LUTON26)
    H2_WRITE_MASKED(VTSS_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG,
                    VTSS_F_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_DES_100FX_CPMD_ENA,
                    VTSS_F_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_DES_100FX_CPMD_ENA);

    H2_WRITE_MASKED(VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1,
                    VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_FX100_ENA,
                    VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_FX100_ENA);

    H2_WRITE_MASKED(VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG,
                    VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_CPMD_SEL(2UL),
                    VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_CPMD_SEL);

#elif defined(VTSS_ARCH_OCELOT)
    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG,
                    VTSS_F_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_DES_100FX_CPMD_ENA(1),
                    VTSS_M_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_DES_100FX_CPMD_ENA);

    H2_WRITE_MASKED(VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_DES_CFG,
                    VTSS_F_HSIO_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_CPMD_SEL(2UL),
                    VTSS_M_HSIO_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_CPMD_SEL);
#endif

    h2_sd6g_write(addr);
}
#endif

/**
 * Serdes6G setup (Disable/2G5/QSGMII/SGMII).
 */
void h2_sd6g_cfg_change(vtss_serdes_mode_t mode, ulong addr)
{
#if defined(VTSS_ARCH_LUTON26)
    h2_sd6g_read(addr);
    delay_1(MSEC_20);
    h2_sd6g_cfg(mode, addr);
    h2_sd6g_write(addr);
    delay_1(10); // Wait until LCPLL stable again

    if (mode == VTSS_SERDES_MODE_100FX) {
        _h2_sd6g_cfg_100fx(addr);
    }

#elif defined(VTSS_ARCH_OCELOT)
    h2_sd6g_cfg(mode, addr);
#endif // VTSS_ARCH_LUTON26
}

#if defined(VTSS_ARCH_OCELOT)
#if defined(FERRET_F11) || defined(FERRET_F10P) || defined(FERRET_F5) || defined(FERRET_F4P)
void h2_sd1g_write(ulong addr)
{
    ulong dat;

    // EA = 0; // Disable interrupt while writing the date via serial Macro Configuration Bus (MCB)
    H2_WRITE(VTSS_HSIO_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG,
             VTSS_F_HSIO_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG_SERDES1G_ADDR(addr) |
             VTSS_F_HSIO_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG_SERDES1G_WR_ONE_SHOT(1));

    /* Wait until write operation is completed  */
    do {
        H2_READ(VTSS_HSIO_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG, dat);
    } while(VTSS_X_HSIO_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG_SERDES1G_WR_ONE_SHOT(dat));
    // EA = 1; // Enable interrupt
}

void h2_sd1g_read(ulong addr)
{
    ulong dat;

    // EA = 0; // Disable interrupt while reading the date via serial Macro Configuration Bus (MCB)
    H2_WRITE(VTSS_HSIO_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG,
             VTSS_F_HSIO_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG_SERDES1G_ADDR(addr) |
             VTSS_F_HSIO_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG_SERDES1G_RD_ONE_SHOT(1));

    /* Wait until write operation is completed  */
    do {
        H2_READ(VTSS_HSIO_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG, dat);
    } while(VTSS_X_HSIO_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG_SERDES1G_RD_ONE_SHOT(dat));
    // EA = 1; // Enable interrupt
}
#endif // FERRET_F11 || FERRET_F10P

#if defined(FERRET_F11) || defined(FERRET_F10P) || defined(FERRET_F5) || defined(FERRET_F4P)
void h2_sd1g_cfg(vtss_serdes_mode_t mode, ulong addr)
{
    /* The following code is based on the main trunk Serval implementation */
    BOOL ena_lane = 1, if_100fx = 0, ena_dc_coupling = 0, ob_idle = 0;
    u32  ob_amp_ctrl = 0, cpmd_sel=0, mbtr_ctrl=2, des_bw_ana=6, test_mode = 0, test_pattern = 0;
    u32 des_phs_ctrl = 2, ib_ena_cmv_term = 1; // BZ#21826

    switch (mode) {
    case VTSS_SERDES_MODE_SGMII:
        ob_amp_ctrl = 12;
        break;
    case VTSS_SERDES_MODE_100FX:
        ob_amp_ctrl = 12;
        if_100fx = 1;
        cpmd_sel = 2;
        mbtr_ctrl = 3;
        ena_dc_coupling = 1;
        des_bw_ana = 0;
        des_phs_ctrl = 10;     // BZ#21826
        ib_ena_cmv_term = 0;   // BZ#21826
        break;
    case VTSS_SERDES_MODE_1000BaseX:
        ob_amp_ctrl = 15;
        break;
    case VTSS_SERDES_MODE_DISABLE:
        ena_lane = 0;
        ob_amp_ctrl = 0;
        break;
    case VTSS_SERDES_MODE_IDLE:
        ob_idle = 1;
        break;
    case VTSS_SERDES_MODE_TEST_MODE:
        test_mode = 2; // Fixed pattern
        test_pattern = 0x5f289;
        break;
    default:
#ifndef NDEBUG
        print_str("%% Calling h2_sd6g_cfg(): mode is unsupported, mode=0x");
        print_hex_b(mode);
        print_cr_lf();
#endif
        return;
    }

    h2_sd1g_read(addr);

    H2_WRITE_MASKED(VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_SER_CFG,
                    VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_SER_CFG_SER_IDLE(ob_idle),
                    VTSS_M_HSIO_SERDES1G_ANA_CFG_SERDES1G_SER_CFG_SER_IDLE);

    if (mode == VTSS_SERDES_MODE_IDLE) {
        /* Apply the idle mode (port down) and return */
        h2_sd1g_write(addr);
        return;
    }

    /* 1. Configure macro, apply reset */
    /* Set test mode and test pattern */
    H2_WRITE(VTSS_HSIO_SERDES1G_DIG_CFG_SERDES1G_TP_CFG, test_pattern);
    H2_WRITE_MASKED(VTSS_HSIO_SERDES1G_DIG_CFG_SERDES1G_DFT_CFG0,
                    VTSS_F_HSIO_SERDES1G_DIG_CFG_SERDES1G_DFT_CFG0_TEST_MODE(test_mode),
                    VTSS_M_HSIO_SERDES1G_DIG_CFG_SERDES1G_DFT_CFG0_TEST_MODE);

    /* OB_CFG */
    H2_WRITE_MASKED(VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_OB_CFG,
                    VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_OB_CFG_OB_AMP_CTRL(ob_amp_ctrl),
                    VTSS_M_HSIO_SERDES1G_ANA_CFG_SERDES1G_OB_CFG_OB_AMP_CTRL);

    /* IB_CFG */
    H2_WRITE_MASKED(VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_IB_CFG,
                    VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_FX100_ENA(if_100fx) |
                    VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_ENA_CMV_TERM(ib_ena_cmv_term) |
                    VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_ENA_DC_COUPLING(ena_dc_coupling) |
                    VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_RESISTOR_CTRL(13),
                    VTSS_M_HSIO_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_FX100_ENA |
                    VTSS_M_HSIO_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_ENA_CMV_TERM |
                    VTSS_M_HSIO_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_ENA_DC_COUPLING |
                    VTSS_M_HSIO_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_RESISTOR_CTRL);

    /* DES_CFG */
    H2_WRITE_MASKED(VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_DES_CFG,
                    VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_PHS_CTRL(des_phs_ctrl) |
                    VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_CPMD_SEL(cpmd_sel) |
                    VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_MBTR_CTRL(mbtr_ctrl) |
                    VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_BW_ANA(des_bw_ana),
                    VTSS_M_HSIO_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_PHS_CTRL |
                    VTSS_M_HSIO_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_CPMD_SEL |
                    VTSS_M_HSIO_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_MBTR_CTRL |
                    VTSS_M_HSIO_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_BW_ANA);

    /* MISC_CFG */
    H2_WRITE_MASKED(VTSS_HSIO_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG,
                    VTSS_F_HSIO_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG_DES_100FX_CPMD_ENA(if_100fx) |
                    VTSS_F_HSIO_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG_LANE_RST(1),
                    VTSS_M_HSIO_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG_DES_100FX_CPMD_ENA |
                    VTSS_M_HSIO_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG_LANE_RST);

    /* PLL_CFG */
    H2_WRITE_MASKED(VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_PLL_CFG,
                    VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_PLL_CFG_PLL_FSM_ENA(1),
                    VTSS_M_HSIO_SERDES1G_ANA_CFG_SERDES1G_PLL_CFG_PLL_FSM_ENA);

    /* COMMON_CFG */
    H2_WRITE_MASKED(VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG,
                    VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_ENA_LANE(ena_lane),
                    VTSS_M_HSIO_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_ENA_LANE);
    h2_sd1g_write(addr);

    /* 2. Release PLL reset */
    H2_WRITE_MASKED(VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG,
                 VTSS_F_HSIO_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_SYS_RST(1),
                 VTSS_M_HSIO_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_SYS_RST);
    h2_sd1g_write(addr);

    /* 3. Release digital reset */
    H2_WRITE_MASKED(VTSS_HSIO_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG,
                    VTSS_F_HSIO_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG_LANE_RST(0),
                    VTSS_M_HSIO_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG_LANE_RST);
    h2_sd1g_write(addr);
}
#endif // FERRET_F11 || FERRET_F10P
#endif /* FERRET */

/* ************************************************************************ */
uchar h2_serdes_macro_phase_loop_locked (void)
/* ------------------------------------------------------------------------ --
 * Purpose     : Init PLL5G
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    ulong locked;

#if defined(VTSS_ARCH_LUTON26)
    const ulong code pll5g_cfg2 = VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG2;

    H2_WRITE(VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG4, 0x00007ae0);

    delay_1(2);
    H2_WRITE(pll5g_cfg2, 0x00610400);
    delay_1(2);
    H2_WRITE(pll5g_cfg2, 0x00610c00);
    delay_1(2);
    H2_WRITE(pll5g_cfg2, 0x00610800);
    delay_1(2);
    H2_WRITE(pll5g_cfg2, 0x00610000);
    delay_1(2);

    H2_READ(VTSS_MACRO_CTRL_PLL5G_STATUS_PLL5G_STATUS0, locked);
    return (locked & VTSS_F_MACRO_CTRL_PLL5G_STATUS_PLL5G_STATUS0_LOCK_STATUS);

#elif defined(VTSS_ARCH_OCELOT)
    /*
     * On the unmanaged Ferret board the LCPLL configuration (selecting the input
     * frequency) is done via strapping of GPIO 0, 1 and 3, and the LCPLL will
     * read the value of these pins while reset is applied.
     * This "should" cause the startup to be identical independent of if the board
     * is powered up or reset is applied afterwards, but when powering up the board
     * initially the input clock might not be running, so some of the initial
     * calibration/biasing might be generating a different results than when
     * applying a reset after power up.
     * When reset is applied after power up the LCPLL might also see the REFCLK_SEL
     * signals toggling as the pins are used as output in the application code
     * and it will take a little time for them to reach their final stage (controlled
     * by the pin strapping).
     *
     * In order to get the LCPLL to operate with optimum performance there are a
     * number of register writes that must be performed after power up
     * (documented as part of the EVE procedures)
     *
     * The following code is refer to main trunk
     * vtss_api\base\ail\vtss_pll5g_procs.c
     * vtss_api\base\serval\vtss_ocelot_pll5g_setup.c
     */
    // +++ Start of the EVE procedures
    H2_WRITE_MASKED(VTSS_HSIO_PLL5G_CFG_PLL5G_CFG4,
                    VTSS_F_HSIO_PLL5G_CFG_PLL5G_CFG4_IB_CTRL(0x7600),
                    VTSS_M_HSIO_PLL5G_CFG_PLL5G_CFG4_IB_CTRL);
    H2_WRITE_MASKED(VTSS_HSIO_PLL5G_CFG_PLL5G_CFG0,
                    0,
                    VTSS_M_HSIO_PLL5G_CFG_PLL5G_CFG0_ENA_VCO_CONTRH);
    H2_WRITE_MASKED(VTSS_HSIO_PLL5G_CFG_PLL5G_CFG0,
                    VTSS_F_HSIO_PLL5G_CFG_PLL5G_CFG0_LOOP_BW_RES(14),
                    VTSS_M_HSIO_PLL5G_CFG_PLL5G_CFG0_LOOP_BW_RES);
    H2_WRITE_MASKED(VTSS_HSIO_PLL5G_CFG_PLL5G_CFG0,
                    VTSS_F_HSIO_PLL5G_CFG_PLL5G_CFG0_SELBGV820(4),
                    VTSS_M_HSIO_PLL5G_CFG_PLL5G_CFG0_SELBGV820);

    /* Toggle DISABLE_FSM to re-trigger the Startup-FSM of the LCPLL */
    H2_WRITE_MASKED(VTSS_HSIO_PLL5G_CFG_PLL5G_CFG2,
                    VTSS_F_HSIO_PLL5G_CFG_PLL5G_CFG2_DISABLE_FSM(1),
                    VTSS_M_HSIO_PLL5G_CFG_PLL5G_CFG2_DISABLE_FSM);
    delay_1(2);
    H2_WRITE_MASKED(VTSS_HSIO_PLL5G_CFG_PLL5G_CFG2,
                    VTSS_F_HSIO_PLL5G_CFG_PLL5G_CFG2_DISABLE_FSM(0),
                    VTSS_M_HSIO_PLL5G_CFG_PLL5G_CFG2_DISABLE_FSM);
    delay_1(50); // Wait for 50 ms for end of the EVE procedures
    // +++ End of the EVE procedures

    H2_READ(VTSS_HSIO_PLL5G_STATUS_PLL5G_STATUS0, locked);
    return (VTSS_X_HSIO_PLL5G_STATUS_PLL5G_STATUS0_LOCK_STATUS(locked));
#endif
}
