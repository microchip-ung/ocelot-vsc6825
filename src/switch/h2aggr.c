//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT

#include "common.h"
#include "h2.h"
#include "h2reg.h"
#include "h2io.h"
#include "main.h"
#include "h2aggr.h"
//#include "eeprom.h"
#include "phytsk.h"
#include "misc2.h"
#include "hwport.h"
#if TRANSIT_LACP
#include "vtss_lacp.h"
#endif /* TRANSIT_LACP */
#if TRANSIT_RSTP
#include "vtss_rstp.h"
#endif /* TRANSIT_RSTP */

#if TRANSIT_DOT1X
#include "hwport.h"
#include "ieee8021x.h"
#endif

#if TRANSIT_LAG


/* ************************************************************************ **
 *
 *
 * Defines
 *
 *
 *
 * ************************************************************************ */
//VTSS_F_ANA_COMMON_AGGR_CFG_AC_IP4_SIPDIP_ENA | VTSS_F_ANA_COMMON_AGGR_CFG_AC_IP4_TCPUDP_ENA |
//VTSS_F_ANA_COMMON_AGGR_CFG_AC_DMAC_ENA | VTSS_F_ANA_COMMON_AGGR_CFG_AC_SMAC_ENA |
//VTSS_F_ANA_COMMON_AGGR_CFG_AC_IP6_TCPUDP_ENA | VTSS_F_ANA_COMMON_AGGR_CFG_AC_IP6_FLOW_LBL_ENA
#define     DEFAULT_AGGR_MODE   0x7E 



/* ************************************************************************ **
 *
 *
 * Typedefs and enums
 *
 *
 *
 * ************************************************************************ */


/* ************************************************************************ **
 *
 *
 * Prototypes for local functions
 *
 *
 *
 * ************************************************************************ */
static void h2_aggr_mode (uchar mode);

#if TRANSIT_DOT1X
static void update_aggr_mask(uchar port_no);
#endif

/* ************************************************************************ **
 *
 *
 * Local data
 *
 *
 *
 * ************************************************************************ */
static uchar           h2_aggr_mode_conf;
static port_bit_mask_t h2_aggr_group[MAX_AGGR_GROUP];


#if TRANSIT_LACP
/* ************************************************************************ */
static void lacp_control_ports(port_bit_mask_t port_mask, vtss_common_bool_t enable)
/* ------------------------------------------------------------------------ --
 * Purpose     : Enable/Disable LACP on a all the ports in the port_mask
 * Remarks     : The configured LACP enable/disable state also has to match.
 * Restrictions:
 * See also    :
 * Example     :
 * ************************************************************************ */
{
    vtss_lacp_port_config_t cfg;
    uchar portno;

    for (portno = MIN_PORT; portno < MAX_PORT; portno++)
        if (TEST_PORT_BIT_MASK(portno, &port_mask)) {
            vtss_lacp_get_portconfig(OSINT2LACP(portno), &cfg);
            if (cfg.enable_lacp != enable && /* Oper state differs */
                (!enable //||     /* hard defined aggr wins over LACP  */
                 /* eeprom_read_lacp_enable(portno) == enable*/)) { /* but admin state matches */
                cfg.enable_lacp = enable;
                vtss_lacp_set_portconfig(OSINT2LACP(portno), &cfg); /* oper = admin */
            }
        }
}
#endif /* TRANSIT_LACP */

/* ************************************************************************ */
void h2_aggr_init (void)
/* ------------------------------------------------------------------------ --
 * Purpose     : Read configuration from EEPROM and update Heathrow accordingly.
 * Remarks     : 
 * Restrictions:
 * See also    :
 * Example     :
 * ************************************************************************ */
{
    uchar group;

    h2_aggr_mode_conf = DEFAULT_AGGR_MODE;
    h2_aggr_mode(DEFAULT_AGGR_MODE);

    for (group = 0; group < MAX_AGGR_GROUP; group++) {
        h2_aggr_group [group] = 0;           
    }
    
    //vtss_set_aggr_group(1, 0x3);
    //h2_aggr_add(0x3);
}

/* ************************************************************************ */
#if 0 // not used
void h2_aggr_set_mode (uchar mode)
/* ------------------------------------------------------------------------ --
 * Purpose     : Set aggregation mode in EEPROM and in Heathrow.
 * Remarks     : mode is 1 for SMAC, 2 for DMAC, 3 for SMAC XOR DMAC
 * Restrictions:
 * See also    :
 * Example     :
 * ************************************************************************ */
{
    h2_aggr_mode_conf = DEFAULT_AGGR_MODE;
    h2_aggr_mode(mode);
}
#endif // not used

