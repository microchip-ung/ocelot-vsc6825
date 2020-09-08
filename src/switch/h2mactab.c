//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */

#include "vtss_api_base_regs.h"
#include "h2mactab.h"
#include "h2.h"
#include "h2io.h"
#include "timer.h"
#include "main.h"
#include "misc2.h"  // For TEST_PORT_BIT_MASK()/test_bit_32()

/*****************************************************************************
 *
 *
 * Defines
 *
 *
 *
 ****************************************************************************/


/* MAC table commands */
#define MAC_TAB_IDLE   0
#define MAC_TAB_LEARN  1
#define MAC_TAB_FORGET 2
#define MAC_TAB_AGE    3
#define MAC_TAB_GET_NX 4
#define MAC_TAB_CLEAR  5
#define MAC_TAB_READ   6
#define MAC_TAB_WRITE  7


/* Define period for performing ageing, granularity is 1 second */
#define AGEING_TIMEOUT 300UL


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
#if TRANSIT_UNMANAGED_MAC_OPER_GET || TRANSIT_UNMANAGED_MAC_OPER_SET
static void write_macdata_reg(mac_tab_t xdata *mac_entry_ptr);
static ulong build_machdata(mac_tab_t xdata *mac_entry_ptr);
static ulong build_macldata(mac_tab_t xdata *mac_entry_ptr);
static BOOL is_ipmc_entry(mac_tab_t xdata *mac_entry_ptr);
static BOOL is_ipv6mc_entry(mac_tab_t xdata *mac_entry_ptr);
static ushort h2_mac_type(mac_tab_t xdata *mac_entry_ptr);
#endif // TRANSIT_UNMANAGED_MAC_OPER_GET || TRANSIT_UNMANAGED_MAC_OPER_SET

static void do_mactab_cmd(ulong mac_access_reg_val);

/*****************************************************************************
 *
 *
 * Local data
 *
 *
 *
 ****************************************************************************/
#if TRANSIT_UNMANAGED_MAC_OPER_SET
/* Add/Delete MAC address entry
 * Only support unicast and one port setting in port_mask parameter currently.
 */
void h2_mactab_set(const mac_tab_t xdata *mac_tab_entry_ptr, BOOL xdata is_add)
{
    uchar           pgid = 0;
    ulong           idx;
    ulong           mach, macl, aged = 0;
    port_bit_mask_t mask = 0, ipv6_mask = 0;
    ushort          type;

    if (!is_add) { // Delete operation
        /* Copy MAC and vid to register and execute forget command */
        write_macdata_reg(mac_tab_entry_ptr);
        do_mactab_cmd(VTSS_F_ANA_ANA_TABLES_MACACCESS_ENTRY_TYPE(h2_mac_type(mac_tab_entry_ptr)) |
                      VTSS_F_ANA_ANA_TABLES_MACACCESS_MAC_TABLE_CMD(MAC_CMD_FORGET));
    } else { // Add operation
        type = h2_mac_type(mac_tab_entry_ptr);
        mach = build_machdata(mac_tab_entry_ptr);
        macl = build_macldata(mac_tab_entry_ptr);
        
        if (type == MAC_TYPE_IPV4_MC || type == MAC_TYPE_IPV6_MC) {
            mask = mac_tab_entry_ptr->port_mask;
            if (type == MAC_TYPE_IPV4_MC) {
                /* Encode port mask directly */
                macl = ((macl & 0x00FFFFFF) | ((mask<<24) & 0xFF000000));
                mach = ((mach & 0xFFFF0000) | ((mask>>8) & 0x0000FFFF));
                idx = ((mask>>24) & 0x3); /* Ports 24-25 */
            } else {
                /* Encode port mask directly */
                mach = ((mach & 0xFFFF0000) | (mask & 0x0000FFFF)); /* ports 0-15  */
                idx = ((mask>>16) & 0x3F);                          /* ports 16-21 */
                ipv6_mask = ((mask>>22) & 0x3);                     /* ports 22-24 */
                aged = ((mask>>25) & 1);                            /* port 25     */
            }
        } else {
            type = MAC_TYPE_LOCKED;
            for (idx = 0; idx < NO_OF_CHIP_PORTS; idx++) {
                if (TEST_PORT_BIT_MASK(idx, &mac_tab_entry_ptr->port_mask)) {
                    break;
                }
            }
        }
        
        /* Copy MAC and vid to register and execute learn command */
        H2_WRITE(VTSS_ANA_ANA_TABLES_MACHDATA, mach);
        H2_WRITE(VTSS_ANA_ANA_TABLES_MACLDATA, macl);

#if defined(VTSS_ARCH_LUTON26)
        do_mactab_cmd(VTSS_F_ANA_ANA_TABLES_MACACCESS_IP6_MASK(ipv6_mask)       |
                      (aged ? VTSS_F_ANA_ANA_TABLES_MACACCESS_AGED_FLAG    : 0) |
                      VTSS_F_ANA_ANA_TABLES_MACACCESS_VALID                     |
                      VTSS_F_ANA_ANA_TABLES_MACACCESS_ENTRY_TYPE(type)          |
                      VTSS_F_ANA_ANA_TABLES_MACACCESS_DEST_IDX(idx)             |
                      VTSS_F_ANA_ANA_TABLES_MACACCESS_MAC_TABLE_CMD(MAC_CMD_LEARN));
#elif defined(VTSS_ARCH_OCELOT)
        do_mactab_cmd(VTSS_F_ANA_ANA_TABLES_MACACCESS_AGED_FLAG(aged)           |
                      VTSS_F_ANA_ANA_TABLES_MACACCESS_VALID(1)                  |
                      VTSS_F_ANA_ANA_TABLES_MACACCESS_ENTRY_TYPE(type)          |
                      VTSS_F_ANA_ANA_TABLES_MACACCESS_DEST_IDX(idx)             |
                      VTSS_F_ANA_ANA_TABLES_MACACCESS_MAC_TABLE_CMD(MAC_CMD_LEARN));
#endif
    }
}
#endif // TRANSIT_UNMANAGED_MAC_OPER_SET

