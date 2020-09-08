//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "print.h"
#include "timer.h"

#include "phy_base.h"
#include "phy_family.h"
#include "phydrv.h"
#include "phymap.h"
#include "veriphy.h"

#define PHY_DEBUG (0)

/************************************************************************/
/* Microchip PHY chips                                                    */
/************************************************************************/


#if VTSS_ATOM12 || VTSS_TESLA

// Function that is called any time the micro reset is being asserted
// This function specifically avoids bugzilla #5731
// IN : port_no - Any port within the chip.
vtss_rc vtss_phy_micro_assert_reset(const vtss_port_no_t port_no)
{
    // Set to micro/GPIO-page
    VTSS_RC(vtss_phy_page_gpio(port_no));

    // force micro into a loop, preventing any SMI accesses
    VTSS_RC(vtss_phy_wr_masked(port_no, 12, 0x0000, 0x0800)); // Disable patch vector 3 (just in case)
    VTSS_RC(vtss_phy_wr(port_no,  9, 0x005b));     // Setup patch vector 3 to trap MicroWake interrupt
    VTSS_RC(vtss_phy_wr(port_no, 10, 0x005b));     // Loop forever on MicroWake interrupts
    VTSS_RC(vtss_phy_wr_masked(port_no, 12, 0x0800, 0x0800)); // Enable patch vector 3
    VTSS_RC(vtss_phy_wr(port_no, 18, 0x800f));     // Trigger MicroWake interrupt to make safe to reset

    // Assert reset after micro is trapped in a loop (averts micro-SMI access deadlock at reset)
    VTSS_RC(vtss_phy_wr_masked(port_no, 0, 0x0000, 0x8000));
    VTSS_RC(vtss_phy_wr(port_no, 18, 0x0000));     // Make sure no MicroWake persists after reset
    VTSS_RC(vtss_phy_wr_masked(port_no, 12, 0x0000, 0x0800)); // Disable patch vector 3

    VTSS_RC(vtss_phy_page_std(port_no));
    return VTSS_RC_OK;
}


// Function for downloading code into the internal 8051 CPU.
// In : port_no - Any port within the chip where to download the 8051 code
//      code_array - Pointer to array containing the 8051 code
//      code_size  - The size of the code to be downloaded.
// Return : VTSS_RC_OK if download was done else error code.
vtss_rc atom_download_8051_code(
    vtss_port_no_t port_no, u8 const *code_array, u16 code_size)
{
    u16 i;

//    VTSS_I("atom_download_8051_code port:%d, code_size:%d ", port_no, code_size);

    // Note that the micro/GPIO-page, Reg31=0x10, is a global page, one per PHY chip
    // thus even though broadcast is turned off, it is still sufficient to do once`

    // Hold the micro in reset during patch download
    VTSS_RC(vtss_phy_micro_assert_reset(port_no));
    VTSS_RC(vtss_phy_wr(port_no, 31, 0x10));
    VTSS_RC(vtss_phy_wr(port_no, 0, 0x7009));      //  Hold 8051 in SW Reset,Enable auto incr address and patch clock,Disable the 8051 clock

    VTSS_RC(vtss_phy_wr(port_no, 12, 0x5002));     // write to addr 4000= 02
    VTSS_RC(vtss_phy_wr(port_no, 11, 0x0000));     // write to address reg.

    for (i = 0; i < code_size; i++)
    {
        VTSS_RC(vtss_phy_wr(port_no, 12, 0x5000 | code_array[i]));
    }

    VTSS_RC(vtss_phy_wr(port_no, 12, 0x0000));     // Clear internal memory access
    return VTSS_RC_OK;
 }


