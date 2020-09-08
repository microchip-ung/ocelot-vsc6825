//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __PHY_FAMILY_H__
#define __PHY_FAMILY_H__

#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "phydrv.h"

/****************************************************************************
 *
 *
 * Defines
 *
 *
 ****************************************************************************/

/* Define revision for the different chips. */
#define VTSS_PHY_ATOM_REV_A 0
#define VTSS_PHY_ATOM_REV_B 1
#define VTSS_PHY_ATOM_REV_C 2
#define VTSS_PHY_ATOM_REV_D 3

// Tesla family
#define VTSS_PHY_TESLA_REV_A 0
#define VTSS_PHY_TESLA_REV_B 1
#define VTSS_PHY_TESLA_REV_D 2

// Viper family
#define VTSS_PHY_VIPER_REV_A 0

// Elise family
#define VTSS_PHY_ELISE_REV_A 0

/****************************************************************************
 *
 *
 * Typedefs and enums
 *
 *
 ****************************************************************************/

typedef enum {
    PHY_VENDOR_UNKNOWN,
    PHY_VENDOR_VTSS,
} phy_vendor_t;

/* PHY family */
typedef enum {
    VTSS_PHY_FAMILY_NONE,     /* Unknown */
    VTSS_PHY_FAMILY_COBRA,    /* VSC8211, VSC8221 */
    VTSS_PHY_FAMILY_QUATTRO,  /* VSC8224, VSC8234, VSC8244 */
    VTSS_PHY_FAMILY_SPYDER,   /* VSC8538, VSC8558, VSC8658 */
    VTSS_PHY_FAMILY_ENZO,     /* VSC8664 */
    VTSS_PHY_FAMILY_ATOM,     /* VSC8512, VSC8522 */
    VTSS_PHY_FAMILY_LUTON26,  /* VSC7420 */
    VTSS_PHY_FAMILY_TESLA,    /* VSC8574, VSC8504, VSC8552 */
    VTSS_PHY_FAMILY_VIPER,    /* VSC8582, VSC8584, VSC8575, VSC8564, VSC8586 */
    VTSS_PHY_FAMILY_ELISE,    /* VSC8514 */
    VTSS_PHY_FAMILY_FERRET,   /* VSC7512 */
} vtss_phy_family_t;

/*****************************************************************************
 *
 *
 * PHY ids
 *
 *
 *
 ****************************************************************************/

#define PHY_OUI_VTSS_1   0x000fc400
#define PHY_OUI_VTSS_2   0x00070400

#define PHY_ID_VTSS_8658_A 0x00070750

#define PHY_ID_VTSS_8558_A 0x00070580
#define PHY_ID_VTSS_8538_A 0x00070480

#define PHY_ID_VTSS_8558 0x00070780 /* no revision A/0 actually exists */
#define PHY_ID_VTSS_8538 0x00070680 /* no revision A/0 actually exists */

#define PHY_ID_VTSS_8211 0x000FC4B0
#define PHY_ID_VTSS_8221 0x000FC550

#define PHY_ID_VTSS_8224 0x000FC580
#define PHY_ID_VTSS_8234 0x000FC780
#define PHY_ID_VTSS_8244 0x000FC6C0

#define PHY_ID_VTSS_7422 0x000706D0 /* TBD */

#define PHY_ID_VTSS_8512 0x000706E0
#define PHY_ID_VTSS_8522 0x000706F0

/* Tesla */
#define PHY_ID_VTSS_8574 0x000704A0
#define PHY_ID_VTSS_8504 0x000704C0
#define PHY_ID_VTSS_8552 0x000704E0

/* Elise */
#define PHY_ID_VTSS_8514 0x00070670

/* Enzo */
#define PHY_ID_VTSS_8664 0x00070660

/* Mini */
#define PHY_ID_VTSS_7512 0x00070540

typedef enum {
    PHY_MODEL_NONE, // Not PHY mode, for instance, Serdes mode
    PHY_MODEL_UNKNOWN,

    PHY_MODEL_VTSS_8211,
    PHY_MODEL_VTSS_8221,

    PHY_MODEL_VTSS_8224,
    PHY_MODEL_VTSS_8234,
    PHY_MODEL_VTSS_8244,

    PHY_MODEL_VTSS_8538,
    PHY_MODEL_VTSS_8558,
    PHY_MODEL_VTSS_8658,

    PHY_MODEL_VTSS_8664,

    PHY_MODEL_VTSS_7422,
    PHY_MODEL_VTSS_8512,

    PHY_MODEL_VTSS_8504,
    PHY_MODEL_VTSS_8552,

    PHY_MODEL_VTSS_8514,
	PHY_MODEL_VTSS_7512,
} phy_model_t;

