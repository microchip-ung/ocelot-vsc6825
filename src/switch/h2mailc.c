//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "vtss_api_base_regs.h"
#include "h2io.h"
#include "spiflash.h"
#include "h2mailc.h"
#if TRANSIT_LACP
#include "vtss_lacp.h"
#endif // TRANSIT_LACP
#include "print.h"


#if TRANSIT_MAILBOX_COMM


void h2mailc_conf_set(h2mailc_conf_t comm_conf)
{
    H2_WRITE(VTSS_DEVCPU_ORG_DEVCPU_ORG_MAILBOX, comm_conf.reg_value);
}

void h2mailc_tsk(void)
{
    static BOOL         init_state = TRUE;  // Initial state
    static mac_addr_t   sys_mac_buf;
    BOOL                is_changed = FALSE;
    h2mailc_conf_t      comm_conf;
    uchar               rc = 0;

    H2_READ(VTSS_DEVCPU_ORG_DEVCPU_ORG_MAILBOX, comm_conf.reg_value);

    if (init_state) {
        init_state = FALSE;
        is_changed = TRUE;
        comm_conf.reg_value = 0;
        comm_conf.reg_bit.conf_oper = H2MAILC_OPER_READY;

    } else { // Normal state (none initial state)
        if (comm_conf.reg_bit.conf_oper == H2MAILC_OPER_REQUIRE) {
            is_changed = TRUE;
            comm_conf.reg_bit.conf_oper = (comm_conf.reg_bit.conf_oper < H2MAILC_TYPE_CNT) ?
                                           H2MAILC_OPER_IN_PROG :   /* Goto to IN_PROGRESS state and do the process later */
                                           H2MAILC_OPER_READY;      /* Back to READ state when unknown configured type */
        } else if (comm_conf.reg_bit.conf_oper == H2MAILC_OPER_IN_PROG) {
#if defined(H2_MAILC_DEBUG_ENABLE)
            print_str("h2mailc_tsk(), comm_conf.reg_bit.conf_type = ");
            print_hex_b(comm_conf.reg_bit.conf_type);
            print_cr_lf();
#endif /* H2_MAILC_DEBUG_ENABLE */
            switch (comm_conf.reg_bit.conf_type) {
                case H2MAILC_TYPE_SYS_MAC_SET_LOW:
                    sys_mac_buf[5] = comm_conf.reg_bit.conf_data.sys_mac_l.mac0;
                    sys_mac_buf[4] = comm_conf.reg_bit.conf_data.sys_mac_l.mac1;
                    sys_mac_buf[3] = comm_conf.reg_bit.conf_data.sys_mac_l.mac2;
                    break;
                case H2MAILC_TYPE_SYS_MAC_SET_HIGH:
                    sys_mac_buf[2] = comm_conf.reg_bit.conf_data.sys_mac_h.mac3;
                    sys_mac_buf[1] = comm_conf.reg_bit.conf_data.sys_mac_h.mac4;
                    sys_mac_buf[0] = comm_conf.reg_bit.conf_data.sys_mac_h.mac5;
                    break;
                case H2MAILC_TYPE_SYS_MAC_APPLY:
                    rc = flash_write_mac_addr(&sys_mac_buf);
                    break;
    
#if TRANSIT_LACP
                case H2MAILC_TYPE_LACP_ENABLE:
                case H2MAILC_TYPE_LACP_DISABLE: {
                    vtss_lacp_port_config_t lacp_port_conf;
                    vtss_uport_no_t uport = comm_conf.reg_bit.conf_data.lacp_conf.uport;

                    vtss_lacp_get_portconfig(uport, &lacp_port_conf);
                    if (comm_conf.reg_bit.conf_type == H2MAILC_TYPE_LACP_ENABLE) {
                        lacp_port_conf.enable_lacp = TRUE;
                        lacp_port_conf.port_key = comm_conf.reg_bit.conf_data.lacp_conf.key;
                    } else {
                        lacp_port_conf.enable_lacp = FALSE;
                    }
                    vtss_lacp_set_portconfig(uport, &lacp_port_conf);
                    break;
                }
#endif // TRANSIT_LACP
    
                default:
            println_str("%% unknown configured type for MAILBOX communication");
                    break;
            } // End of switch

            /* Done the required process and ready for the next requirement */
            is_changed = TRUE;
            comm_conf.reg_bit.conf_oper = H2MAILC_OPER_READY;

            /* Fill return code.
             * Currently, there is no return code from the called APIs
             * so we fill 0 in the <conf_data> field
             */
            comm_conf.reg_bit.conf_data.general.data0 = 0;
            comm_conf.reg_bit.conf_data.general.data1 = 0;
            comm_conf.reg_bit.conf_data.general.data2 = rc;
        } // End Normal state
    }

    if (is_changed) {
        h2mailc_conf_set(comm_conf);
    }
}

#endif // TRANSIT_MAILBOX_COMM
