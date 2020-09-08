//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "lldp_os.h"
#include "lldp_sm.h"
#include "lldp.h"
#include "lldp_private.h"
#include "vtss_common_os.h"
#include "taskdef.h"
#if TRANSIT_LLDP

/*****************************************************************************
 *
 *
 * Defines
 *
 *
 *
 ****************************************************************************/

/*
** Definitions for easing state machine implementation
*/

#define SM_STATE(machine, state) \
static void sm_ ## machine ## _ ## state ## _Enter(lldp_sm_t xdata * sm)

#define SM_ENTRY(machine, _state, _data) SM_ENTRY_DBG(machine, _state, _data)

#define SM_ENTRY_DBG(machine, _state, _data) \
if(sm->_data.state != _state){\
  VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("Port %u " #machine " -> " #_state, (unsigned)sm->port_no));\
}\
sm->_data.state = _state;

#define SM_ENTER(machine, state) sm_ ## machine ## _ ## state ## _Enter(sm)

#define SM_STEP(machine) \
static void sm_ ## machine ## _Step(lldp_sm_t xdata *sm)

#define SM_STEP_RUN(machine) sm_ ## machine ## _Step(sm)


/*
** Misc action functions
*/
#define txInitializeLLDP() lldp_tx_initialize_lldp(sm)
#define mibConstrInfoLLDPDU() lldp_construct_info_lldpdu(sm->port_no)
#define mibConstrShutdownLLDPDU() lldp_construct_shutdown_lldpdu(sm->port_no)
#define txFrame() lldp_tx_frame(sm->port_no)
#define rxInitializeLLDP() lldp_rx_initialize_lldp (sm->port_no)
#define rxProcessFrame() lldp_rx_process_frame(sm)

/* These "functions" need not have a bodyas the MIB module fetches info from the lldp_remote module */
#define mibDeleteObjects() VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("Port %u - mibDeleteObjects", (unsigned)sm->port_no));
#define mibUpdateObjects() VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("Port %u - mibUpdateObjects", (unsigned)sm->port_no));

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
 * Public data
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

/*
** in all of these functions, the variable sm is a pointer to the current
** set of state machine set
*/

SM_STATE(TX, TX_LLDP_INITIALIZE)
{
    SM_ENTRY(TX, TX_LLDP_INITIALIZE, tx);

    txInitializeLLDP();
}

SM_STATE(TX, TX_IDLE)
{
    lldp_timer_t msgTxInterval;

    SM_ENTRY(TX, TX_IDLE, tx);

    msgTxInterval = lldp_os_get_msg_tx_interval();
    sm->tx.txTTL = MIN(65535, (msgTxInterval * lldp_os_get_msg_tx_hold()));
    sm->timers.txTTR = msgTxInterval;
    sm->tx.somethingChangedLocal = LLDP_FALSE;
    sm->timers.txDelayWhile = lldp_os_get_tx_delay();

    sm->tx.re_evaluate_timers = LLDP_FALSE;
}

SM_STATE(TX, TX_SHUTDOWN_FRAME)
{
    SM_ENTRY(TX, TX_SHUTDOWN_FRAME, tx);

    mibConstrShutdownLLDPDU();
    txFrame();
    sm->timers.txShutdownWhile = lldp_os_get_reinit_delay();
}

SM_STATE(TX, TX_INFO_FRAME)
{
    SM_ENTRY(TX, TX_INFO_FRAME, tx)
    mibConstrInfoLLDPDU();
    txFrame();
}

SM_STEP(TX)
{
    if(sm->initialize || sm->portEnabled == LLDP_FALSE) {
        SM_ENTER(TX, TX_LLDP_INITIALIZE);
        return;
    }

    switch(sm->tx.state) {
    case TX_LLDP_INITIALIZE:
        if((sm->adminStatus == LLDP_ENABLED_RX_TX) || (sm->adminStatus == LLDP_ENABLED_TX_ONLY)) {
            SM_ENTER(TX, TX_IDLE);
        }
        break;

    case TX_IDLE:
        if((sm->adminStatus == LLDP_DISABLED) || (sm->adminStatus == LLDP_ENABLED_RX_ONLY)) {
            TASK(SUB_TASK_ID_LLDP_TX, SM_ENTER(TX, TX_SHUTDOWN_FRAME));
        } else if((sm->timers.txDelayWhile == 0) && ((sm->timers.txTTR == 0) || (sm->tx.somethingChangedLocal == LLDP_TRUE))) {
            TASK(SUB_TASK_ID_LLDP_TX, SM_ENTER(TX, TX_INFO_FRAME));
        } else if(sm->tx.re_evaluate_timers) {
            SM_ENTER(TX, TX_IDLE);
        }
        break;

    case TX_SHUTDOWN_FRAME:
        if(sm->timers.txShutdownWhile == 0) {
            SM_ENTER(TX, TX_LLDP_INITIALIZE);
        }
        break;

    case TX_INFO_FRAME:
        /* UCT */
        SM_ENTER(TX, TX_IDLE);
        break;

    default:
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("Port %u - Unhandled TX state %u", (unsigned)sm->port_no, (unsigned)sm->tx.state));
        break;
    }
}



