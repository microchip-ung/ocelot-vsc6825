//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __PHYDRV_H__
#define __PHYDRV_H__

#include "common.h"     /* Always include common.h at the first place of user-defined herder files */

/****************************************************************************
 *
 *
 * Defines
 *
 *
 ****************************************************************************/

// *********** Registers *************


// Function for setting a bit in a vector.
#define VTSS_PHY_BIT(bit)       (1 << (bit))

// Set x bits to 1. E.g x=3 -> b111 (0x7)
#define VTSS_PHY_BITMASK(x)               ((1 << (x)) - 1)

#define VTSS_PHY_EXTRACT_BITFIELD(value, offset, width)  (((value) >> (offset)) & VTSS_PHY_BITMASK(width))
#define VTSS_PHY_ENCODE_BITFIELD(value, offset, width)   (((value) & VTSS_PHY_BITMASK(width)) << (offset))
#define VTSS_PHY_ENCODE_BITMASK(offset, width)      (VTSS_PHY_BITMASK(width) << (offset))


// ** Register definitions - Standard page **

// Register 0 + Bit fields
#define VTSS_PHY_MODE_CONTROL 0
#define  VTSS_F_PHY_MODE_CONTROL_LOOP  VTSS_PHY_BIT(14)



// Register 23 + Bit fields
#define VTSS_PHY_EXTENDED_PHY_CONTROL 23
#define  VTSS_F_PHY_EXTENDED_PHY_CONTROL_FAR_END_LOOPBACK_MODE  VTSS_PHY_BIT(3)
#define  VTSS_F_PHY_EXTENDED_PHY_CONTROL_AMS_PREFERENCE         VTSS_PHY_BIT(11)



// Register 25 + Bit fields
#define VTSS_PHY_INTERRUPT_MASK      25

#define VTSS_PHY_INTERRUPT_MASK_INT_MASK                     VTSS_PHY_BIT(15)
#define VTSS_PHY_INTERRUPT_MASK_SPEED_STATE_CHANGE_MASK      VTSS_PHY_BIT(14)
#define VTSS_PHY_INTERRUPT_MASK_LINK_MASK                    VTSS_PHY_BIT(13)
#define VTSS_PHY_INTERRUPT_MASK_FDX_STATE_CHANGE_MASK        VTSS_PHY_BIT(12)
#define VTSS_PHY_INTERRUPT_MASK_AUTO_NEG_ERROR_MASK          VTSS_PHY_BIT(11)
#define VTSS_PHY_INTERRUPT_MASK_AUTO_NEG_COMPLETE_MASK       VTSS_PHY_BIT(10)
#define VTSS_PHY_INTERRUPT_MASK_INLINE_POW_DEV_DETECT_MASK   VTSS_PHY_BIT(9)
#define VTSS_PHY_INTERRUPT_MASK_SYMBOL_ERR_INT_MASK          VTSS_PHY_BIT(8)
#define VTSS_PHY_INTERRUPT_MASK_FAST_LINK_MASK               VTSS_PHY_BIT(7)
#define VTSS_PHY_INTERRUPT_MASK_TX_FIFO_OVERFLOW_INT_MASK    VTSS_PHY_BIT(6)
#define VTSS_PHY_INTERRUPT_MASK_RX_FIFO_OVERFLOW_INT_MASK    VTSS_PHY_BIT(5)
#define VTSS_PHY_INTERRUPT_MASK_AMS_MEDIA_CHANGE_MASK        VTSS_PHY_BIT(4)
#define VTSS_PHY_INTERRUPT_MASK_FALSE_CARRIER_INT_MASK       VTSS_PHY_BIT(3)
#define VTSS_PHY_INTERRUPT_MASK_LINK_SPEED_DS_DETECT_MASK    VTSS_PHY_BIT(2)
#define VTSS_PHY_INTERRUPT_MASK_MASTER_SLAVE_RES_ERR_MASK    VTSS_PHY_BIT(1)
#define VTSS_PHY_INTERRUPT_MASK_RX_ER_INT_MASK               VTSS_PHY_BIT(0)

// Register 26 + Bit fields
#define VTSS_PHY_INTERRUPT_STATUS    26