static uchar ports_in_mask(port_bit_mask_t i_port_mask, port_bit_mask_t *c_port_mask)
/* ------------------------------------------------------------------------ --
 * Purpose     : Count the number of ports included in the mask
 *               Map i_port to c_port   
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 * ************************************************************************ */
{
    uchar port_no;
    uchar port_count = 0;

    for (port_no = MIN_PORT; port_no < MAX_PORT; port_no++) {
        if (TEST_PORT_BIT_MASK(port_no, &i_port_mask)) {
            WRITE_PORT_BIT_MASK(iport2cport(port_no), 1, c_port_mask);
            port_count++;
        }
    }
    return port_count;
}

/* ************************************************************************ */
uchar h2_aggr_add (port_bit_mask_t i_port_mask)
/* ------------------------------------------------------------------------ --
 * Purpose     : Define a new aggregation group. Update EEPROM and switch chip
 *               accordingly.
 * Remarks     : i_port_mask specifies port members of the group.
 *               Returns 0, if succeeded, otherwise an error code indicating
 *               either port already in another group or no free groups.
 * Restrictions:
 * See also    :
 * Example     :
 * ************************************************************************ */
{
    uchar           group;
    uchar           port_count;
    port_bit_mask_t c_port_mask = 0;

    
    /* Check if max. number of aggregated ports is exceeded */
    port_count = ports_in_mask(i_port_mask, &c_port_mask);

    if (port_count > MAX_NO_OF_AGGR_PORTS) {
        return ERROR_AGGR_4;
    }

    /* check if a port is already member of another group */
    if ((group = h2_aggr_find_group(i_port_mask)) != 0xff) {
        if (h2_aggr_group[group] == i_port_mask) {
            return 0; /* Group already defined */
        }
        else {
            return ERROR_AGGR_1;
        }
    }

    /* Find free group */
    for (group = 0; group < MAX_AGGR_GROUP; group++) {
        if (h2_aggr_group[group] == 0) {
            h2_aggr_group[group] = i_port_mask ;
            vtss_set_aggr_group(group, c_port_mask); // Important!!! - c_port_mask to use when call to h2.c
#if TRANSIT_LACP
            /* Disable LACP on ports that have a defined aggr */
            lacp_control_ports(i_port_mask, VTSS_COMMON_BOOL_FALSE);
#endif /* TRANSIT_LACP */
            return 0;
        }
    }

    return ERROR_AGGR_2;
}

/* ************************************************************************ */
uchar h2_aggr_delete (port_bit_mask_t port_mask)
/* ------------------------------------------------------------------------ --
 * Purpose     : Delete aggregation group(s) of which the ports specified are
 *               members. Update EEPROM and Heathrow accordingly.
 * Remarks     : port_mask specifies port members of the group(s).
 *               Returns 0, if succeeded, otherwise an error code indicating
 *               that group was not found.
 * Restrictions:
 * See also    :
 * Example     :
 * ************************************************************************ */
{
    uchar group;
    uchar status;

    status = ERROR_AGGR_3;

    /* Find the group(s) */
    while ((group = h2_aggr_find_group(port_mask)) != 0xff) {
        /* Reverse the sequence to display the real time configs in eeprom for chk_staggr_member usage*/ 
        h2_aggr_group[group] = 0;
        vtss_set_aggr_group(group, 0);
   //    eeprom_write_aggr_group(group, 0);
        status = 0;
#if TRANSIT_LACP
        /* Enable LACP on ports that no longer have a defined aggr */
        lacp_control_ports(port_mask, VTSS_COMMON_BOOL_TRUE);
#endif /* TRANSIT_LACP */
    }

    return status;
}

#if TRANSIT_RSTP_TRUNK_COWORK && TRANSIT_RSTP
/**
 * Check if a port belongs to a static trunk AND the state of that trunk
 * -- Return 0 if pno is not in a static trunk
 * -- Return 1 if pno is member of a static trunk AND one port of the 
 * * trunk is in forwarding state
 * -- Return 2 if pno is member of a static trunk AND NO member of the
 * * trunk is in forwarding state
 */
uchar chk_staggr_member (uchar pno)
{
    uchar group_no;
    port_bit_mask_t member_mask = 0x0;
    port_bit_mask_t port_mask = 0x0;
    uchar found;
    uchar port_no;
    
    WRITE_PORT_BIT_MASK(pno, 1, &port_mask);

    found = FALSE;
    for (group_no = 0; group_no < MAX_AGGR_GROUP; group_no++) {
        member_mask = eeprom_read_aggr_group(group_no);
        if ((port_mask & member_mask) != 0) {

            for (port_no = MIN_PORT; port_no < MAX_PORT; port_no++) {
                if (TEST_PORT_BIT_MASK(port_no, &member_mask)) {
                      if(H2_STP_FORWARDING(port_no)) {
                                        found = TRUE;
                                        return 1;
                       }
                }
            }

            found = FALSE;
            return 2;

        }
    }

    if (!found) {
		return 0;
    }

    return 0;
}
#endif