#if VTSS_ATOM12_B || VTSS_ATOM12_C || VTSS_ATOM12_D || VTSS_TESLA
// Function that wait for the micro code to complete
// In : port_no : port number staring from 0.
vtss_rc vtss_phy_wait_for_micro_complete(vtss_port_no_t port_no)
{
    u8  timeout = 500;
    u16 reg18g = 0;

    VTSS_RC(vtss_phy_page_gpio(port_no));

    // Wait for micro to complete MCB command (bit 15)
    start_timer(MSEC_500);
    VTSS_RC(vtss_phy_rd(port_no, VTSS_PHY_MICRO_PAGE, &reg18g));
    do {
        VTSS_RC(vtss_phy_rd(port_no, VTSS_PHY_MICRO_PAGE, &reg18g));
    } while (reg18g & 0x8000 && (timeout--));

    if (!timeout) {
#if PHY_DEBUG
        print_str("MCB timeout, port_no ");
        print_dec(port_no);
        print_cr_lf();
#endif // PHY_DEBUG
        return VTSS_RC_ERROR;
    }

    VTSS_RC(vtss_phy_page_std(port_no));
    return VTSS_RC_OK;
}
#endif /* VTSS_ATOM12_B || VTSS_ATOM12_C || VTSS_ATOM12_D || VTSS_TESLA */

#if VTSS_ATOM12_B || VTSS_ATOM12_C || VTSS_ATOM12_D || VTSS_TESLA
//Function for checking that the 8051 code is loaded correctly.

//In : port_no : port number staring from 0.
//     code_length : The length of the downloaded 8051 code
//     expected_crc: The expected CRC for the downloaded 78051 code
vtss_rc vtss_phy_is_8051_crc_ok_private (vtss_port_no_t port_no, u16 start_addr, u16 code_length, u16 expected_crc)
{
    u16 crc_calculated = 0;

    VTSS_RC(vtss_phy_page_ext(port_no));
    VTSS_RC(vtss_phy_wr(port_no, 25, start_addr));
    VTSS_RC(vtss_phy_wr(port_no, 26, code_length));

    VTSS_RC(vtss_phy_page_gpio(port_no)); // Change to GPIO page
    VTSS_RC(vtss_phy_wr(port_no, VTSS_PHY_MICRO_PAGE, 0x8008)); // Start Mirco command

    //MISSING VTSS_RC() is not an omission, it is intended so we get a CRC error
    (void) vtss_phy_wait_for_micro_complete(port_no);

    // Get the CRC
    VTSS_RC(vtss_phy_page_ext(port_no));
    VTSS_RC(vtss_phy_rd(port_no, 25, &crc_calculated));

    VTSS_RC(vtss_phy_page_std(port_no)); // return to standard page


    if (crc_calculated == expected_crc) {
      // println_str("CRCOK");
//      VTSS_I("8051 crc_calculated = 0x%X, expected_crc = 0x%X", crc_calculated, expected_crc);
        return VTSS_RC_OK;
    } else {
        println_str("CRCERR");

//      VTSS_E("Microcode crc_calculated = 0x%X, expected_crc = 0x%X", crc_calculated, expected_crc);
        return VTSS_RC_ERROR;
    }
}
#endif /* VTSS_ATOM12_B || VTSS_ATOM12_C || VTSS_ATOM12_D || VTSS_TESLA */
#endif /* VTSS_ATOM12 || VTSS_TESLA */


#if VTSS_ATOM12
/**
 * Function for suspending / resuming the 8051 patch.
 *
 * @param   port_no     Any port within the chip where to supend 8051 patch
 * @param   suspend     True if 8051 patch shall be suspended, else patch is resumed.
 */
