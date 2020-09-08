//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include <string.h>

#include "common.h"     /* Always include common.h at the first place of user-defined herder files */

#include "print.h"
#include "timer.h"

#include "phy_base.h"
#include "misc2.h"
#include "hwport.h"
#include "h2reg.h"
#include "h2io.h"

#include "veriphy.h"
#include "phymap.h"

#if TRANSIT_VERIPHY

/*****************************************************************************
 *
 *
 * Defines
 *
 *
 *
 ****************************************************************************/


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
void veriphy_start (vtss_port_no_t port_no)
/* ------------------------------------------------------------------------ --
 * Purpose     :
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    phy_id_t phy_id;
	/* Read PHY id to determine action */
	phy_read_id(port_no, &phy_id);
	
	switch (phy_id.family) {
#if VTSS_COBRA
	case VTSS_PHY_FAMILY_COBRA:
		break;
#endif

    default:
#if !defined(VTSS_ARCH_OCELOT)
        //signal to API the veriphy is running for this port
        vtss_phy_veriphy_running(port_no, TRUE, TRUE);

        // Micro patch must be suspended while veriphy is running.
        atom_patch_suspend(port_no, TRUE);

        // Starting Veriphy
        phy_page_ext(port_no);
        phy_write_masked(port_no, VTSS_PHY_VERIPHY_CTRL_REG1, 0x8000, 0x8000);
        phy_page_std(port_no);
#endif        
        break;
	}

}

/* ************************************************************************ */
static void veriphy_done (vtss_port_no_t port_no)
/* ------------------------------------------------------------------------ --
 * Purpose     :
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{

    // Ok veriphy done we can resume the micro patchning.
    atom_patch_suspend(port_no, FALSE);
}


ulong vtss_phy_veriphy_running(vtss_port_no_t port_no, BOOL set, uchar running) {
    static ulong veriphy_running_bit_vector = 0; // Bit vector that indicates if veriphy is running for a port. The bit in the vector correspond to the port number

    if (set) {
        if (running) {
            veriphy_running_bit_vector |= 1 << port_no; // Set the running bit in the vector
        } else {
            veriphy_running_bit_vector &= ~(1 << port_no); // Clear the running bit in the vector
        }
    }
    return veriphy_running_bit_vector;
}


/* ************************************************************************ */
uchar veriphy_run (vtss_port_no_t port_no, veriphy_parms_t *veriphy_parms_ptr, BOOL *done)
/* ------------------------------------------------------------------------ --
 * Purpose     :
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    ushort reg_val;
    uchar  result = 1;
    uchar  cnt = 0;


    // Check if veriphy is done
    phy_page_ext(port_no);
    reg_val = phy_read(port_no, VTSS_PHY_VERIPHY_CTRL_REG1);
    if ((reg_val & 0x8000) != 0) {
        *done = FALSE;
        goto done;
    } else {
        *done = TRUE;
    }

    reg_val = phy_read(port_no, VTSS_PHY_VERIPHY_CTRL_REG1);

    // Multiply by 3 because resolution is 3 meters // See datasheet
    veriphy_parms_ptr->loc[0] = ((reg_val & VTSS_PHY_VERIPHY_CTRL_REG1_M_PAIR_A_DISTANCE) >> 8) * 3;
    veriphy_parms_ptr->loc[1] = (reg_val & VTSS_PHY_VERIPHY_CTRL_REG1_M_PAIR_B_DISTANCE) * 3;

    reg_val= phy_read(port_no, VTSS_PHY_VERIPHY_CTRL_REG2);
    veriphy_parms_ptr->loc[2] = ((reg_val & VTSS_PHY_VERIPHY_CTRL_REG2_M_PAIR_C_DISTANCE) >> 8) * 3;
    veriphy_parms_ptr->loc[3] = (reg_val & VTSS_PHY_VERIPHY_CTRL_REG2_M_PAIR_D_DISTANCE) * 3;

    reg_val = phy_read(port_no, VTSS_PHY_VERIPHY_CTRL_REG3);
    veriphy_parms_ptr->stat[3] =  reg_val & VTSS_PHY_VERIPHY_CTRL_REG3_M_PAIR_D_TERMINATION_STATUS;
    veriphy_parms_ptr->stat[2] = (reg_val & VTSS_PHY_VERIPHY_CTRL_REG3_M_PAIR_C_TERMINATION_STATUS) >> 4;
    veriphy_parms_ptr->stat[1] = (reg_val & VTSS_PHY_VERIPHY_CTRL_REG3_M_PAIR_B_TERMINATION_STATUS) >> 8;
    veriphy_parms_ptr->stat[0] = (reg_val & VTSS_PHY_VERIPHY_CTRL_REG3_M_PAIR_A_TERMINATION_STATUS) >> 12;

    veriphy_parms_ptr->flags = 1 << 1; // Signal Veriphy result valid

    for (cnt=0; cnt<4; cnt++)
    {
        if (veriphy_parms_ptr->stat[cnt] != 0) //!=0 : meaning link status is OPEN (status_txt_tab[0]==TXT_NO_VERIPHY_OK)
        {
            veriphy_parms_ptr->flags = 0 ; // Signal Veriphy result invalid 
            break;
        }
    }

    if (phy_map(port_no)==0)//Not CuPHY port (not connected to MIIM0 or MIIM1)
    {
        for (cnt=0; cnt<4; cnt++)
        {
            veriphy_parms_ptr->stat[cnt] = 1;//force link status as OPEN
            veriphy_parms_ptr->loc[cnt]=0;//set len 0  
        }
    }

    phy_page_std(port_no);

    //signal to API the veriphy isn't running for this port
    vtss_phy_veriphy_running(port_no, TRUE, FALSE);
done:

    veriphy_done(port_no);

    return result;
}

#endif

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