#if TRANSIT_UNMANAGED_MAC_OPER_GET
ulong h2_mactab_result(mac_tab_t xdata *mac_tab_entry_ptr, uchar xdata *ipmc_entry)
{
    ulong value;
    ulong type, idx, aged;
    ulong mach, macl;

    H2_READ(VTSS_ANA_ANA_TABLES_MACACCESS, value);

    /* Check if entry is valid */
#if defined(VTSS_ARCH_LUTON26)
    if (!(value & VTSS_F_ANA_ANA_TABLES_MACACCESS_VALID)) {
        return 0XFFFFFFFF;
    }
#elif defined(VTSS_ARCH_OCELOT)
    if (!(value & VTSS_M_ANA_ANA_TABLES_MACACCESS_VALID)) {
        return 0XFFFFFFFF;
    }
#endif

    type = VTSS_X_ANA_ANA_TABLES_MACACCESS_ENTRY_TYPE(value);
    idx  = VTSS_X_ANA_ANA_TABLES_MACACCESS_DEST_IDX(value);
#if defined(VTSS_ARCH_LUTON26)
    aged = VTSS_BOOL(value & VTSS_F_ANA_ANA_TABLES_MACACCESS_AGED_FLAG);
#elif defined(VTSS_ARCH_OCELOT)
    aged = VTSS_BOOL(value & VTSS_M_ANA_ANA_TABLES_MACACCESS_AGED_FLAG);
#endif

    H2_READ(VTSS_ANA_ANA_TABLES_MACHDATA, mach); // get the next MAC address
    H2_READ(VTSS_ANA_ANA_TABLES_MACLDATA, macl);

    if (type == MAC_TYPE_IPV4_MC || type == MAC_TYPE_IPV6_MC) {
        /* IPv4/IPv6 multicast address */
        *ipmc_entry = TRUE;

        /* Read encoded port mask and update address registers */
        if (type == MAC_TYPE_IPV6_MC) {
            /* IPv6 entry  */
            /* Portmask:  25               24-22                21-16         15-0    */
            mac_tab_entry_ptr->port_mask = (aged<<25  | (((value>>16) & 0x7)<<22) | (idx<<16) | (mach & 0xffff));
            mach = ((mach & 0xffff0000) | 0x00003333);
        } else {
            /* IPv4 entry */
            /* Portmask:25-24        23-0  */
            mac_tab_entry_ptr->port_mask = ((idx<<24) | ((mach<<8) & 0xFFFF00) | ((macl>>24) & 0xff));
            mach = ((mach & 0xffff0000) | 0x00000100);
            macl = ((macl & 0x00ffffff) | 0x5e000000);
        }
    } else {
        *ipmc_entry = FALSE;
#if defined(VTSS_ARCH_LUTON26)
        H2_READ(VTSS_ANA_ANA_TABLES_PGID(idx), mac_tab_entry_ptr->port_mask);
#elif defined(VTSS_ARCH_OCELOT)
        H2_READ(VTSS_ANA_PGID_PGID(idx), mac_tab_entry_ptr->port_mask);
#endif
    }

    mac_tab_entry_ptr->vid = ((mach>>16) & 0xFFF);
    mac_tab_entry_ptr->mac_addr[0] = ((mach>>8)  & 0xff);
    mac_tab_entry_ptr->mac_addr[1] = ((mach>>0)  & 0xff);
    mac_tab_entry_ptr->mac_addr[2] = ((macl>>24) & 0xff);
    mac_tab_entry_ptr->mac_addr[3] = ((macl>>16) & 0xff);
    mac_tab_entry_ptr->mac_addr[4] = ((macl>>8)  & 0xff);
    mac_tab_entry_ptr->mac_addr[5] = ((macl>>0)  & 0xff);

#if 0
    print_str("Found MAC ");
    print_mac_addr(mac_tab_entry_ptr->mac_addr);
    print_str(": ");
    print_port_list(mac_tab_entry_ptr->port_mask);
    print_cr_lf();
#endif

    return value;
}