// ** Register definitions -  EXT1 page**
// Register 24E1 + Bit fields
#define VTSS_PHY_VERIPHY_CTRL_REG1 24
#define VTSS_PHY_VERIPHY_CTRL_REG1_F_TRIGGER VTSS_PHY_BIT(15)
#define VTSS_PHY_VERIPHY_CTRL_REG1_F_VALID   VTSS_PHY_BIT(14)
#define VTSS_PHY_VERIPHY_CTRL_REG1_M_PAIR_A_DISTANCE VTSS_PHY_ENCODE_BITMASK(8, 6)
#define VTSS_PHY_VERIPHY_CTRL_REG1_M_PAIR_B_DISTANCE VTSS_PHY_ENCODE_BITMASK(0, 6)

// Register 25E1 + Bit fields
#define VTSS_PHY_VERIPHY_CTRL_REG2 25
#define VTSS_PHY_VERIPHY_CTRL_REG2_M_PAIR_C_DISTANCE VTSS_PHY_ENCODE_BITMASK(8, 6)
#define VTSS_PHY_VERIPHY_CTRL_REG2_M_PAIR_D_DISTANCE VTSS_PHY_ENCODE_BITMASK(0, 6)

// Register 26E1 + Bit fields
#define VTSS_PHY_VERIPHY_CTRL_REG3 26
#define VTSS_PHY_VERIPHY_CTRL_REG3_M_PAIR_A_TERMINATION_STATUS VTSS_PHY_ENCODE_BITMASK(12, 4)
#define VTSS_PHY_VERIPHY_CTRL_REG3_M_PAIR_B_TERMINATION_STATUS VTSS_PHY_ENCODE_BITMASK(8, 4)
#define VTSS_PHY_VERIPHY_CTRL_REG3_M_PAIR_C_TERMINATION_STATUS VTSS_PHY_ENCODE_BITMASK(4, 4)
#define VTSS_PHY_VERIPHY_CTRL_REG3_M_PAIR_D_TERMINATION_STATUS VTSS_PHY_ENCODE_BITMASK(0, 4)

// ** Register definitions -  GPIO page**

// Register 18G
#define VTSS_PHY_MICRO_PAGE 18

// Register 19G + Bit fields
#define VTSS_PHY_GPIO_CONTROL_3 19
#define VTSS_PHY_F_GPIO_CONTROL_3_MAC_IF_MODE_SELECT(value)  VTSS_PHY_ENCODE_BITFIELD(value, 14, 2)
#define VTSS_PHY_M_GPIO_CONTROL_3_MAC_IF_MODE_SELECT VTSS_PHY_ENCODE_BITMASK(14, 2)

/* Special page registers */
#define TP_PAGE_CODE 0x2a30
#define TR_PAGE_CODE 0x52b5
#define GP_PAGE_CODE 0x0010

/**
 * Defines for better sharing of code between different Microchip projects
 */
#define vtss_phy_wr(port_no, reg_no, value) phy_write(port_no, reg_no, value)
#define vtss_phy_wr_masked(port_no, reg_no, value, mask) phy_write_masked (port_no, reg_no, value, mask)
#define vtss_phy_rd(port_no, reg_no, value) *value = phy_read(port_no, reg_no)
#define vtss_phy_page_ext(port_no) phy_page_ext(port_no);
#define vtss_phy_page_gpio(port_no) phy_page_gp(port_no);
#define vtss_phy_page_std(port_no) phy_page_std(port_no);
#define VTSS_MSLEEP_LU26(time) delay_1(time + 1)

#define VTSS_RC(x) x

/****************************************************************************
 *
 *
 * Typedefs and enums
 *
 *
 ****************************************************************************/

