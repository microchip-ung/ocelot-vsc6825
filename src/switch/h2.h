//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __H2_H__
#define __H2_H__

#include "common.h"
#include "phyconf.h"

/****************************************************************************
 *
 *
 * Defines
 *
 *
 ****************************************************************************/

#define VTSS_PGIDS      48

/* Pseudo PGID for IPv4/IPv6 MC */
#define VTSS_PGID_NONE  VTSS_PGIDS
#define MAX_PGIDS       (VTSS_PGIDS - NO_OF_CHIP_PORTS - 1) /* First MAX_PORT reserved for auto-learned MACs */
#define BAD_PGID        (0xFF)


/*
 * This clause is dirty, but is a must when debugging hard-to-find
 * vtss_update_masks() issues.
 */

#define VTSS_UPDATE_MASKS_DEBUG()
#if TRANSIT_LACP
#define TOTAL_AGGRS     (MAX_AGGR_GROUP + VTSS_LACP_MAX_AGGR)
#else
#define TOTAL_AGGRS     MAX_AGGR_GROUP
#endif

/****************************************************************************
 *
 *
 * Functions
 *
 *
 ****************************************************************************/

void                h2_reset                (void) small;
void                h2_post_reset           (void);
void                h2_init_ports           (void);
void                h2_setup_port           (vtss_cport_no_t chip_port, uchar link_mode);
#if H2_ID_CHECK
BOOL                h2_chip_family_support  (void) small;
#endif // H2_ID_CHECK
void                h2_enable_exc_col_drop  (vtss_cport_no_t chip_port, uchar drop_enable);
/**
 * Update source, destination and aggregation masks
 */
void                vtss_update_masks       (void);
void                vtss_show_masks         (void);

extern void             vtss_set_aggr_group(uchar group, port_bit_mask_t members);
extern port_bit_mask_t  vtss_get_aggr_group(uchar group);

void h2_chip_reset(void);

#if defined(NPI_CHIP_PORT) && NPI_CHIP_PORT != NPI_ACT_NORMAL_PORT
/* When a port is set as NPI port, the frames contain the 16-byte CPU
 * injection header as the first part of the frame. Frames are forwarded based
 * on the contents in the CPU injection header instead of normal forwarding.
 *
 * No prefix:
 * <CPU header> <Original frame>
 * 128 bits
 *
 * Short prefix:
 * <8880>  <000A>  <CPU header> <Original frame>
 * 16 bits 16 bits 128 bits
 *
 * Long prefix:
 * <FF-FF-FF-FF-FF-FF> <FE-FF-FF-FF-FF-FF> <8880>  <000A>  <CPU header> <Original frame>
 * 48 bits             48 bits             16 bits 16 bits 128 bits
 *
 * To get the deatails of CPU header, please refer to the chip specification
 * Table 121 - CPU Injection Header.
 */
typedef enum
{
    H2_PREFIX_HEADER_MODE_NORMAL,   /* 0: No CPU header (normal frames) */
    H2_PREFIX_HEADER_MODE_NONE,     /* 1: CPU header without prefix */
    H2_PREFIX_HEADER_MODE_SHORT,    /* 2: CPU header with short prefix */
    H2_PREFIX_HEADER_MODE_LONG      /* 3: CPU header with long prefix */
} h2_prefix_header_mode_t;

typedef struct
{
    vtss_cport_no_t         chip_port;          /* The chip port No. of NPI (Only one port can be an NPI at the same time.) */
    BOOL                    mode_enabled;       /* TRUE: Mode enabled, FALSE: Mode disabled */
    uchar                   queue_mask;         /* (Insignificant when mode_enabled=FALSE) The bitmask presents which queue are redirected to the NPI */
    h2_prefix_header_mode_t prefix_header_mode; /* (Insignificant when mode_enabled=FALSE) Prefix header mode */
} h2_npi_conf_t;

/* Set NPI configuration
 * Notices that the original NPI port will be inactive after applied the new setting.
 */
void npi_port_set(h2_npi_conf_t *npi_conf);
#endif // NPI_CHIP_PORT && NPI_CHIP_PORT != NPI_ACT_NORMAL_PORT

#endif /* __H2_H__ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
