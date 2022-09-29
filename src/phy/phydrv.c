//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */

#include "timer.h"

/** todo: move switch register access out of phydrv.c */
#include "h2io.h"
#include "vtss_api_base_regs.h"

#include "phydrv.h"
#include "phymap.h"
#include "misc2.h"
#include "veriphy.h"

#include "phy_base.h"
#include "phy_family.h"

#if defined(PHYDRV_DEBUG_ENABLE)
#include "print.h"
#endif /* PHYDRV_DEBUG_ENABLE */
#include "print.h"

/****************************************************************************
 *
 *
 * Defines
 *
 *
 ****************************************************************************/

#if LOOPBACK_TEST
#define __PHY_RESET__
#endif

#if PERFECT_REACH_LNK_UP
#define WORST_SUBCHAN_MSE 120
#define WORST_AVERAGE_MSE 100
#endif

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
#if 0   // Uncalled function
static uchar code ecpdset[] = { 0, 5, 9, 12, 14 };
#define NUM_ECPD_SETTINGS (sizeof(ecpdset)/sizeof(ecpdset[0]))
#endif  // Uncalled function

/*****************************************************************************
 *
 *
 * Local functions
 *
 *
 *
 ****************************************************************************/

#ifdef __PHY_RESET__
static void assert_reset (vtss_port_no_t port_no) {
    uchar timeout;

    // Work around of bugzilla#2521
    phy_page_std(port_no);
    phy_write(port_no, 0, 0x1000);
    delay_1(4); /* wait 4 msec before reseting */


    phy_write(port_no, 0, 0x8000);
    phy_write(port_no, 31, 0x0000);
    delay_1(2); /* wait 1-2 msec before accessing registers */
    timeout = 0;
    while (phy_read(port_no, 0) & 0x8000) {
        if (++timeout > 200) {
            break; /* should not occur */
        }
        delay_1(1);
    }
}
#endif /* __PHY_RESET__ */

#if PERFECT_REACH_LNK_UP
/**
 * Computes optimal power setting level reductions based on calculated noise
 * values from the specific cable being used.
 *
 * @see vtss_phy_power_opt
 */
static void phy_power_optimizer (vtss_port_no_t port_no)
{
    uchar ncpd, ecpd_idx;
    uchar half_adc;
    ushort reg17;
    phy_id_t phy_id;
    ushort maxMse, meanMse, done;
    ecpd_idx = 0;
    ncpd = 1;
    phy_read_id(port_no, &phy_id);
    if ((phy_id.family == VTSS_PHY_FAMILY_ATOM)    ||
        (phy_id.family == VTSS_PHY_FAMILY_LUTON26) ||
        (phy_id.family == VTSS_PHY_FAMILY_TESLA)   ||
        (phy_id.family == VTSS_PHY_FAMILY_ELISE))
    {
        half_adc = 1;
    } else {
        half_adc = 0;
    }

    phy_page_tr (port_no);
    phy_write (port_no, 16, 0xa3aa);
    done = phy_read (port_no, 17);
    phy_write (port_no, 18, 0x0003);
    phy_write (port_no, 17, done);
    phy_write (port_no, 16, 0x83aa);

    done = 0;

    while (done < 2) {
        phy_page_tp (port_no);
        phy_write_masked (port_no, 12, ((ushort)ecpdset[ecpd_idx] << 12)
                          | ((ushort)ncpd << 10), 0xfC00);
        if ((phy_id.family == VTSS_PHY_FAMILY_ATOM)  ||
            (phy_id.family == VTSS_PHY_FAMILY_LUTON26)  ||
            (phy_id.family == VTSS_PHY_FAMILY_TESLA) ||
            (phy_id.family == VTSS_PHY_FAMILY_ELISE))
        {
            phy_page_tr(port_no);
            phy_write(port_no, 16, 0xafe4);
            reg17 = phy_read(port_no, 17);
            reg17 = (reg17 & 0xffef) | ((half_adc & 1) << 4);
            phy_write(port_no, 17, reg17);
            phy_write(port_no, 16, 0x8fe4);
        }
        if (done != 0) {
            if ((short)done < 0) {
                delay (MSEC_50);
            }
            ++done;
        } else {
            phy_page_tr (port_no);
            phy_write (port_no, 16, 0xa3c0);
            done = phy_read (port_no, 17);
            maxMse = done & 0x0fff;
            done = (done >> 12) | (phy_read (port_no, 18) << 4);
            meanMse = maxMse + done;
            if (done > maxMse)
                maxMse = done;

            phy_write (port_no, 16, 0xa3c2);
            done = phy_read (port_no, 17);
            meanMse += (done & 0x0fff);
            if ((done & 0x0fff) > maxMse)
                maxMse = done & 0x0fff;
            done = (done >> 12) | (phy_read (port_no, 18) << 4);
            meanMse += done;
            if (done > maxMse)
                maxMse = done;

            meanMse /= 4;
            done = 0;
            if ((maxMse  >= WORST_SUBCHAN_MSE) ||
                    (meanMse >= WORST_AVERAGE_MSE)) {
                if (ecpd_idx == 0) {
                    ncpd--;
                    done = (ushort)-1;
                    ecpd_idx = 1;
                } else {
                    ecpd_idx--;
                    done = 1;
                }
            } else if ((ecpd_idx == 0) && (ncpd < 2)) {
                ncpd++;
            } else if (++ecpd_idx >= NUM_ECPD_SETTINGS) {
                done = 2;
            }
        }
    }

    phy_page_tr (port_no);
    phy_write (port_no, 16, 0xa3aa);
    done = phy_read (port_no, 17);
    phy_write (port_no, 18, 0x0000);
    phy_write (port_no, 17, done);
    phy_write (port_no, 16, 0x83aa);
    phy_page_std (port_no);
}
#endif  /* PERFECT_REACH_LNK_UP */

/**
 * Await completion of a MIIM command.
 */