SM_STATE(RX, LLDP_WAIT_PORT_OPERATIONAL)
{
    SM_ENTRY(RX, LLDP_WAIT_PORT_OPERATIONAL, rx);
}

SM_STATE(RX, DELETE_AGED_INFO)
{
    SM_ENTRY(RX, DELETE_AGED_INFO, rx);

    sm->rx.somethingChangedRemote = LLDP_FALSE;
    mibDeleteObjects();
    sm->rx.rxInfoAge = LLDP_FALSE;
    sm->rx.somethingChangedRemote = LLDP_TRUE;
}

SM_STATE(RX, RX_LLDP_INITIALIZE)
{
    SM_ENTRY(RX, RX_LLDP_INITIALIZE, rx);

    rxInitializeLLDP();
    sm->rx.rcvFrame = LLDP_FALSE;
}

SM_STATE(RX, RX_WAIT_FOR_FRAME)
{
    SM_ENTRY(RX, RX_WAIT_FOR_FRAME, rx);

    sm->rx.badFrame = LLDP_FALSE;
    sm->rx.rxInfoAge = LLDP_FALSE;
    sm->rx.somethingChangedRemote = LLDP_FALSE;
}

SM_STATE(RX, RX_FRAME)
{
    SM_ENTRY(RX, RX_FRAME, rx);

    sm->rx.badFrame = LLDP_FALSE;
    sm->rx.rxChanges = LLDP_FALSE;
    sm->rx.rcvFrame = LLDP_FALSE;
    rxProcessFrame();
}

SM_STATE(RX, DELETE_INFO)
{
    SM_ENTRY(RX, DELETE_INFO, rx);

    mibDeleteObjects();
    sm->rx.somethingChangedRemote = LLDP_TRUE;
}

SM_STATE(RX, UPDATE_INFO)
{
    SM_ENTRY(RX, UPDATE_INFO, rx);

    mibUpdateObjects();
    sm->rx.somethingChangedRemote = LLDP_TRUE;
}