void atom_patch_suspend(vtss_port_no_t port_no, BOOL suspend)
{
#if TRANSIT_VERIPHY
    // We must disable the EEE patch when VeriPHY is running,  When VeriPHY  is running, the patch  needs to be suspended by
    // writing 0x800f to register 18 on the Micro/GPIO page (0x10).  0x800f is chosen as this is an unimplemented micro-command
    // and issuing any micro command when the patch is running, causes the patch to stop.
    // The micro patch must not be enabled until all pending VeriPHY requests have been completed on all ports in the relevant Luton26 or Atom12 chip.
    // When all have been completed, the micro patch should re-enable.  Note that this is necessary only because the patch for EEE consumes
    //the micro continually to service all 12 PHYs in a timely manner and workaround one of the weaknesses in gigabit EEE in Luton26/Atom12.
    if (vtss_phy_veriphy_running(port_no, FALSE, FALSE) != 0) {
        phy_page_gp(port_no); // Change to GPIO page
        phy_write(port_no, VTSS_PHY_MICRO_PAGE, 0x800F); // Suspend the micro patch
        phy_page_std(port_no); // Back to standard page
        return;
    }
#endif  // TRANSIT_VERIPHY

    phy_page_gp(port_no); // Change to GPIO page

    if (suspend) {
        // Suspending 8051 patch
#if VTSS_ATOM12_A
        // From JimB
        // You are suspending the micro patch momentarily by writing
        // 0x9014 to GPIO-page register 18 and resuming by writing 0x9004 to
        // GPIO-page register 18..
        phy_write(port_no, 18, 0x9014); // Suspend vga patch
#endif
#if VTSS_ATOM12_B
        {
            // See comment below.
            int word = phy_read(port_no, 18);
            if (!(word & 0x4000)) {
                phy_write(port_no, 18, 0x8009); // Suspend 8051 patch
            }
        }
#endif
    } else {
        // Resuming 8051 patch
#if VTSS_ATOM12_A
        phy_write(port_no, 18, 0x9004);
#endif
#if VTSS_ATOM12_B
        {
            // On page 0x10 (Reg31 = 0x10), write register 18 with 0x8009 to turn on the EEE patch.
            // Once this is done all code that might attempt to access page 0x52b5 will fail and likely cause issues.
            // If you need to access page 0x52b5 or run another page 0x10, register 18 function, you must first disable
            // the patch by writing 0x8009 again to register 18.  In response, the error bit (bit 14) will be set,
            // but the micro is now freed from running the EEE patch and you may issue your other micro requests.
            // When the other requests are complete, you will want to rerun the EEE patch by writing 0x8009 to register 18 on page 0x10.
            // The events that are handled by the micro patch occur occasionally, say one event across 12 ports every 30 seconds.
            // As a result, suspending the EEE patch for short durations Is unlikely to result in link drops, but it is possible.
            int word = phy_read(port_no, 18);
            if (word & 0x4000) {
                phy_write(port_no, 18, 0x8009);
            }
        }
#endif
    }

    phy_page_std(port_no); // Change to standard page
}
#endif // VTSS_ATOM12

#if TRANSIT_FAN_CONTROL || TRANSIT_THERMAL
ushort vtss_phy_read_temp(vtss_port_no_t port_no)
{
    ushort reg;

    phy_page_gp(port_no); // Change to GPIO page

    // Workaround because background temperature monitoring doesn't work
    phy_write_masked(port_no,0x1A, 0x0, 0x0040);
    phy_write_masked(port_no,0x1A, 0x40, 0x0040);

    // Read current register value (Temperature Registers 26G,27G and 28G)
    reg = phy_read(port_no, 28);
    reg &= 0xFF; // See data sheet for Registers 26G,27G and 28G
    return reg;
}
#endif // TRANSIT_FAN_CONTROL || TRANSIT_THERMAL