static void phy_await_completed (uchar miim_no) small
{
    ulong dat;

    /* wait until data ready or timeout */
#if defined(VTSS_ARCH_OCELOT)
    do {
        H2_READ(VTSS_DEVCPU_GCB_MIIM_MII_STATUS(miim_no), dat);
    } while(VTSS_X_DEVCPU_GCB_MIIM_MII_STATUS_MIIM_STAT_BUSY(dat));

#elif defined(VTSS_ARCH_LUTON26)
    do {
        H2_READ(VTSS_DEVCPU_GCB_MIIM_MII_STATUS(miim_no), dat);
    } while(dat & VTSS_F_DEVCPU_GCB_MIIM_MII_STATUS_MIIM_STAT_BUSY);
#endif
}

#if VTSS_QUATTRO || VTSS_SPYDER || VTSS_ELISE || VTSS_TESLA || VTSS_ATOM12
static void phy_receiver_init (vtss_port_no_t port_no)
{
#if PERFECT_REACH_LNK_UP
    ushort reg17;
#endif
    phy_id_t phy_id;
    phy_read_id(port_no, &phy_id);
    phy_page_tp(port_no);
    phy_write_masked(port_no, 12, 0x0200, 0x0300);
#if PERFECT_REACH_LNK_UP
    phy_write_masked(port_no, 12, 0x0000, 0xfc00);
    if ((phy_id.family == VTSS_PHY_FAMILY_ATOM)    ||
        (phy_id.family == VTSS_PHY_FAMILY_LUTON26) ||
        (phy_id.family == VTSS_PHY_FAMILY_TESLA)   ||
        (phy_id.family == VTSS_PHY_FAMILY_ELISE))
    {
        phy_page_tr(port_no);
        phy_write(port_no, 16, 0xafe4);
        reg17 = phy_read(port_no, 17);
        reg17 = (reg17 & 0xffef); //Clear half_adc as desired
        phy_write(port_no, 17, reg17);
        phy_write(port_no, 16, 0x8fe4);
    } else
    if (1) {
        phy_write_masked(port_no, 24, 0x2000, 0x2000);
    }
#endif  /* PERFECT_REACH_LNK_UP */
    phy_page_std(port_no);
}
#endif  // VTSS_QUATTRO || VTSS_SPYDER || VTSS_ELISE || VTSS_TESLA || VTSS_ATOM12

#if TRANSIT_EEE
/**
 * Write to a MMD register (clause 45 read-modify mask write)
 */
static void vtss_phy_mmd_reg_wr_masked(const vtss_port_no_t port_no,
                                       const u16            devad,
                                       const u16            addr,
                                       const u16            data_val,
                                       const u16            mask)
{
    u16     current_reg_val;
    u16     new_reg_val;

    // Read current value of the register
    current_reg_val = phy_mmd_rd(port_no, devad, addr);

    // Modify current value and write.
    new_reg_val     = (current_reg_val & ~mask) | (data_val & mask) ;
    phy_write(port_no, 14, new_reg_val);
}
#endif // TRANSIT_EEE

static BOOL _phy_read_reg_data(vtss_cport_no_t chip_port, uchar reg_no, ushort *phy_data) small
{
    ulong  reg_data;
    uchar  miim_no = phy_map_miim_no(chip_port);

    /* Addressing part */
    reg_data =
#if defined(VTSS_ARCH_OCELOT)
    VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_VLD(1) | /* Valid command */
#elif defined(VTSS_ARCH_LUTON26)
    VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_VLD | /* Valid command */
#endif
    VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_REGAD(reg_no) | /* Register address */
    VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_PHYAD(phy_map_phy_no(chip_port))| /* Phy/port address */
    VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_OPR_FIELD(2); /* Read op, Clause 22 */

    /* Enqueue MIIM operation to be executed */
    H2_WRITE(VTSS_DEVCPU_GCB_MIIM_MII_CMD(miim_no), reg_data);

    /* Wait for MIIM operation to finish */
    phy_await_completed(miim_no);

    H2_READ(VTSS_DEVCPU_GCB_MIIM_MII_DATA(miim_no), reg_data);
#if defined(VTSS_ARCH_OCELOT)
    if (VTSS_X_DEVCPU_GCB_MIIM_MII_DATA_MIIM_DATA_SUCCESS(reg_data))
#elif defined(VTSS_ARCH_LUTON26)
    if (dat & VTSS_F_DEVCPU_GCB_MIIM_MII_DATA_MIIM_DATA_SUCCESS(3))
#endif
    {
#if defined(PHYDRV_DEBUG_ENABLE)
        print_str("%% Error: phy_read() failed, chip_port=");
        print_dec(chip_port);
        print_str(" reg_no=");
        print_dec(reg_no);
        print_cr_lf();
#endif // PHYDRV_DEBUG_ENABLE

        return 1; // Read failed
    }

    *phy_data = (ushort) VTSS_X_DEVCPU_GCB_MIIM_MII_DATA_MIIM_DATA_RDDATA(reg_data);
    return 0;
}

/*****************************************************************************
 *
 *
 * Public functions
 *
 *
 *
 ****************************************************************************/
/**
 * Read a PHY register.
 *
 * @param port_no   The port number to which the PHY is attached.
 * @param reg_no    the PHY register number (0-31).
 */
ushort phy_read (vtss_port_no_t port_no, uchar reg_no) small
{
    ushort phy_data;

    if (!phy_map(port_no)) {
#if defined(PHYDRV_DEBUG_ENABLE)
        print_str("%% Error: Wrong parameter when calling phy_read(), chip_port=");
        print_dec(port_no);
        print_str(" reg_no=");
        print_dec(reg_no);
        print_cr_lf();
#endif // PHYDRV_DEBUG_ENABLE
        return 0;
    }

    // EA = 0; // Disable interrupt while reading the PHY register value (indirect access).
    while(_phy_read_reg_data(port_no, reg_no, &phy_data)); // Infinite loop until read success
    // EA = 1; // Enable interrupt

    return phy_data;
}

/**
 * Write to a PHY register.
 *
 * @param port_no   The port number to which the PHY is attached.
 * @param reg_no    The PHY register number (0-31).
 * @param value     Value to be written.
 */
