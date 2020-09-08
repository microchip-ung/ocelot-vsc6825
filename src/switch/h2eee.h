//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



/**
 * \file h2eee.h
 * \brief Luton26 EEE API
 * \details This header file describes Luton26 eee functions.
 */

#ifndef __H2_EEE_H__
#define __H2_EEE_H__

/** \brief EEE port configuration */
typedef struct
{
    BOOL              eee_ena;           /**< EEE enabled */
    uchar             eee_fast_queues;   /**< Queues set in this mask will activate egress path as soon as any data is avaiable. */
    ushort            tx_tw;             /**< Time from path is activated until frame transmission restarted.*/
    ushort            rx_tw;
    uchar             lp_advertisement;  /**  Link partner EEE advertisement. Bit 0 = Link partner advertises 100BASE-T capability. Bit 1 = Link partner advertises 1000BASE-T capability. */
} eee_port_conf_t;

/**
 * \brief  Set EEE configuration
 *
 * \param port_no [IN]  Port number.
 *
 * \param conf [IN]     New configuration for the port.
 *
 * \return None.
 **/
void h2_eee_port_conf_set(const vtss_cport_no_t chip_port, const eee_port_conf_t * const conf);

#endif /* __H2_EEE_H__ */










