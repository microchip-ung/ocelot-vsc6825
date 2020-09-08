//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __H2MACTAB_H__
#define __H2MACTAB_H__

#if TRANSIT_UNMANAGED_MAC_OPER_SET
/* Add/Delete MAC address entry
 * Only support unicast and one port setting in port_mask parameter currently.
 */
void h2_mactab_set(const mac_tab_t xdata *mac_tab_entry_ptr, BOOL xdata is_add);
#endif // TRANSIT_UNMANAGED_MAC_OPER_SET

#if TRANSIT_UNMANAGED_MAC_OPER_GET
/* GET/GETNEXT MAC address entry
 * Return 0xFFFFFFFF when entry is invalid. Otherwize, the operation is success.
 */
ulong h2_mactab_get_next(mac_tab_t xdata *mac_tab_entry_ptr, BOOL xdata *ipmc_entry, BOOL xdata is_getnext);
#endif // TRANSIT_UNMANAGED_MAC_OPER_GET

void h2_mactab_agetime_set(void);
void h2_mactab_flush_port(vtss_cport_no_t chip_port);
void h2_mactab_age(uchar pgid_age, uchar pgid, uchar vid_age, ushort vid);
void h2_mactab_clear(void);

#endif // __H2MACTAB_H__