void phy_write(vtss_port_no_t   port_no,
               uchar            reg_no,
               ushort           value) small
{
    uchar  miim_no;
    ulong  dat;

    if (!phy_map(port_no)) {
#if defined(PHYDRV_DEBUG_ENABLE)
        print_str("%% Error: Wrong parameter when calling phy_write(), port_no=");
        print_dec(port_no);
        print_str(" reg_no=");
        print_dec(reg_no);
        print_cr_lf();
#endif // PHYDRV_DEBUG_ENABLE
        return;
    }

    miim_no = phy_map_miim_no(port_no);

    // EA = 0; // Disable interrupt while writing the PHY register value (indirect access).

#if defined(VTSS_ARCH_OCELOT)
    dat = VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_OPR_FIELD(1) | /* Write op */
          VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_WRDATA(value) | /* value */
          VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_VLD(1) | /* Valid command */
          VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_REGAD(reg_no) | /* Register address */
          VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_PHYAD(phy_map_phy_no(port_no)); /* Phy/port address */

#elif defined(VTSS_ARCH_LUTON26)
    dat = VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_OPR_FIELD(1) | /* Write op */
          VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_WRDATA(value) | /* value */
          VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_VLD | /* Valid command */
          VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_REGAD(reg_no) | /* Register address */
          VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_PHYAD(phy_map_phy_no(port_no)); /* Phy/port address */
#endif

    /* Enqueue MIIM operation to be executed */
    H2_WRITE(VTSS_DEVCPU_GCB_MIIM_MII_CMD(miim_no), dat);

    /* Wait for MIIM operation to finish */
    phy_await_completed(miim_no);

    // EA = 1; // Enable interrupt
}

/**
 * Update specified bit(s) of a PHY register.
 *
 * @param port_no   The port number to which the PHY is attached.
 * @param reg_no    The PHY register number (0-31).
 * @param value     Holds bits (in right positions) to be written.
 * @param mask      Bit mask specifying the bits to be updated.
 *
 * Example: To set AN_EN and Restart_AN bits in register 0, write:
 *          phy_write_masked(miim_no, phy_no, 0, 0x1200, 0x1200);
 */
void phy_write_masked(vtss_port_no_t port_no,
                      uchar          reg_no,
                      ushort         value,
                      ushort         mask) small
{
    phy_write(port_no, reg_no, (phy_read(port_no, reg_no) & ~mask) | (value & mask));
}

int phy_mmd_rd(const vtss_port_no_t port_no,
               const u16            devad,
               const u16            addr)
{
    phy_page_std(port_no);
    phy_write(port_no, 13, devad); // Setup cmd=address + devad address
    phy_write(port_no, 14, addr);  // Setup address
    phy_write(port_no, 13, (1 << 14) + devad); // Setup cmd=data + devad address

    return phy_read(port_no, 14);  // read data
}

void phy_page_std (vtss_port_no_t port_no) {
    phy_write(port_no, 31, 0);
}

#if VTSS_ATOM12_B || VTSS_ATOM12_C || VTSS_ATOM12_D || VTSS_TESLA
void phy_page_ext (vtss_port_no_t port_no) {
    phy_write(port_no, 31, 1);
}
#endif  // VTSS_ATOM12_B || VTSS_ATOM12_C || VTSS_ATOM12_D || VTSS_TESLA

#if VTSS_ATOM12_B || VTSS_ATOM12_C || VTSS_ATOM12_D
void phy_page_ext2 (vtss_port_no_t port_no) {
    phy_write(port_no, 31, 2);
}
#endif // VTSS_ATOM12_B || VTSS_ATOM12_C || VTSS_ATOM12_D

#if VTSS_QUATTRO || VTSS_SPYDER || VTSS_ELISE || VTSS_TESLA || VTSS_ATOM12
void phy_page_gp (vtss_port_no_t port_no) {
    phy_write(port_no, 31, GP_PAGE_CODE);
}
#endif // VTSS_QUATTRO || VTSS_SPYDER || VTSS_ELISE || VTSS_TESLA || VTSS_ATOM12

void phy_page_tp (vtss_port_no_t port_no) {
    phy_write(port_no, 31, TP_PAGE_CODE);
}

void phy_page_tr (vtss_port_no_t port_no) {
    phy_write(port_no, 31, TR_PAGE_CODE);
}

/* Fixme: Tune the two functions to fit VSC8522/12 */
uchar phy_get_speed_and_fdx (vtss_port_no_t port_no)
{
    uchar speed_fdx_mode;
    ushort reg_val;
#if 0
    phy_id_t phy_id;

    phy_read_id(port_no, &phy_id);

    if (phy_id.vendor != PHY_VENDOR_VTSS) {
        /* Get speed and duplex mode into speed_fdx_mode variable */
        PHY_READ_SPEED_AND_FDX(port_no, reg_val, speed_fdx_mode);
        return speed_fdx_mode;
    }
#endif

    /* Get info about speed and duplex mode from PHY reg. 28 */
    reg_val = phy_read(port_no, 28);

    /* set speed field (bit 1:0) = bit 4:3 of PHY reg. */
    speed_fdx_mode = ((uchar) reg_val >> 3) & 0x03;

    /* update full duplex bit */
    if (reg_val & 0x20) {
        speed_fdx_mode |= LINK_MODE_FDX_MASK;
    }

#if PHY_HDX_FLOW_CTRL_MODE
    if ((reg_val & 0x20) == 0) {
        speed_fdx_mode |= LINK_MODE_PAUSE_MASK;
    }
#endif /* PHY_HDX_FLOW_CTRL_MODE */

#if PHY_AN_FAIL_FLOW_CTRL_MODE
    /* Enable flow control support when LP doesn't have auto-nego  */
    if ((reg_val & 0x8000) && (phy_read(port_no, 6) & 0x1) == 0) {
        speed_fdx_mode |= LINK_MODE_PAUSE_MASK;
    }
#endif /* PHY_AN_FAIL_FLOW_CTRL_MODE */

    return speed_fdx_mode;
}

#if LOOPBACK_TEST
/**
 * Set forced speed.
 */