/* ************************************************************************ **
 *
 *
 * Support functions
 *
 *
 *
 * ************************************************************************ */

static void h2_aggr_mode (uchar mode)
{
    H2_WRITE(VTSS_ANA_COMMON_AGGR_CFG, mode);
}

uchar h2_aggr_find_group (port_bit_mask_t port_mask)
{
    uchar group;

    for (group = 0; group < MAX_AGGR_GROUP; group++) {
        if ((h2_aggr_group[group] & port_mask) != 0) {
            return group;
        }
    }
    return 0xff;
}

#if TRANSIT_LACP
#if TRANSIT_RSTP
static vtss_lacp_agid_t rstpmap[VTSS_RSTP_MAX_APORTS];

static uchar find_rstp_aid(vtss_lacp_agid_t aid)
{
    uchar pno;

    for (pno = 0; pno < VTSS_RSTP_MAX_APORTS; pno++)
        if (rstpmap[pno] == aid)
            return pno;
    return 0xFF;
}

static void rstp_aggr_add(uchar raid, port_bit_mask_t bm)
{
    uchar pno;
    vtss_common_bool_t members[NO_OF_PORTS + 1];

    for (pno = 1; pno <= NO_OF_PORTS; pno++)
        members[pno] = TEST_PORT_BIT_MASK(port2int(pno), &bm);
    vtss_rstp_aggr_add(raid + VTSS_RSTP_MAX_PORTS + 1, members);
}
#endif /* TRANSIT_RSTP */

/**
 * vtss_os_set_hwaggr - Add a specified port to an existing set of ports for aggregation.
 * @aid: The set the port should be added to. (1 - VTSS_LACP_MAX_AGGR)
 * @new_port: Port number (1 - VTSS_LACP_MAX_PORTS)
 *
 */
void vtss_os_set_hwaggr(vtss_lacp_agid_t aid,
                        vtss_common_port_t new_port)
{
    port_bit_mask_t members;
#if TRANSIT_RSTP    
    port_bit_mask_t c_port_mask;
#endif /* TRANSIT_RSTP */    
    uchar pno = LACP2OSINT(new_port);

    members = vtss_get_aggr_group(MAX_AGGR_GROUP - 1 + aid);
    if (TEST_PORT_BIT_MASK(pno, &members))
        return;
    WRITE_PORT_BIT_MASK(pno, 1, &members);
    vtss_set_aggr_group(MAX_AGGR_GROUP - 1 + aid, members);
#if TRANSIT_RSTP
    if (ports_in_mask(members, &c_port_mask) > 1) {
        pno = find_rstp_aid(aid);
        if (pno == 0xFF) {
            pno = find_rstp_aid(0);
            VTSS_COMMON_ASSERT(pno != 0xFF);
            rstpmap[pno] = aid;
        }
        rstp_aggr_add(pno, c_port_mask);
    }
#endif /* TRANSIT_RSTP */
}

/**
 * vtss_os_clear_hwaggr - Remove a specified port from an existing set of ports for aggregation.
 * @aid: The set the port should be added to. (1 - VTSS_LACP_MAX_AGGR)
 * @old_port: Port number (1 - VTSS_LACP_MAX_PORTS)
 *
 */
void vtss_os_clear_hwaggr(vtss_lacp_agid_t aid,
                          vtss_common_port_t old_port)
{
    port_bit_mask_t members;
    uchar pno = LACP2OSINT(old_port);
#if TRANSIT_RSTP
    port_bit_mask_t c_port_mask;
    uchar pcnt;
#endif /* TRANSIT_RSTP */

    members = vtss_get_aggr_group(MAX_AGGR_GROUP - 1 + aid);
    if (TEST_PORT_BIT_MASK(pno, &members)) {
        WRITE_PORT_BIT_MASK(pno, 0, &members);
        vtss_set_aggr_group(MAX_AGGR_GROUP - 1 + aid, members);
#if TRANSIT_RSTP
        pcnt = ports_in_mask(members, &c_port_mask);
        pno = find_rstp_aid(aid);
        if (pno != 0xFF) {
            if (pcnt <= 1)
                vtss_rstp_aggr_del(pno + VTSS_RSTP_MAX_PORTS + 1);
            else
                rstp_aggr_add(pno, c_port_mask);
        }
#endif /* TRANSIT_RSTP */
    }
}
#endif /* TRANSIT_LACP */

#endif // TRANSIT_LAG