typedef enum
{
    VTSS_RC_OK                                  =  0,  /**< Success */
    VTSS_RC_ERROR                               = -1,  /**< Unspecified error */
    VTSS_RC_INV_STATE                           = -2,  /**< Invalid state for operation */
    VTSS_RC_INCOMPLETE                          = -3,  /**< Incomplete result */
    VTSS_RC_ERR_MACSEC_NO_SCI                   = -4,  /**< MACSEC Could not find SC (from sci) */
    VTSS_RC_ERR_MACSEC_INVALID_SCI_MACADDR      = -5,   /**< From IEEE 802.1AE-2006, section 9.9 - The 64-bit value FF-FF-FF-FF-FF-FF is never used as an SCI and is reserved for use by implementations to indicate the absence of an SC or an SCI in contexts where an SC can be present */
    VTSS_RC_ERR_CLK_CONF_NOT_SUPPORTED          = -6, /**< The PHY doesn't support clock configuration (for SynceE) */
    VTSS_RC_ERR_KR_CONF_NOT_SUPPORTED           = -7, /**< The PHY doesn't support 10GBASE_KR equalization */
    VTSS_RC_ERR_KR_CONF_INVALID_PARAMETER       = -8, /**< One of the parameters are out of range */
    VTSS_RC_ERR_PHY_BASE_NO_NOT_FOUND           = -50, /**< Port base number (first port within a chip) is not found */
    VTSS_RC_ERR_PHY_6G_MACRO_SETUP              = -51, /**< Setup of 6G macro failed */
    VTSS_RC_ERR_PHY_MEDIA_IF_NOT_SUPPORTED      = -52, /**< PHY does not support the selected media mode */
    VTSS_RC_ERR_PHY_CLK_CONF_NOT_SUPPORTED      = -53, /**< The PHY doesn't support clock configuration (for SynceE) */
    VTSS_RC_ERR_PHY_GPIO_ALT_MODE_NOT_SUPPORTED = -54, /**< The PHY doesn't support the alternative mode for the selected GPIO pin*/
    VTSS_RC_ERR_PHY_GPIO_PIN_NOT_SUPPORTED      = -55, /**< The PHY doesn't support the selected GPIO pin */
    VTSS_RC_ERR_PHY_PORT_OUT_CHANGE             = -56, /**< PHY API called with port number larger than VTSS_PORTS*/
} vtss_rc;

typedef enum {
    VTSS_PHY_MEDIA_IF_CU,                      /**< Copper Interface */
    VTSS_PHY_MEDIA_IF_SFP_PASSTHRU,            /**< Fiber/Cu SFP Pass-thru */
    VTSS_PHY_MEDIA_IF_FI_1000BX,               /**< 1000Base-X */
    VTSS_PHY_MEDIA_IF_FI_100FX,                /**< 100Base-FX */
    VTSS_PHY_MEDIA_IF_AMS_CU_PASSTHRU,         /**< AMS - Cat5/SerDes/CuSFP passthru */
    VTSS_PHY_MEDIA_IF_AMS_FI_PASSTHRU,
    VTSS_PHY_MEDIA_IF_AMS_CU_1000BX,	       /**< AMS - Cat5/1000BX/CuSFP */
    VTSS_PHY_MEDIA_IF_AMS_FI_1000BX,
    VTSS_PHY_MEDIA_IF_AMS_CU_100FX	,          /**< AMS - Cat5/100FX/CuSFP */
    VTSS_PHY_MEDIA_IF_AMS_FI_100FX
} vtss_phy_media_interface_t;

/** \brief The different interfaces for connecting MAC and PHY */
typedef enum
{
    VTSS_PORT_INTERFACE_NO_CONNECTION, /**< No connection */
    VTSS_PORT_INTERFACE_LOOPBACK,      /**< Internal loopback in MAC */
    VTSS_PORT_INTERFACE_INTERNAL,      /**< Internal interface */
    VTSS_PORT_INTERFACE_MII,           /**< MII (RMII does not exist) */
    VTSS_PORT_INTERFACE_GMII,          /**< GMII */
    VTSS_PORT_INTERFACE_RGMII,         /**< RGMII */
    VTSS_PORT_INTERFACE_TBI,           /**< TBI */
    VTSS_PORT_INTERFACE_RTBI,          /**< RTBI */
    VTSS_PORT_INTERFACE_SGMII,         /**< SGMII */
    VTSS_PORT_INTERFACE_SERDES,        /**< SERDES */
    VTSS_PORT_INTERFACE_VAUI,          /**< VAUI */
    VTSS_PORT_INTERFACE_100FX,         /**< 100FX */
    VTSS_PORT_INTERFACE_XAUI,          /**< XAUI */
    VTSS_PORT_INTERFACE_RXAUI,         /**< RXAUI */
    VTSS_PORT_INTERFACE_XGMII,         /**< XGMII */
    VTSS_PORT_INTERFACE_SPI4,          /**< SPI4 */
    VTSS_PORT_INTERFACE_SFP_CU_SGMII,  /**< COPPER SFP */
    VTSS_PORT_INTERFACE_QSGMII         /**< QSGMII */
} vtss_port_interface_t;

