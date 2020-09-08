//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT




#ifndef __H2E2ETC_H__
#define __H2E2ETC_H_

#include "common.h"

#if TRANSIT_E2ETC
/** \brief EEE port configuration */
typedef struct
{
    BOOL              e2etc_ena;           /**< PTP enabled */
    ushort            ingress_latency;     /**< Expected delay at ingress */
    ushort            egress_latency;      /**< Expected delay at egress  */
} e2etc_conf_t;

/**
 * \brief  Initialize PTP 
 * \return None.
 **/

 void h2_e2etc_init(void);

/**
 * \brief  Set E2E TC configuration
 *
 * \param cport_no [IN]  Chip Port number.
 *
 * \param conf [IN]     New configuration for the port.
 *
 * \return None.
 **/
void h2_e2etc_conf_set(const uchar cport_no, const e2etc_conf_t * const conf);

/**
 * \brief  Set ingress and egress latencies according to speed 
 *
 * \param cport_no [IN]  Chip Port number.
 *
 * \return None.
 **/

void e2etc_latency_set(const uchar chip_port);
#endif
#endif /* __H2_E2ETC_H__ */

