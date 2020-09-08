//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT




#ifndef __H2TCAM_H__
#define __H2TCAM_H_

#include "common.h"

#if TRANSIT_TCAM_IS2

#define TCAM_ENTRY_WIDTH 12
/** \brief TCAM configuration types */
typedef enum {
    TCAM_E2E_TC_PTP_ETH,   /**< PTP over ethernet */
    TCAM_E2E_TC_PTP_IP,    /**< PTP over IP       */
    TCAM_E2E_TC_PTP_IPV6,    /**< PTP over IPv6     */
} tcam_e2e_tc_t;

/** \brief TCAM Target types */
typedef enum {
    TCAM_TARGET_ES0,   /**< Target ES0 */
    TCAM_TARGET_IS1,   /**< Target IS1 */
    TCAM_TARGET_IS2,   /**< Target IS2 */
} tcam_target_t;

/** \brief TCAM configuration */
typedef struct 
{
    u32 tcam_entry[TCAM_ENTRY_WIDTH];
    u32 tcam_mask[TCAM_ENTRY_WIDTH];
    u32 tcam_action[2];
    u32 tcam_count;
    u8  tg;
    u8  entry_length;
    u8  action_length;
} tcam_data_conf_t;

/**
 * \brief  Initialize TCAM 
 * \return None.
 **/

 void h2_tcam_init(void);

 /**
 * \brief  Get current ACL count 
 *
 * \param  trgt [IN]  Target(IS1,IS2,ES0).
 *
 * \return ACL count.
 **/

 u16 h2_tcam_count_get(const tcam_target_t trgt);

/**
 * \brief  Set TCAM configuration of E2E TC
 *
 * \param conf [IN]     New configuration for the TCAM entry.
 *
 * \return None.
 **/
void h2_tcam_e2e_tc_set(const tcam_e2e_tc_t conf);

/**
 * \brief  Get TCAM configuration 
 *
 * \param entry_id [IN]     TCAM entry id.
 *
 * \param conf     [OUT]    Configuration of the tcam entry.
 *
 * \return None.
 **/

void h2_tcam_e2e_tc_get(u16 entry_id,tcam_data_conf_t *conf); 
#endif
#endif /* __H2_E2ETC_H__ */