/* GET/GETNEXT MAC address entry
 * Return 0xFFFFFFFF when entry is invalid. Otherwize, the operation is success.
 */
ulong h2_mactab_get_next(mac_tab_t xdata *mac_tab_entry_ptr, BOOL xdata *ipmc_entry, BOOL xdata is_getnext)
{
    ushort entry_type;
    ulong rc;

    // EA = 0; // Disable interrupt while doing the indirect register access.
    write_macdata_reg(mac_tab_entry_ptr);

    if (is_getnext) {
        entry_type = h2_mac_type(mac_tab_entry_ptr);

        /* Do a get next lookup */
        do_mactab_cmd(VTSS_F_ANA_ANA_TABLES_MACACCESS_MAC_TABLE_CMD(MAC_CMD_GET_NEXT));
    } else {
#if defined(VTSS_ARCH_LUTON26)
        do_mactab_cmd(VTSS_F_ANA_ANA_TABLES_MACACCESS_VALID |
                      VTSS_F_ANA_ANA_TABLES_MACACCESS_ENTRY_TYPE(entry_type) |
                      VTSS_F_ANA_ANA_TABLES_MACACCESS_MAC_TABLE_CMD(MAC_CMD_READ));
#elif defined(VTSS_ARCH_OCELOT)
        do_mactab_cmd(VTSS_F_ANA_ANA_TABLES_MACACCESS_VALID(1) |
                      VTSS_F_ANA_ANA_TABLES_MACACCESS_ENTRY_TYPE(entry_type) |
                      VTSS_F_ANA_ANA_TABLES_MACACCESS_MAC_TABLE_CMD(MAC_CMD_READ));
#endif
    }

    rc = h2_mactab_result(mac_tab_entry_ptr, ipmc_entry);

    // EA = 1; // Enable interrupt
    return rc;
}
#endif // TRANSIT_UNMANAGED_MAC_OPER_GET

/* ************************************************************************ */
void h2_mactab_agetime_set(void)
/* ------------------------------------------------------------------------ --
 * Purpose     : Normal auto aging in seconds
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    /* ANA:ANA:AUTOAGE:AGE_PERIOD
     * An inactive unlocked MAC table entry is aged after 2*AGE_PERIOD
     */
#if defined(VTSS_ARCH_OCELOT)
    H2_WRITE_MASKED(VTSS_ANA_ANA_AUTOAGE,
                    VTSS_F_ANA_ANA_AUTOAGE_AGE_PERIOD(AGEING_TIMEOUT / 2),
                    VTSS_M_ANA_ANA_AUTOAGE_AGE_PERIOD);
#elif defined(VTSS_ARCH_LUTON26)
    H2_WRITE(VTSS_ANA_ANA_AUTOAGE,
             VTSS_F_ANA_ANA_AUTOAGE_AGE_PERIOD(AGEING_TIMEOUT / 2));
#endif
}

/* ************************************************************************ */
void h2_mactab_age(uchar pgid_age, uchar pgid, uchar vid_age, ushort vid)
/* ------------------------------------------------------------------------ --
 * Purpose     : Perform ageing operation on MAC address table.
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    /* Selective aging */
#if defined(VTSS_ARCH_LUTON26)
    H2_WRITE(VTSS_ANA_ANA_ANAGEFIL,
           (pgid_age ? VTSS_F_ANA_ANA_ANAGEFIL_PID_EN : 0) |
           (vid_age ? VTSS_F_ANA_ANA_ANAGEFIL_VID_EN  : 0) |
           VTSS_F_ANA_ANA_ANAGEFIL_PID_VAL(pgid) |
           VTSS_F_ANA_ANA_ANAGEFIL_VID_VAL(vid));