/* ************************************************************************ */
void phy_read_id (vtss_port_no_t port_no, phy_id_t *phy_id_p)
/* ------------------------------------------------------------------------ --
 * Purpose     : Read PHY id from register 2 and 3 and generate software ids.
 * Remarks     : port_no: The port number to which the PHY is attached.
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    ul_union_t phy_id_raw;

    if (!phy_map(port_no)) {
        phy_id_p->vendor = PHY_MODEL_NONE;
        phy_id_p->family = VTSS_PHY_FAMILY_NONE;
        phy_id_p->model  = PHY_MODEL_NONE;
        return;
    }

    phy_id_raw.w[0] = phy_read(port_no, 2);
    phy_id_raw.w[1] = phy_read(port_no, 3);

    /* Generate vendor identification */
    if (((phy_id_raw.l & 0xfffffc00) == PHY_OUI_VTSS_1) ||
            ((phy_id_raw.l & 0xfffffc00) == PHY_OUI_VTSS_2)) {
        phy_id_p->vendor = PHY_VENDOR_VTSS;
    } else {
        phy_id_p->vendor = PHY_VENDOR_UNKNOWN;
    }

    /* Retrieve revision number */
    phy_id_p->revision = phy_id_raw.w[1] & 0x000f;

    /* Generate family and model identifications */
    switch (phy_id_raw.l & 0xfffffff0) { /* mask out revision */
    case PHY_ID_VTSS_7512:
	    phy_id_p->family = VTSS_PHY_FAMILY_FERRET;
		phy_id_p->model  = PHY_MODEL_VTSS_7512;
		break;
#if VTSS_COBRA
    case PHY_ID_VTSS_8211:
        phy_id_p->family = VTSS_PHY_FAMILY_COBRA;
        phy_id_p->model  = PHY_MODEL_VTSS_8211;
        break;
    case PHY_ID_VTSS_8221:
        phy_id_p->family = VTSS_PHY_FAMILY_COBRA;
        phy_id_p->model  = PHY_MODEL_VTSS_8221;
        break;
#endif
#if VTSS_QUATTRO
    case PHY_ID_VTSS_8224:
        phy_id_p->family = VTSS_PHY_FAMILY_QUATTRO;
        phy_id_p->model  = PHY_MODEL_VTSS_8224;
        break;
    case PHY_ID_VTSS_8234:
        phy_id_p->family = VTSS_PHY_FAMILY_QUATTRO;
        phy_id_p->model  = PHY_MODEL_VTSS_8234;
        break;
    case PHY_ID_VTSS_8244:
        phy_id_p->family = VTSS_PHY_FAMILY_QUATTRO;
        phy_id_p->model  = PHY_MODEL_VTSS_8244;
        break;
#endif
#if VTSS_SPYDER
    case PHY_ID_VTSS_8538_A:
    case PHY_ID_VTSS_8538:
        phy_id_p->family = VTSS_PHY_FAMILY_SPYDER;
        phy_id_p->model  = PHY_MODEL_VTSS_8538;
        break;
    case PHY_ID_VTSS_8558_A:
    case PHY_ID_VTSS_8558:
        phy_id_p->family = VTSS_PHY_FAMILY_SPYDER;
        phy_id_p->model  = PHY_MODEL_VTSS_8558;
        break;
    case PHY_ID_VTSS_8658_A:
        phy_id_p->family = VTSS_PHY_FAMILY_SPYDER;
        phy_id_p->model  = PHY_MODEL_VTSS_8658;
        break;
#endif
    case PHY_ID_VTSS_7422:
        phy_id_p->family = VTSS_PHY_FAMILY_LUTON26;
        phy_id_p->model  = PHY_MODEL_VTSS_7422;
        break;
#if VTSS_ATOM12
    case PHY_ID_VTSS_8512:
    case PHY_ID_VTSS_8522:
        phy_id_p->family = VTSS_PHY_FAMILY_ATOM;
        phy_id_p->model  = PHY_MODEL_VTSS_8512;
        break;
#endif
#if VTSS_TESLA
    case PHY_ID_VTSS_8574:
    case PHY_ID_VTSS_8504:
        phy_id_p->family = VTSS_PHY_FAMILY_TESLA;
        phy_id_p->model  = PHY_MODEL_VTSS_8504;
        break;
    case PHY_ID_VTSS_8552:
        phy_id_p->family = VTSS_PHY_FAMILY_TESLA;
        phy_id_p->model  = PHY_MODEL_VTSS_8552;
        break;
#endif
#if VTSS_ELISE
    case PHY_ID_VTSS_8514:
        phy_id_p->family = VTSS_PHY_FAMILY_ELISE;
        phy_id_p->model  = PHY_MODEL_VTSS_8514;
        break;
#endif
#if VTSS_ENZO
    case PHY_ID_VTSS_8664:
        phy_id_p->family = VTSS_PHY_FAMILY_ENZO;
        phy_id_p->model = PHY_MODEL_VTSS_8664;
        break;
#endif
    default:
        phy_id_p->family = VTSS_PHY_FAMILY_NONE;
        phy_id_p->model  = PHY_MODEL_UNKNOWN;
        break;
    }
}