/****************************************************************************
 *
 *
 * Structures
 *
 *
 ****************************************************************************/

/**
 * The complete description of a PHY indentifier
 */
typedef struct {
    phy_vendor_t        vendor;
    vtss_phy_family_t   family;
    phy_model_t         model; /* unique software id used by SW, *not* the same as model number */
    uchar               revision;
} phy_id_t;

/****************************************************************************
 *
 *
 * Functions
 *
 *
 ****************************************************************************/

#if VTSS_QUATTRO
vtss_rc init_seq_8224(
    vtss_port_no_t          port_no,
    phy_id_t                *phy_id
);
#endif /* VTSS_QUATTRO */

#if VTSS_SPYDER
vtss_rc init_seq_8538(
    vtss_port_no_t          port_no,
    phy_id_t                *phy_id
);
#endif /* VTSS_SPYDER */

#if VTSS_SPYDER
vtss_rc init_seq_8634_8664(
    vtss_port_no_t          port_no,
    phy_id_t                *phy_id
);
#endif /* VTSS_SPYDER */

#if VTSS_ATOM12
/**
 * Function to be called at startup before the phy ports are reset.
 *
 * @param   family      The family ID of PHY chip.
 * @param   port_no     The phy port number stating from 0.
 */
vtss_rc atom12_init_seq_pre(
    vtss_port_no_t          port_no,
    phy_id_t                *phy_id
);

vtss_rc atom12_init_seq(
    vtss_port_no_t          port_no,
    phy_id_t                *phy_id
);

/**
 * Setup the MAC interface mode and media interface mode.
 */
vtss_rc atom12_mac_media_if_setup(
    vtss_port_no_t          port_no,
    vtss_phy_reset_conf_t   *conf
);

/**
 * Read temperature from PHY chip.
 */
vtss_rc atom12_read_temp_reg(
    vtss_port_no_t          port_no,
    ushort                  *temp
);
#endif /* VTSS_ATOM12 */

#if VTSS_ELISE
/**
 * Function to be called at startup before the phy ports are reset.
 *
 * @param   family      The family ID of PHY chip.
 * @param   port_no     The phy port number stating from 0.
 */
vtss_rc elise_init_seq_pre(
    vtss_port_no_t          port_no,
    phy_id_t                *phy_id
);

vtss_rc elise_init_seq(
    vtss_port_no_t          port_no,
    phy_id_t                *phy_id
);

/**
 * Setup the MAC interface mode and media interface mode.
 */
vtss_rc elise_mac_media_if_setup(
    vtss_port_no_t          port_no,
    vtss_phy_reset_conf_t   *conf
);

/**
 * Read temperature from PHY chip.
 */
vtss_rc elise_read_temp_reg(
    vtss_port_no_t          port_no,
    ushort                  *temp
);
#endif /* VTSS_ELISE */


#if VTSS_TESLA
/**
 * Function to be called at startup before the phy ports are reset.
 *
 * @param   family      The family ID of PHY chip.
 * @param   port_no     The phy port number stating from 0.
 */
vtss_rc tesla_init_seq_pre(
    vtss_port_no_t          port_no,
    phy_id_t                *phy_id
);

vtss_rc tesla_init_seq(
    vtss_port_no_t          port_no,
    phy_id_t                *phy_id
);

/**
 * Setup the MAC interface mode and media interface mode.
 */
vtss_rc tesla_mac_media_if_setup
(
    vtss_port_no_t          port_no,
    vtss_phy_reset_conf_t   *conf
);

/**
 * Read temperature from PHY chip.
 */
vtss_rc tesla_read_temp_reg
(
    vtss_port_no_t  port_no,
    ushort          *temp
);

#endif /* VTSS_TESLA */

#if VTSS_COBRA 
/*
 * Function to be called at startup before the phy ports are reset.

 * @param   port_no     The phy port number stating from 0.
 */
vtss_rc cobra_init_seq(vtss_port_no_t port_no);

/* Disable Power Savings */
void cobra_power_saving_disable(vtss_cport_no_t chip_port);

/*
 * Setup the MAC interface mode and media interface mode.
 */
vtss_rc cobra_mac_media_if_setup
(
    vtss_port_no_t port_no, 
    vtss_phy_reset_conf_t *conf
);
#endif /* VTSS_COBRA */


#endif /* __PHY_FAMILY_H__ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