#elif defined(VTSS_ARCH_OCELOT)
    H2_WRITE_MASKED(VTSS_ANA_ANA_ANAGEFIL,
                    VTSS_F_ANA_ANA_ANAGEFIL_PID_EN(pgid_age) |
                    VTSS_F_ANA_ANA_ANAGEFIL_VID_EN(vid_age) |
                    VTSS_F_ANA_ANA_ANAGEFIL_PID_VAL(pgid) |
                    VTSS_F_ANA_ANA_ANAGEFIL_VID_VAL(vid),
                    VTSS_M_ANA_ANA_ANAGEFIL_PID_EN |
                    VTSS_M_ANA_ANA_ANAGEFIL_VID_EN |
                    VTSS_M_ANA_ANA_ANAGEFIL_PID_VAL |
                    VTSS_M_ANA_ANA_ANAGEFIL_VID_VAL);
#endif

    /* Do the aging */
    do_mactab_cmd(VTSS_F_ANA_ANA_TABLES_MACACCESS_MAC_TABLE_CMD(MAC_CMD_TABLE_AGE));

    /* Clear age filter again to avoid affecting automatic ageing */
    H2_WRITE(VTSS_ANA_ANA_ANAGEFIL, 0);
}

/* ************************************************************************ */
void h2_mactab_clear(void)
/* ------------------------------------------------------------------------ --
 * Purpose     : Perform clear operation on MAC address table.
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    do_mactab_cmd(MAC_CMD_TABLE_CLEAR);
}


void h2_mactab_flush_port(vtss_cport_no_t chip_port)
{
    // Age twice means flush
    h2_mactab_age(1, chip_port, 0, 0);
    h2_mactab_age(1, chip_port, 0, 0);
}

/*****************************************************************************
 *
 *
 * Support functions
 *
 *
 *
 ****************************************************************************/
static void do_mactab_cmd(ulong mac_access_reg_val)
{
    ulong cmd;

    H2_WRITE(VTSS_ANA_ANA_TABLES_MACACCESS, mac_access_reg_val);
    do {
        H2_READ(VTSS_ANA_ANA_TABLES_MACACCESS, cmd);
    } while (VTSS_X_ANA_ANA_TABLES_MACACCESS_MAC_TABLE_CMD(cmd) != MAC_CMD_IDLE);
}

#if TRANSIT_UNMANAGED_MAC_OPER_GET || TRANSIT_UNMANAGED_MAC_OPER_SET
static void write_macdata_reg(mac_tab_t xdata *mac_entry_ptr)
{
    H2_WRITE(VTSS_ANA_ANA_TABLES_MACHDATA, build_machdata(mac_entry_ptr));
    H2_WRITE(VTSS_ANA_ANA_TABLES_MACLDATA, build_macldata(mac_entry_ptr));
}

static ulong build_machdata(mac_tab_t xdata *mac_entry_ptr)
{
    ul_union_t tmp;

    tmp.b[0] = HIGH_BYTE(mac_entry_ptr->vid);
    tmp.b[1] = LOW_BYTE(mac_entry_ptr->vid);
    tmp.b[2] = mac_entry_ptr->mac_addr[0];
    tmp.b[3] = mac_entry_ptr->mac_addr[1];

    return tmp.l;
}

static ulong build_macldata(mac_tab_t xdata *mac_entry_ptr)
{
    ul_union_t tmp;

    tmp.b[0] = mac_entry_ptr->mac_addr[2];
    tmp.b[1] = mac_entry_ptr->mac_addr[3];
    tmp.b[2] = mac_entry_ptr->mac_addr[4];
    tmp.b[3] = mac_entry_ptr->mac_addr[5];

    return tmp.l;
}


static BOOL is_ipmc_entry(mac_tab_t xdata *mac_entry_ptr)
{
    return (mac_entry_ptr->mac_addr[0] == 0x01) &&
           (mac_entry_ptr->mac_addr[1] == 0x00) &&
           (mac_entry_ptr->mac_addr[2] == 0x5e);
}

static BOOL is_ipv6mc_entry(mac_tab_t xdata *mac_entry_ptr)
{
    return (mac_entry_ptr->mac_addr[0] == 0x33) &&
           (mac_entry_ptr->mac_addr[1] == 0x33);
}


static ushort h2_mac_type(mac_tab_t xdata *mac_entry_ptr)
{
    if(is_ipmc_entry(mac_entry_ptr))
        return MAC_TYPE_IPV4_MC;
    if(is_ipv6mc_entry(mac_entry_ptr))
        return MAC_TYPE_IPV6_MC;
    return MAC_TYPE_NORMAL;
}
#endif // TRANSIT_UNMANAGED_MAC_OPER_GET || TRANSIT_UNMANAGED_MAC_OPER_SET