SM_STEP(RX)
{
    if(sm->initialize || ((sm->rx.rxInfoAge == LLDP_FALSE) && (sm->portEnabled == LLDP_FALSE))) {
        SM_ENTER(RX, LLDP_WAIT_PORT_OPERATIONAL);
        return;
    }

    switch(sm->rx.state) {
    case LLDP_WAIT_PORT_OPERATIONAL:
        if(sm->rx.rxInfoAge == LLDP_TRUE) {
            SM_ENTER(RX, DELETE_AGED_INFO);
        } else {
            SM_ENTER(RX, RX_LLDP_INITIALIZE);
        }
        break;

    case DELETE_AGED_INFO:
        /* UCT */
        SM_ENTER(RX, LLDP_WAIT_PORT_OPERATIONAL);
        break;

    case RX_LLDP_INITIALIZE:
        if((sm->adminStatus == LLDP_ENABLED_RX_TX) ||
                (sm->adminStatus == LLDP_ENABLED_RX_ONLY)) {
            SM_ENTER(RX, RX_WAIT_FOR_FRAME);
        }
        /* IEEE802.1AB doesn't specifically specify that we shall clear rcvFrame, but in practise
        ** if we don't do it, we might end up trying to read and parse arbitrary input data
        ** because the rcvFrame is stuck with being TRUE until rx is enabled */
        else {
            /* this effectively throws away "old frames" when rx is disabled */
            sm->rx.rcvFrame = LLDP_FALSE;
        }
        break;

    case RX_WAIT_FOR_FRAME:
        if(sm->rx.rxInfoAge == LLDP_TRUE) {
            SM_ENTER(RX, DELETE_INFO);
        } else if(sm->rx.rcvFrame == LLDP_TRUE) {
            SM_ENTER(RX, RX_FRAME);
        } else if((sm->adminStatus == LLDP_DISABLED) ||
                  (sm->adminStatus == LLDP_ENABLED_TX_ONLY)) {
            SM_ENTER(RX, RX_LLDP_INITIALIZE);
        }
        break;

    case RX_FRAME:
        if(sm->rx.rxTTL == 0) {
            SM_ENTER(RX, DELETE_INFO);
        } else if((sm->rx.rxTTL != 0) && (sm->rx.rxChanges == LLDP_TRUE)) {
            SM_ENTER(RX, UPDATE_INFO);
        } else if((sm->rx.badFrame == LLDP_TRUE) || ((sm->rx.rxTTL != 0) && (sm->rx.rxChanges == LLDP_FALSE))) {
            SM_ENTER(RX, RX_WAIT_FOR_FRAME);
        }
        break;

    case UPDATE_INFO:
        /* UCT */
        SM_ENTER(RX, RX_WAIT_FOR_FRAME);
        break;

    case DELETE_INFO:
        /* UCT */
        SM_ENTER(RX, RX_WAIT_FOR_FRAME);
        break;

    default:
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("Port %u - Unhandled RX state %u", (unsigned)sm->port_no, (unsigned)sm->rx.state));
        break;
    }
}

/*
** Step state machines
*/

void lldp_sm_step (lldp_sm_t xdata * sm)
{
    lldp_u8_t prev_tx;
    lldp_u8_t prev_rx;

    do {
        /* register previous states of state machines */
        prev_tx = sm->tx.state;
        prev_rx = sm->rx.state;

        /* run state machines */
        SM_STEP_RUN(TX);
        SM_STEP_RUN(RX);

        /* repeat until no changes */
    } while ( prev_tx != sm->tx.state ||
              prev_rx != sm->rx.state );
}

void lldp_sm_init (lldp_sm_t xdata * sm, lldp_port_t port)
{
    VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("Port %u - initializing", (unsigned)port));

    sm->port_no = port;
    sm->adminStatus = lldp_os_get_admin_status(port);

    /*
    ** set initialize, and step machine single time before de-asserting initialize
    */

    sm->portEnabled = LLDP_FALSE;

    lldp_sm_step(sm);
    sm->initialize = LLDP_FALSE;
}

void lldp_port_timers_tick(lldp_sm_t xdata *sm)
{
    /* Timer handling */

    if(sm->timers.txShutdownWhile > 0) {
        sm->timers.txShutdownWhile--;
    }

    if(sm->timers.txDelayWhile > 0) {
        sm->timers.txDelayWhile--;
    }

    if(sm->timers.txTTR > 0) {
        sm->timers.txTTR--;
    }

    /* step state machines */
    lldp_sm_step(sm);
}

#endif