#if TRANSIT_EEE
#if VTSS_ATOM12_A
typedef enum {
    ATOM12_EN_BOTH, /* 00: power down neither.   */
    ATOM12_EN_ADC,  /* 01: power down ADCs only. */
    ATOM12_EN_VGA,  /* 10: power down VGAs only. */
    ATOM12_EN_NONE, /* 11: power down both.      */
} phy_atom12_en_t;
#endif
#endif

/****************************************************************************
 *
 *
 * Structures
 *
 *
 ****************************************************************************/

/**
 * PHY reset request parameters.
 */
typedef struct {

    /**
     * For dual media port in AMS mode- True CU port is preferred.
     */
    BOOL                       cu_preferred;

    /**
     * Selected MAC interface.
     */
    vtss_port_interface_t      mac_if;

    /**
     * Selected Media interface.
     */
    vtss_phy_media_interface_t media_if;

} vtss_phy_reset_conf_t;

/****************************************************************************
 *
 *
 * Functions
 *
 *
 ****************************************************************************/

/*
 * PHY MIIM and MMD Register Access Functions
 */

ushort  phy_read                (const vtss_port_no_t port_no,
                                 const uchar          reg_no) small;

void    phy_write               (const vtss_port_no_t port_no,
                                 const uchar          reg_no,
                                 const u16            value) small;

void    phy_write_masked        (const vtss_port_no_t port_no,
                                 const uchar          reg_no,
                                 const u16            value,
                                 const u16            mask) small;

int     phy_mmd_rd              (const vtss_port_no_t port_no,
                                 const u16            devad,
                                 const u16            addr);

/*
 * PHY Register Page Functions
 */

void    phy_page_std            (vtss_port_no_t port_no);

#if VTSS_ATOM12_B || VTSS_ATOM12_C || VTSS_ATOM12_D || VTSS_TESLA
void    phy_page_ext            (vtss_port_no_t port_no);
void    phy_page_ext2           (vtss_port_no_t port_no);
#endif  // VTSS_ATOM12_B || VTSS_ATOM12_C || VTSS_ATOM12_D || VTSS_TESLA

void    phy_page_gp             (vtss_port_no_t port_no);

void    phy_page_tp             (vtss_port_no_t port_no);

void    phy_page_tr             (vtss_port_no_t port_no);

/*
 * Link status functions
 */

uchar   phy_get_speed_and_fdx   (vtss_port_no_t port_no);
void    phy_set_forced_speed    (vtss_port_no_t port_no, uchar link_mode);
bool    phy_link_status         (vtss_cport_no_t chip_port) small;

/*
 * Configure and reset functions
 */

void    phy_do_link_up_settings(vtss_cport_no_t chip_port, uchar link_mode, uchar power_mode);
void    phy_do_link_down_settings
                                (vtss_port_no_t port_no);
void    phy_power_down          (vtss_port_no_t port_no);

void    phy_receiver_reconfig(vtss_cport_no_t chip_port, uchar power_mode);

void    phy_restart_aneg        (vtss_port_no_t port_no);

void    phy_pre_reset           (vtss_port_no_t port_no);
void    phy_reset               (vtss_port_no_t port_no);
void    phy_post_reset          (vtss_port_no_t port_no);
void    phy_setup               (vtss_port_no_t port_no);

/*
 * EEE functions
 */

void    vtss_phy_eee_ena_private(const vtss_port_no_t port_no,
                                 const BOOL           enable);

vtss_rc vtss_phy_eee_link_partner_advertisements_get
                                (const vtss_cport_no_t chip_port,
                                 char                  *advertisements);

/*
 * Temperature API
 */
#if TRANSIT_FAN_CONTROL || TRANSIT_THERMAL
void    phy_init_temp_mode_regs (vtss_port_no_t port_no);
ushort  phy_read_temp_reg       (vtss_port_no_t port_no);
#endif // TRANSIT_FAN_CONTROL || TRANSIT_THERMAL

/*
 * Test Purpose
 */
#if TRANSIT_EEE
#if VTSS_ATOM12_A
void vga_adc_debug(vtss_port_no_t   port_no,
                   phy_atom12_en_t  vga_adc_pwr);
#endif
#endif

#endif /* __PHYDRV_H__ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
