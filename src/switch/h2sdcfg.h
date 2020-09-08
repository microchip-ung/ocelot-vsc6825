//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __H2SDCFG_H__
#define __H2SDCFG_H__

/** \brief Serdes macro mode */
typedef enum
{
    VTSS_SERDES_MODE_DISABLE,       /**< Disable serdes */
    VTSS_SERDES_MODE_XAUI_12G,      /**< XAUI 12G mode  */
    VTSS_SERDES_MODE_XAUI,          /**< XAUI 10G mode  */
    VTSS_SERDES_MODE_RXAUI,         /**< RXAUI 10G mode */
    VTSS_SERDES_MODE_RXAUI_12G,     /**< RXAUI 12G mode */
    VTSS_SERDES_MODE_2G5,           /**< 2.5G mode      */
    VTSS_SERDES_MODE_QSGMII,        /**< QSGMII mode    */
    VTSS_SERDES_MODE_SGMII,         /**< SGMII mode     */
    VTSS_SERDES_MODE_100FX,         /**< 100FX mode     */
    VTSS_SERDES_MODE_1000BaseX,     /**< 1000BaseX mode */
    VTSS_SERDES_MODE_SFI,           /**< LAN/10G mode   */
    VTSS_SERDES_MODE_SFI_DAC,       /**< LAN/10G DAC(CU)*/
    VTSS_SERDES_MODE_SFI_SR,        /**< Short Range > */
    VTSS_SERDES_MODE_SFI_ZR,        /**< ZR with APC hardware algorithm > */
    VTSS_SERDES_MODE_SFI_BP,        /**< Backplane > */
    VTSS_SERDES_MODE_SFI_B2B,       /**< Bord to Board > */
    VTSS_SERDES_MODE_SFI_PR_NONE,   /**< No preset > */
    VTSS_SERDES_MODE_IDLE,          /**< Send idles (port appears as down for LP) */
    VTSS_SERDES_MODE_TEST_MODE,     /**< Send fixed test pattern (port appears as up for LP, but no frame rx/tx) */
    VTSS_SERDES_MODE_PCIE           /**< PCIE mode */
} vtss_serdes_mode_t;

void h2_sd6g_write(ulong addr);
void h2_sd6g_read(ulong addr);
void h2_sd6g_cfg(vtss_serdes_mode_t mode, ulong addr);

void h2_serdes_macro_config (void);
void h2_sd6g_cfg_change(vtss_serdes_mode_t mode, ulong addr);
uchar h2_serdes_macro_phase_loop_locked (void);

void h2_sd1g_write(ulong addr);
void h2_sd1g_read(ulong addr);
void h2_sd1g_cfg(vtss_serdes_mode_t mode, ulong addr);

#endif  // __H2SDCFG_H__