void phy_set_forced_speed (vtss_port_no_t port_no, uchar link_mode)
{
    ushort val;

    if (link_mode & LINK_MODE_INT_LOOPBACK) {
        /* temporary */
        phy_write(port_no, 0, 0);
        /* temporary ends */

        if ((link_mode & LINK_MODE_SPEED_MASK) == LINK_MODE_SPEED_10) {
            val = 0x0000;
        } else if ((link_mode & LINK_MODE_SPEED_MASK) == LINK_MODE_SPEED_100) {
            val = 0x2000;
        } else if ((link_mode & LINK_MODE_SPEED_MASK) == LINK_MODE_SPEED_1000) {
            val = 0x0040;
        }
        if (link_mode & LINK_MODE_FDX_MASK) {
            val |= 0x100;
        }
        if (link_mode & LINK_MODE_INT_LOOPBACK) {
            val |= 0x4000;
        }

        phy_write(port_no, 0, val);
    } else {
        if ((link_mode & LINK_MODE_SPEED_MASK) == LINK_MODE_SPEED_10) {
            /* Update register 4 with 10 advertising, plus pause capability */
            phy_write(port_no, 4, 0x0161);
            /*  Update register 9 to disable 1000 Mbps advertising */
            phy_write(port_no, 9, 0x0400);
        } else if ((link_mode & LINK_MODE_SPEED_MASK) == LINK_MODE_SPEED_100) {
            /* Update register 4 with 100 advertising, plus pause capability */
            phy_write(port_no, 4, 0x0581);
            /*  Update register 9 to disable 1000 Mbps advertising */
            phy_write(port_no, 9, 0x0400);
        } else if ((link_mode & LINK_MODE_SPEED_MASK) == LINK_MODE_SPEED_1000) {
            /* Update register 4 with 100 advertising, plus pause capability */
            phy_write(port_no, 4, 0x0101);
            /*  Update register 9 to disable 1000 Mbps advertising */
            phy_write(port_no, 9, 0x0600);
        }
        /* Restart auto-negotiation */
        phy_restart_aneg(port_no);
    }
}
#endif

/* ************************************************************************ */
bool phy_link_status (vtss_cport_no_t chip_port) small
/* ------------------------------------------------------------------------ --
 * Purpose     : Read link status bit from PHY.
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    return ((phy_read(chip_port, 1) & 0x0004) != 0);
}

/**
 * Do any PHY settings after link transition to up.
 *
 * @see vtss_phy_status_get_private()
 */
void phy_do_link_up_settings (vtss_cport_no_t chip_port,
                              uchar           link_mode,
                              uchar            power_mode)
{
#if VTSS_SPYDER
    phy_id_t phy_id;
#endif // VTSS_SPYDER

    /* Possibly disable echo-mode on PHY when running half-duplex at 10M */
    if ((link_mode & (LINK_MODE_SPEED_MASK | LINK_MODE_FDX_MASK)) == LINK_MODE_SPEED_10)
        {
        PHY_DISABLE_ECHO_MODE(chip_port);
    } else {
        PHY_ENABLE_ECHO_MODE(chip_port);
    }

    if ((link_mode & LINK_MODE_SPEED_MASK) == LINK_MODE_SPEED_1000)
        phy_receiver_reconfig(chip_port, power_mode);

#if VTSS_SPYDER
    /* Read PHY id to determine action */
    phy_read_id(chip_port, &phy_id); 
    if ((phy_id.family == VTSS_PHY_FAMILY_SPYDER) && (phy_id.revision == 0)) {

        uchar rx_tr_lock;

        phy_page_tr(chip_port);
        phy_write(chip_port, 16, 0xa60c);
        phy_write(chip_port, 16, 0xa60c);
        rx_tr_lock = (phy_read(chip_port, 17) >> 3) & 0x01;
        if (!rx_tr_lock) {
            phy_write(chip_port, 17, 0x0010);
            phy_write(chip_port, 16, 0x8604);
            phy_write(chip_port, 17, 0x00df);
            phy_write(chip_port, 16, 0x8600);
            phy_write(chip_port, 17, 0x00ff);
            phy_write(chip_port, 16, 0x8600);
            phy_write(chip_port, 17, 0x0000);
            phy_write(chip_port, 16, 0x8604);
            phy_write(chip_port, 16, 0xa60c);
            phy_write(chip_port, 16, 0xa60c);
            rx_tr_lock = (phy_read(chip_port, 17) >> 3) & 0x01;
            if (!rx_tr_lock) {
            }
        }
        phy_page_std(chip_port);
    }
#endif /* VTSS_SPYDER */
}

/**
 * Do any PHY settings after link transition to down.
 */
void phy_do_link_down_settings (vtss_port_no_t port_no)
{
    phy_write_masked(port_no, 0, 0x800, 0x800);
    delay_1(2);
    phy_write_masked(port_no, 0, 0x000, 0x800);
}

/**
 * Set the PHY in power down mode
 */
void phy_power_down (vtss_port_no_t port_no)
{
    phy_write_masked(port_no, 0, 0x800, 0x800);
}

/**
 * @see Reference vtss_phy_optimize_receiver_reconfig()
 */
void phy_receiver_reconfig(vtss_cport_no_t chip_port, uchar power_mode)
{
    uchar       vga_state_a;
    short       max_vga_state_to_optimize;
    phy_id_t    phy_id;

    phy_read_id(chip_port, &phy_id);

    switch (phy_id.family) {
    case VTSS_PHY_FAMILY_ATOM:
    case VTSS_PHY_FAMILY_LUTON26:
    case VTSS_PHY_FAMILY_TESLA:
    case VTSS_PHY_FAMILY_ELISE:
    case VTSS_PHY_FAMILY_FERRET:
        /*
         * 65nm PHY adjusted VGA window to more effectively use dynamic range
         * as a result, VGA gains for a given cable length are higher here.
         */
        max_vga_state_to_optimize =  -9;
        break;
    default:
        max_vga_state_to_optimize = -12;
        break;
    }

    phy_page_tr(chip_port);
    phy_write(chip_port, 16, 0xaff0);
    vga_state_a = (phy_read(chip_port, 17) >> 4) & 0x01f;
    if ((vga_state_a < 16) || (vga_state_a > (max_vga_state_to_optimize & 0x1f)))
    {
        phy_page_tp(chip_port);
        phy_write_masked(chip_port, 12, 0x0000, 0x0300);
    }
#if PERFECT_REACH_LNK_UP
    else {
        power_mode = 0;
            if (!(phy_id.family == VTSS_PHY_FAMILY_ATOM)    &&
                !(phy_id.family == VTSS_PHY_FAMILY_LUTON26) &&
                !(phy_id.family == VTSS_PHY_FAMILY_TESLA)   &&
                !(phy_id.family == VTSS_PHY_FAMILY_ELISE)   &&
                !(phy_id.family == VTSS_PHY_FAMILY_FERRET))
            {
                phy_page_tp(chip_port);
                phy_write_masked(chip_port, 24, 0x0000, 0x2000);
            }
            phy_power_optimizer(chip_port);
    }
#else
    power_mode = power_mode;    // Quiet Kail compile warning
#endif /* PERFECT_REACH_LNK_UP */

    phy_page_std(chip_port);
}