#if 0 // Ferret, TODO. Does it relate to POLARITY_DETECT_FOR_10HDX_MODE?
//#define SERDES_MAC_RX     (0x1 << 6)
//#define SERDES_MAC_TX     (0x1 << 5)
//#define SERDES_MAC_RX_TX     (SERDES_MAC_RX | SERDES_MAC_TX)
void phy_serdes_polarity_invert(
    vtss_port_no_t port_no,
    ushort         mac_direction
)
{
    u16 regtmp;

    //if (conf->mac_if ==  VTSS_PORT_INTERFACE_SGMII) 
    {				
        print_str("set 16E3 for port :");
        print_dec(port_no);
        VTSS_RC(vtss_phy_wr(port_no, 31, 3));//switch to phy page3													
        VTSS_RC(vtss_phy_wr_masked(port_no, 16,  mac_direction, 0x0060)); //set [6..5] 16E3 register: To invert the SerDes's signal's polarity at Tx/Rx

        VTSS_RC(vtss_phy_rd(port_no, 16, &regtmp)); //read
        print_str(":");
        print_hex_w(regtmp);
        print_cr_lf();
				
        VTSS_RC(vtss_phy_wr(port_no, 31, 0));	//switch to STD page
    }

}
#endif // if 0

#if VTSS_ELISE || VTSS_TESLA
vtss_rc vtss_phy_soft_reset_port (
    vtss_port_no_t          port_no
) {
    u16                     reg;
    u16                     loops = 30;

    VTSS_RC(vtss_phy_wr        (port_no, 31, 0 )); // STD page

#if PHY_DEBUG
    println_str("step 10. software reset");
#endif

    VTSS_RC(vtss_phy_wr_masked (port_no, 0x00, 0x8000, 0x8000)); // Reset phy port

#if PHY_DEBUG
    println_str("step 11. wait bit 15 to become 0");
#endif

    while (loops--) {
        delay_1(5);                  /* pause after reset */
        vtss_phy_rd(port_no,  0, &reg);
        if ((reg & 0x8000) == 0) {
            break;
        }

#if PHY_DEBUG
        print_str("reg=");
        print_dec(reg);
        print_cr_lf();
#endif // PHY_DEBUG
    }

    // After reset of a port, we need to re-configure it
#if 0
    VTSS_RC(vtss_phy_conf_set_private(port_no)); // TODO: Leave to phy_post_reset
#endif

    return VTSS_RC_OK;
}
#endif /* VTSS_ELISE || VTSS_TESLA */

#if VTSS_QUATTRO || VTSS_SPYDER || VTSS_COBRA
void vtss_phy_enab_smrt_premphasis(vtss_port_no_t port_no)
{
    phy_page_tr(port_no);
    phy_write(port_no, 16, 0xa7fa);
    phy_write_masked(port_no, 18, 0x0000, 0x0000);
    phy_write_masked(port_no, 17, 0x0008, 0x0008);
    phy_write(port_no, 16, 0x87fa);
    phy_page_std(port_no);
}
#endif /* VTSS_QUATTRO || VTSS_SPYDER */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