/**
 * Restart auto-negotiation on the PHY connected to the specified port.
 */
void phy_restart_aneg (vtss_port_no_t port_no)
{
    /* start autonegotiation by writing to register 0 */
    phy_write_masked(port_no, 0, 0x1200, 0x1200);
    delay_1(2);
}

/* ------------------------------------------------------------------------ --
 * Purpose     : Do the necessary hardware configuration of a PHY before it
 *               has been reset.
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
void phy_pre_reset (vtss_port_no_t port_no)
{
    phy_id_t phy_id;

    /* Read PHY id to determine action */
    phy_read_id(port_no, &phy_id);

    switch (phy_id.family) {
#if VTSS_COBRA
    case VTSS_PHY_FAMILY_COBRA:
        // COBRA not necessary for pre_reset
        // cobra_init_seq(port_no);
        break;
#endif
#if VTSS_ATOM12
    case VTSS_PHY_FAMILY_LUTON26:
        atom12_init_seq_pre(port_no, &phy_id);
        break;
    case VTSS_PHY_FAMILY_ATOM:
        atom12_init_seq_pre(port_no, &phy_id);
        break;
#endif
#if VTSS_TESLA
    case VTSS_PHY_FAMILY_TESLA:
        tesla_init_seq_pre(port_no, &phy_id);
        break;
#endif
#if VTSS_ELISE
    case VTSS_PHY_FAMILY_ELISE:
        elise_init_seq_pre(port_no, &phy_id);
        break;
#endif
    default:
        // No pre-initialising needed
        break;
    }
}

#ifdef __PHY_RESET__
/**
 * Perform a reset of the PHY attached to the specified port.
 */
void phy_reset (vtss_port_no_t port_no)
{
    assert_reset(port_no);
    phy_post_reset(port_no);
}
#endif  // __PHY_RESET__

/**
 * Do the necessary hardware configuration of a PHY after it has been reset.
 *
 * @See vtss_phy_reset_private() in API.
 */
void phy_post_reset (vtss_port_no_t port_no)
{
    phy_id_t    phy_id;

    /* Read PHY id to determine action */
    phy_read_id(port_no, &phy_id);

    switch (phy_id.family) {
#if VTSS_COBRA
    case VTSS_PHY_FAMILY_COBRA:
        cobra_init_seq(port_no);
        break;
#endif // VTSS_COBRA

#if VTSS_ATOM12
    case VTSS_PHY_FAMILY_LUTON26:
        atom12_init_seq(port_no, &phy_id);

#if defined(LUTON26_L25)
        // Release COMA_MODE after both internal and external PHYs are configured
        //     If COMA_MODE pin is connected between Luton26 and Atom12, only do
        //     this on one device; otherwise, you'll need to do it on both or use
        //     some other means to control the COMA_MODE pin
        phy_write(port_no, 31, 0x10); // Change page
        phy_write(port_no, 14, 0x800); // Force coma pin
#endif /* LUTON26_L25 */

        phy_write(port_no, 31, 0); // Change page back to standard page

        phy_receiver_init(port_no);
        break;

    case VTSS_PHY_FAMILY_ATOM:
#if 0   // Uncalled function
        if(!pll5g_locked && port_no == 12) {
            pll5g_locked = init_atom_phase_loop_locked(port_no);
            if(pll5g_locked) {
                print_str("locked on atom12");
            } else {
                print_str("unlocked on atom12");
            }
        }
#endif  // Uncalled function
        atom12_init_seq(port_no, &phy_id);
        phy_receiver_init(port_no);
        break;
#endif /* VTSS_ATOM12 */

#if VTSS_TESLA
    case VTSS_PHY_FAMILY_TESLA:
        tesla_init_seq(port_no, &phy_id);
        phy_receiver_init(port_no);
        break;
#endif // VTSS_TESLA

#if VTSS_ELISE
    case VTSS_PHY_FAMILY_ELISE: {
        elise_init_seq(port_no, &phy_id);

#if defined(LUTON26_L16) && defined(LUTON26_L16_QSGMII_EXT_PHY)
        if (port_no == 15) {
            // Release COMA_MODE after both internal and external PHYs are configured
            //     If COMA_MODE pin is connected between Luton26 and Atom12, only do
            //     this on one device; otherwise, you'll need to do it on both or use
            //     some other means to control the COMA_MODE pin
            phy_write(port_no, 31, 0x10); // Change page
            phy_write(port_no, 14, 0x800); // Force coma pin
            phy_write(port_no, 31, 0); // Change page back to standard page        phy_receiver_init(port_no);
        }
#endif /* LUTON26_L16 && LUTON26_L16_QSGMII_EXT_PHY */

        phy_write(port_no, 31, 0x10); // Change page
        phy_write_masked(port_no, 14, 0x00,0x3000); // Force coma pin
        phy_write(port_no, 31, 0); // Change page back to standard page        phy_receiver_init(port_no);
        phy_receiver_init(port_no);
        break;
    }
#endif // VTSS_ELISE

#if VTSS_SPYDER
    case VTSS_PHY_FAMILY_SPYDER: {
        if ((phy_id.model == PHY_MODEL_VTSS_8558) || (phy_id.model == PHY_MODEL_VTSS_8658)) {
        }
        init_seq_8538(port_no, &phy_id);
        phy_receiver_init(port_no);
        phy_write(port_no, 29, PHY_LED_MODE);
#if PERFECT_REACH_LNK_UP
        phy_page_ext(port_no);
        phy_write(port_no, 0x11, 0x80);
        phy_page_std(port_no);
#endif
        /* set LED to blink with 5 Hz */
        phy_write_masked(port_no, 30, 0x0400, 0x0400);
        phy_write_masked(port_no, 30, 0x0000, 0x000f);
        /* disable fiber/copper LED combine */
        phy_write_masked(port_no, 30, 0x8000, 0x8000);
        break;
#endif // VTSS_SPYDER

#if VTSS_QUATTRO
    case VTSS_PHY_FAMILY_QUATTRO: {
        phy_write(port_no, 23, VTSS_REG_23);

        assert_reset(port_no);
        init_seq_8224(port_no, &phy_id);
        phy_receiver_init(port_no);
        break;
    }
#endif // VTSS_QUATTRO

#if VTSS_ENZO
    case VTSS_PHY_FAMILY_ENZO: {
        if (phy_id.model == PHY_MODEL_VTSS_8664) {
            phy_write_masked(port_no, 23, 0x1000, 0x1000);
            assert_reset(port_no);
            init_seq_8634_8664(port_no, &phy_id);
        } else {
            print_str("Unknown model ");
            print_dec(phy_id.model);
            print_cr_lf();
        }
        break;
    }
#endif // VTSS_ENZO

    }
}

/**
 * Do the necessary setup configuration of a PHY.
 */
void phy_setup (vtss_port_no_t port_no)
{
    phy_id_t                phy_id;
#if defined(LUTON26_L25) || VTSS_ELISE || VTSS_TESLA || VTSS_COBRA
    vtss_phy_reset_conf_t   phy_conf;
#endif

    /* Read PHY id to determine action */
    phy_read_id(port_no, &phy_id);

      switch (phy_id.family) {
#if VTSS_COBRA
        case VTSS_PHY_FAMILY_COBRA: {
#if defined(DEBUG)
        print_str("Cobra found at port = ");
        print_dec(port_no);
        print_cr_lf();
#endif
        phy_conf.mac_if = VTSS_PORT_INTERFACE_SGMII;
        phy_conf.media_if = VTSS_PHY_MEDIA_IF_CU;
        cobra_mac_media_if_setup(port_no, &phy_conf);
        break;
    }
#endif // VTSS_COBRA

#if VTSS_ATOM12
    case VTSS_PHY_FAMILY_LUTON26: {
        break;
    }

    case VTSS_PHY_FAMILY_ATOM: {
#if defined(LUTON26_L25)
        if ((port_no > 11) && (port_no < 20)) {
            phy_conf.cu_preferred   = FALSE;
            phy_conf.mac_if         = VTSS_PORT_INTERFACE_QSGMII;
            phy_conf.media_if       = VTSS_PHY_MEDIA_IF_CU;
            atom12_mac_media_if_setup(port_no, &phy_conf);
        } else if (port_no >= 20) {
             phy_conf.cu_preferred  = FALSE;
             phy_conf.mac_if        = VTSS_PORT_INTERFACE_QSGMII;
             phy_conf.media_if      = VTSS_PHY_MEDIA_IF_AMS_CU_1000BX;
             atom12_mac_media_if_setup(port_no, &phy_conf);
        }
#endif // LUTON26_L25
        break;
    }
#endif // VTSS_ATOM12

#if VTSS_TESLA
    case VTSS_PHY_FAMILY_TESLA: {
        phy_conf.cu_preferred = FALSE;

#ifdef LUTON26_L16_QSGMII_EXT_PHY
        phy_conf.mac_if       = VTSS_PORT_INTERFACE_QSGMII;
        println_str("mac_if:VTSS_PORT_INTERFACE_QSGMII");
#else
        phy_conf.mac_if       = VTSS_PORT_INTERFACE_SGMII;
        println_str("mac_if:VTSS_PORT_INTERFACE_SGMII");
#endif
        phy_conf.media_if     = VTSS_PHY_MEDIA_IF_CU;
        tesla_mac_media_if_setup(port_no, &phy_conf);
        break;
    }
#endif // VTSS_TESLA

#if VTSS_ELISE
    case VTSS_PHY_FAMILY_ELISE: {
#if defined(DEBUG)
        print_str("Elise found at port = ");
        print_dec(port_no);
        print_cr_lf();
#endif
        phy_conf.cu_preferred = FALSE;
        phy_conf.mac_if       = VTSS_PORT_INTERFACE_QSGMII;
        phy_conf.media_if     = VTSS_PHY_MEDIA_IF_CU;
        elise_mac_media_if_setup(port_no, &phy_conf);
        break;
    }
#endif // VTSS_ELISE

    default:
        break;
    }
}

#if TRANSIT_EEE
void vtss_phy_eee_ena_private(const vtss_port_no_t port_no,
                              const BOOL           enable)
{
    phy_id_t    phy_id;
    u16         ctrl;

    /* Read PHY id to determine action */
    phy_read_id(port_no, &phy_id);
    
    //print_str("port_no: "); print_dec(port_no);
    //print_str("phy_id: ");
    //print_hex_w(phy_id.family); 
	//print_cr_lf();

    switch (phy_id.family) {
#if VTSS_COBRA
    case VTSS_PHY_FAMILY_COBRA:
        break;
#endif
    case VTSS_PHY_FAMILY_ATOM:
    case VTSS_PHY_FAMILY_LUTON26:
#if VTSS_ATOM12_B
        if (enable) {
            atom_patch_suspend(port_no, FALSE); // Make sure that 8051 patch is running
        } else {
            atom_patch_suspend(port_no, TRUE);  // Suspend 8051 Patch.
        }
        // Pass through
#endif
    case VTSS_PHY_FAMILY_TESLA:
    case VTSS_PHY_FAMILY_VIPER:
    case VTSS_PHY_FAMILY_ELISE:
    case VTSS_PHY_FAMILY_FERRET:
        	
        // Enable/Disable all EEE
        //print_str("enable: "); print_dec(enable); 
        //print_str("port: "); 
        //print_dec(port_no);
       //print_cr_lf();   
        
        if (enable) {
            phy_write(port_no, 31, 0x2); // Change page
            phy_write_masked(port_no, 0x11, 0x8000, 0x8000);     // Enable EEE (EEE control, Adress 17E2, bit 15)
            vtss_phy_mmd_reg_wr_masked(port_no, 7, 60, 6, 0x0006); // Enable advertisement

            phy_write(port_no, 31, 0x2a30);                      // Switch to test-register page
            phy_write_masked(port_no, 25, 0, 1);                 // Clear EEE bit (bit 0)

     //       phy_write(port_no, 31, 0x0);                         // Switch to std page
       //     phy_write_masked(port_no, 0, 0x0200, 0x0200);        // Restart auto negotiation
        } else {
            phy_write(port_no, 31, 0x2); // Change page
            phy_write_masked(port_no, 0x11, 0x0, 0x8000);        // Disable EEE (EEE control, Adress 17E2, bit 15)
            vtss_phy_mmd_reg_wr_masked(port_no, 7, 60, 0, 0x0006); // Disable advertisement
        }

        phy_write(port_no, 31, 0x0);  // Go back to standard page.

        // Only re-start auto-neg if in auto neg mode (Primary due to Bugzilla#7343,
        // which is also the reason to setting the AUTO_NED_ENA bit)
        ctrl = phy_read (port_no, 0);
        if (ctrl & 0x1000) {
            /* start autonegotiation by writing to register 0 */
            phy_write_masked(port_no, 0, 0x1200, 0x1200);
//            VTSS_RC(vtss_phy_wr_masked (port_no,  0, 0x1100, 0x1100));
        }
        break;
    default:
        //Error
        break;
    }
}

/**
 * Retrieve link partners EEE advertisement from register.
 *
 * @param port_no           The PHY port number starting from 0.
 * @param advertisements    Advertisement bit mask.
 *                          Bit 0 = Link partner advertises 100BASE-T capability.
 *                          Bit 1 = Link partner advertises 1000BASE-T capability.
 */
vtss_rc vtss_phy_eee_link_partner_advertisements_get(
    const vtss_cport_no_t chip_port,
    char                  *advertisements)
{
    vtss_rc     rc = VTSS_RC_OK;
    uint        reg_val;

    // Get the link partner advertisement.
    reg_val         = phy_mmd_rd(chip_port, 7, 61);

    // Bit 0 is reserved. See data sheet.
    *advertisements = reg_val >> 1;

    return VTSS_RC_OK;
}
#endif // TRANSIT_EEE

#if TRANSIT_FAN_CONTROL || TRANSIT_THERMAL
/**
 * Set the temperature monitor mode
 */
void phy_init_temp_mode_regs(vtss_port_no_t port_no)
{
    phy_page_gp(port_no);
    phy_write_masked(port_no,0x1A, 0x0080, 0x0080); // Enable TMON1, Deassert Reset and enable background moni
    phy_write_masked(port_no,0x1A, 0x00C0, 0x00C0); // Enable TMON1, Deassert Reset and enable background moni
    phy_page_std(port_no);
}

/**
 * Read the temperature in degree C.
 */
ushort phy_read_temp_reg (vtss_port_no_t port_no)
{
    phy_id_t    phy_id;
    ushort      reg;
    vtss_rc     rc;

    phy_read_id(port_no, &phy_id);

    switch (phy_id.family) {
#if VTSS_COBRA
    case VTSS_PHY_FAMILY_COBRA:
        break;
#endif
#if VTSS_ATOM12
        case VTSS_PHY_FAMILY_ATOM:
        case VTSS_PHY_FAMILY_LUTON26:
            rc = atom12_read_temp_reg(port_no, &reg);
            break;
#endif
#if VTSS_TESLA
        case VTSS_PHY_FAMILY_TESLA:
            rc = tesla_read_temp_reg(port_no, &reg);
            break;
#endif
#if VTSS_ELISE
        case VTSS_PHY_FAMILY_ELISE:
            rc = elise_read_temp_reg(port_no, &reg);
            break;
#endif
        default:
            rc = VTSS_RC_ERROR;
            break;
    }

    if (rc != VTSS_RC_OK)
        return 0xFF;

    //135.3 degC - 0.71 degC * ADCOUT
    reg = (13530 - 71 * reg) / 100;

    return reg;
}
#endif // TRANSIT_FAN_CONTROL || TRANSIT_THERMAL

/*************** need to do some research and clean up *****************/

#if 0   // Uncalled function
/* ************************************************************************ */
uchar init_atom_phase_loop_locked (port_no)
/* ------------------------------------------------------------------------ --
 * Purpose     : 8512/8522 PHY PLL locked
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    ushort reg0, patch_en;
    uchar stat_vec[8];
    uchar  idx;

    // ActivateRegSet 0 4 (micro page)
    phy_write (port_no, 31, 0x10);
    // hold 8051 in SW reset; enable address auto-incr. & PRAM clock
    // MicroMiiWriteBits 0 0 15 12 0x7
    reg0 = phy_read (port_no, 0) & 0x1ff;
    phy_write (port_no, 0, 0x7000 | reg0);
    // configure internal mem. bus for PRAM writes & set 1st byte
    // bus select: 2 => LCPLL/RCOMP
    // MicroMiiWrite 0 12 0x5002
    patch_en = phy_read (port_no, 12) & 0xf00;
    phy_write (port_no, 12, 0x5002 | patch_en); // preserve patch enable bits
    // starting address of MCB data structure
    // MicroMiiWrite 0 11 0x47c8
    phy_write (port_no, 11, 0x47c8);
    // zero out these fields (will be filled in by micro command)
    // num_slaves, cfg_size, stat_size, rw
    for (idx=1; idx<6; idx++) {
        // MicroMiiWrite 0 12 0x5000
        phy_write (port_no, 12, 0x5000 | patch_en);
    }
    // MicroMiiWrite 0 12 0x5001
    phy_write (port_no, 12, 0x5001 | patch_en); // set address vector
    // zero out these fields (will be filled in by micro command)
    // addr_vec buff, cfg_vec, stat_vec
    for (idx=7; idx<56; idx++) {
        // MicroMiiWrite 0 12 0x5000
        phy_write (port_no, 12, 0x5000 | patch_en);
    }

    // disable internal mem. bus writes
    // MicroMiiWriteBits 0 12 15 12 0x0
    phy_write (port_no, 12, patch_en);

    // disable address auto-incr. & restore PRAM clock gating
    // MicroMiiWriteBits 0 0 13 12 0x0
    phy_write (port_no, 0, 0x4000 | reg0);
    // release 8051 from SW reset
    // MicroMiiWriteBits 0 0 15 15 0x1
    phy_write (port_no, 0, 0xc000 | reg0);

    // issue micro command to perform MCB read to user specified bus
    // MicroMiiWrite 0 18 0x8080
    phy_write (port_no, 18, 0x8080);

    // wait for micro to complete command
    while (phy_read (port_no, 18) & 0x8000);

    // enable address auto-incr. & PRAM clock
    // You don't absolutely need to put the micro in SW reset here (clearing bit 15),
    // but it wouldn't hurt.  However, I didn't in my experiments.
    // Doing so *might* clear up some random flaky behavior in the PRAM read back
    // due to micro interference.
    // MicroMiiWriteBits 0 0 13 12 0x3
    phy_write (port_no, 0, 0xf000 | reg0);

    // configure internal mem. bus for PRAM reads
    // MicroMiiWriteBits 0 12 15 12 0xc
    phy_write (port_no, 12, 0xc000 | patch_en);

    // PRAM starting address for stat_vec
    // you *could* set to 0x47c8 and read out the whole data structure if you *really* wanted to
    // MicroMiiWrite 0 11 0x47f8
    phy_write (port_no, 11, 0x47f8);

    for (idx=0; idx<8; idx++) {
        // MicroMiiRead 0 12
        stat_vec[idx] = phy_read (port_no, 12) & 0xff;
    }
    // You care about:
    // stat_vec[5].5 = pll5g_status0.lock_status (must be 1)
    // stat_vec[4].0 = pll5g_status1.fsm_lock (must be 1)


    // disable internal mem. bus reads
    // MicroMiiWriteBits 0 12 15 12 0x0
    phy_write(port_no, 12, patch_en);
    // disable address auto-incr. & restore PRAM clock gating
    // MicroMiiWriteBits 0 0 13 12 0x0
    phy_write (port_no, 0, 0xc000 | reg0);

    phy_write (port_no, 31, 0x0);

    print_str("pll5g_status0 : ");
    print_hex_prefix();
    print_hex_w(stat_vec[5]);
    print_cr_lf();
    print_str("pll5g_status1 : ");
    print_hex_prefix();
    print_hex_w(stat_vec[4]);
    print_cr_lf();

    return test_bit_8(5, &stat_vec[5]) && test_bit_8(0, &stat_vec[4]);
}

ushort phy_mac_if_recv_err_cntr (vtss_port_no_t port_no) {
    ushort tx_pktcntr, tx_pktcntr_active, tx_crcerrcntr;

    phy_write(port_no, 31, 0x10 );
    phy_write(port_no, 18, 0x8014 | ((ushort) port_no) << 8 ); // Suspend micro polling of PHY to enable token-ring access
    phy_write(port_no, 31, 0x52b5 );
    phy_write(port_no, 16, 0xbf88 );
    tx_pktcntr = phy_read(port_no, 17 );
    tx_pktcntr_active = tx_pktcntr & 1;
    tx_crcerrcntr = phy_read(port_no, 18 ) << 1 | ((tx_pktcntr >> 15) & 1);
    tx_pktcntr = (tx_pktcntr >> 1) & 0x3fff;
    phy_write(port_no, 31, 0x10 );
    phy_write(port_no, 18, 0x8004 | ((ushort) port_no) << 8 ); // Resume micro polling of PHY after  token-ring access
    phy_write(port_no, 31, 0 );

    print_str("tx cntr flag :");
    print_dec(tx_pktcntr_active);
    print_str(", tx cntr :");
    print_dec(tx_pktcntr);
    print_str(", tx err cntr :");
    print_dec(tx_crcerrcntr);
    print_cr_lf();

    return tx_crcerrcntr;
}
#endif  // Uncalled function

#if TRANSIT_EEE
#if VTSS_ATOM12_A
/**
 * Allows VGA and/or ADC to power down for EEE.
 *
 * @param vga_adc_pwr   Configure mode.
 *                      00: power down neither
 *                      01: power down ADCs only
 *                      10: power down VGAs only
 *                      11: power down both
 *
 * @param port_no       The port to config.
 *
 * @note To be removed for Luton26 Rev B.
 *
 * @see vga_adc_debug() in API
 */
void vga_adc_debug(vtss_port_no_t   port_no,
                   phy_atom12_en_t  vga_adc_pwr)
{
    /*
     * turn off micro VGA patch temporarily to allow token-ring access
     */

    phy_page_gp(port_no);           // switch to micro page (glabal to all 12 PHYs)
    phy_write(port_no, 18, 0x9024);

    phy_page_tr(port_no);           // Switch to token-ring register page
    phy_write(port_no, 18, 0x0001);
    phy_write(port_no, 17, 0x40b9 | (vga_adc_pwr << 1));
    phy_write(port_no, 16, 0x8fda); // for 100
    phy_write(port_no, 18, 0x0000);
    phy_write(port_no, 17, 0x0159 | (vga_adc_pwr << 1));
    phy_write(port_no, 16, 0x8fd6); // for 1000

    /*
     * turn micro VGA patch back on to allow correct PHY start-up
     */

    phy_page_gp(port_no);           // switch to micro page (glabal to all 12 PHYs)
    phy_write(port_no, 18, 0x9004);

    phy_page_std(port_no);          // switch to standard page
}
#endif //VTSS_ATOM12_A
#endif // TRANSIT_EEE

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
