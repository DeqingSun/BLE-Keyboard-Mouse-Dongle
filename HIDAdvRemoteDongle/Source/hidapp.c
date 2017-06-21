/******************************************************************************

 @file  hidapp.c

 @brief This file contains the BLE sample application for HID dongle
        application.

 Group: WCS, BTS
 Target Device: CC2540, CC2541

 ******************************************************************************
 
 Copyright (c) 2012-2016, Texas Instruments Incorporated
 All rights reserved.

 IMPORTANT: Your use of this Software is limited to those specific rights
 granted under the terms of a software license agreement between the user
 who downloaded the software, his/her employer (which must be your employer)
 and Texas Instruments Incorporated (the "License"). You may not use this
 Software unless you agree to abide by the terms of the License. The License
 limits your use, and you acknowledge, that the Software may not be modified,
 copied or distributed unless embedded on a Texas Instruments microcontroller
 or used solely and exclusively in conjunction with a Texas Instruments radio
 frequency transceiver, which is integrated into your product. Other than for
 the foregoing purpose, you may not use, reproduce, copy, prepare derivative
 works of, modify, distribute, perform, display or sell this Software and/or
 its documentation for any purpose.

 YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
 PROVIDED �AS IS� WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
 NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
 TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
 NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
 LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
 INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
 OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
 OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
 (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

 Should you have any questions regarding your right to use this Software,
 contact Texas Instruments Incorporated at www.TI.com.

 ******************************************************************************
 Release Name: ble_sdk_1.4.2.2
 Release Date: 2016-06-09 06:57:10
 *****************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

#include "bcomdef.h"
#include "linkdb.h"

// HAL includes
#include "hal_types.h"
#include "hal_board.h"
#include "hal_drivers.h"
#include "hal_key.h"
#include "hal_led.h"

// HID includes
#include "usb_framework.h"
#include "usb_hid.h"
#include "usb_hid_reports.h"
#include "usb_suspend.h"

// OSAL includes
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "osal_bufmgr.h"
#include "OSAL_Tasks.h"
#include "osal_snv.h"

#include "OnBoard.h"

#if defined ( OSAL_CBTIMER_NUM_TASKS )
  #include "osal_cbtimer.h"
#endif

/* GATT */
#include "gatt.h"
#include "gatt_uuid.h"
#include "gatt_profile_uuid.h"

/* HCI */
#include "hci.h"

#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"
#include "simpleGATTprofile.h"

#include "peripheral.h"

#include "gapbondmgr.h"

/* Application */
#include "hidapp.h"

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */

// How often to perform periodic event
#define HID_PERIODIC_EVT_PERIOD                   5000

// What is the advertising interval when device is discoverable (units of 625us, 160=100ms)
#define DEFAULT_ADVERTISING_INTERVAL          160

// Limited discoverable mode advertises for 30.72s, and then stops
// General discoverable mode advertises indefinitely

#if defined ( CC2540_MINIDK )
#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_LIMITED
#else
#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_GENERAL
#endif  // defined ( CC2540_MINIDK )

// Minimum connection interval (units of 1.25ms, 80=100ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     80

// Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     800

// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY         0

// Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT          1000

// Whether to enable automatic parameter update request when a connection is formed
#define DEFAULT_ENABLE_UPDATE_REQUEST         TRUE

// Connection Pause Peripheral time value (in seconds)
#define DEFAULT_CONN_PAUSE_PERIPHERAL         6

// Company Identifier: Texas Instruments Inc. (13)
#define TI_COMPANY_ID                         0x000D

#define INVALID_CONNHANDLE                    0xFFFF

// Length of bd addr as a string
#define B_ADDR_STR_LEN                        15

// output report buffer size
#define HIDAPP_OUTBUF_SIZE                    3

// Vendor specific report identifiers
#define HID_CMD_REPORT_ID                     1
#define HID_PAIR_ENTRY_REPORT_ID              2

// output report ID
#define HIDAPP_OUTPUT_REPORT_ID               HID_CMD_REPORT_ID

// Count of polling for INPUT packet ready
#define USB_HID_INPUT_RETRY_COUNT             3

#define HIDAPP_INPUT_RETRY_TIMEOUT            5 // ms

/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */


/* ------------------------------------------------------------------------------------------------
 *                                            Macros
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                           Local Functions
 * ------------------------------------------------------------------------------------------------
 */

//static void hidappOutputReport( void );
static uint8 hidappSendInReport( attHandleValueNoti_t *pNoti );

static void hidappProcessGATTMsg( gattMsgEvent_t *pMsg );
static void hidappHandleKeys( uint8 keys, uint8 state );

static void peripheralStateNotificationCB( gaprole_States_t newState );
static void performPeriodicTask( void );
static void simpleProfileChangeCB( uint8 paramID );



/* ------------------------------------------------------------------------------------------------
 *                                           Local Variables
 * ------------------------------------------------------------------------------------------------
 */

//// HID output report buffer
//static uint8 hidappOutBuf[HIDAPP_OUTBUF_SIZE];

// HID input report used for retries
static attHandleValueNoti_t lastInReport = { 0 };
static uint8 reportRetries = 0;

// OSAL task ID assigned to the application task
static uint8 hidappTaskId;

static gaprole_States_t gapProfileState = GAPROLE_INIT;

// GAP - SCAN RSP data (max size = 31 bytes)
static uint8 scanRspData[] =
{
  // complete name
  0x14,   // length of this data
  GAP_ADTYPE_LOCAL_NAME_COMPLETE,
  0x53,   // 'S'
  0x69,   // 'i'
  0x6d,   // 'm'
  0x70,   // 'p'
  0x6c,   // 'l'
  0x65,   // 'e'
  0x42,   // 'B'
  0x4c,   // 'L'
  0x45,   // 'E'
  0x50,   // 'P'
  0x65,   // 'e'
  0x72,   // 'r'
  0x69,   // 'i'
  0x70,   // 'p'
  0x68,   // 'h'
  0x65,   // 'e'
  0x72,   // 'r'
  0x61,   // 'a'
  0x6c,   // 'l'

  // connection interval range
  0x05,   // length of this data
  GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
  LO_UINT16( DEFAULT_DESIRED_MIN_CONN_INTERVAL ),   // 100ms
  HI_UINT16( DEFAULT_DESIRED_MIN_CONN_INTERVAL ),
  LO_UINT16( DEFAULT_DESIRED_MAX_CONN_INTERVAL ),   // 1s
  HI_UINT16( DEFAULT_DESIRED_MAX_CONN_INTERVAL ),

  // Tx power level
  0x02,   // length of this data
  GAP_ADTYPE_POWER_LEVEL,
  0       // 0dBm
};

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
static uint8 advertData[] =
{
  // Flags; this sets the device to use limited discoverable
  // mode (advertises for 30 seconds at a time) instead of general
  // discoverable mode (advertises indefinitely)
  0x02,   // length of this data
  GAP_ADTYPE_FLAGS,
  DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

  // service UUID, to notify central devices what services are included
  // in this peripheral
  0x03,   // length of this data
  GAP_ADTYPE_16BIT_MORE,      // some of the UUID's, but not all
  LO_UINT16( SIMPLEPROFILE_SERV_UUID ),
  HI_UINT16( SIMPLEPROFILE_SERV_UUID ),

};

// GAP GATT Attributes
static uint8 attDeviceName[GAP_DEVICE_NAME_LEN] = "Simple BLE Peripheral";


// GAP Role Callbacks
static gapRolesCBs_t hidapp_PeripheralCBs =
{
  peripheralStateNotificationCB,  // Profile State Change Callbacks
  NULL                            // When a valid RSSI is read from controller (not used by application)
};

// GAP Bond Manager Callbacks
static gapBondCBs_t hidapp_BondMgrCBs =
{
  NULL,                     // Passcode callback (not used by application)
  NULL                      // Pairing / Bonding state Callback (not used by application)
};

// Simple GATT Profile Callbacks
static simpleProfileCBs_t hidapp_SimpleProfileCBs =
{
  simpleProfileChangeCB    // Charactersitic value change callback
};




/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */

/*******************************************************************************
 *
 * @fn      hidappInit
 *
 * @brief   This is the Sample Application task initialization called by OSAL.
 *
 * @param   taskId - task ID assigned after it was added in the OSAL task queue
 *
 * @return  none
 */
void Hidapp_Init( uint8 taskId )
{
  // save task ID assigned by OSAL
  hidappTaskId = taskId;

  // Setup the GAP
  VOID GAP_SetParamValue( TGAP_CONN_PAUSE_PERIPHERAL, DEFAULT_CONN_PAUSE_PERIPHERAL );
  
  // Setup the GAP Peripheral Role Profile
  {
    // For other hardware platforms, device starts advertising upon initialization
    uint8 initial_advertising_enable = TRUE;

    // By setting this to zero, the device will go into the waiting state after
    // being discoverable for 30.72 second, and will not being advertising again
    // until the enabler is set back to TRUE
    uint16 gapRole_AdvertOffTime = 0;

    uint8 enable_update_request = DEFAULT_ENABLE_UPDATE_REQUEST;
    uint16 desired_min_interval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
    uint16 desired_max_interval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
    uint16 desired_slave_latency = DEFAULT_DESIRED_SLAVE_LATENCY;
    uint16 desired_conn_timeout = DEFAULT_DESIRED_CONN_TIMEOUT;

    // Set the GAP Role Parameters
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &initial_advertising_enable );
    GAPRole_SetParameter( GAPROLE_ADVERT_OFF_TIME, sizeof( uint16 ), &gapRole_AdvertOffTime );

    GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, sizeof ( scanRspData ), scanRspData );
    GAPRole_SetParameter( GAPROLE_ADVERT_DATA, sizeof( advertData ), advertData );

    GAPRole_SetParameter( GAPROLE_PARAM_UPDATE_ENABLE, sizeof( uint8 ), &enable_update_request );
    GAPRole_SetParameter( GAPROLE_MIN_CONN_INTERVAL, sizeof( uint16 ), &desired_min_interval );
    GAPRole_SetParameter( GAPROLE_MAX_CONN_INTERVAL, sizeof( uint16 ), &desired_max_interval );
    GAPRole_SetParameter( GAPROLE_SLAVE_LATENCY, sizeof( uint16 ), &desired_slave_latency );
    GAPRole_SetParameter( GAPROLE_TIMEOUT_MULTIPLIER, sizeof( uint16 ), &desired_conn_timeout );
  }

  // Set the GAP Characteristics
  GGS_SetParameter( GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName );

  // Set advertising interval
  {
    uint16 advInt = DEFAULT_ADVERTISING_INTERVAL;

    GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MIN, advInt );
    GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MAX, advInt );
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, advInt );
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, advInt );
  }

  // Setup the GAP Bond Manager
  {
    uint32 passkey = 0; // passkey "000000"
    uint8 pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
    uint8 mitm = TRUE;
    uint8 ioCap = GAPBOND_IO_CAP_NO_INPUT_NO_OUTPUT;
    uint8 bonding = TRUE;
    GAPBondMgr_SetParameter( GAPBOND_DEFAULT_PASSCODE, sizeof ( uint32 ), &passkey );
    GAPBondMgr_SetParameter( GAPBOND_PAIRING_MODE, sizeof ( uint8 ), &pairMode );
    GAPBondMgr_SetParameter( GAPBOND_MITM_PROTECTION, sizeof ( uint8 ), &mitm );
    GAPBondMgr_SetParameter( GAPBOND_IO_CAPABILITIES, sizeof ( uint8 ), &ioCap );
    GAPBondMgr_SetParameter( GAPBOND_BONDING_ENABLED, sizeof ( uint8 ), &bonding );
  }

  // Initialize GATT attributes
  GGS_AddService( GATT_ALL_SERVICES );            // GAP
  GATTServApp_AddService( GATT_ALL_SERVICES );    // GATT attributes
  DevInfo_AddService();                           // Device Information Service
  SimpleProfile_AddService( GATT_ALL_SERVICES );  // Simple GATT Profile

  // Setup the SimpleProfile Characteristic Values
  {
    uint8 charValue1 = 1;
    uint8 charValue2 = 2;
    uint8 charValue3 = 3;
    uint8 charValue4 = 4;
    uint8 charValue5[SIMPLEPROFILE_CHAR5_LEN] = { 1, 2, 3, 4, 5 };
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR1, sizeof ( uint8 ), &charValue1 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR2, sizeof ( uint8 ), &charValue2 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR3, sizeof ( uint8 ), &charValue3 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR4, sizeof ( uint8 ), &charValue4 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR5, SIMPLEPROFILE_CHAR5_LEN, charValue5 );
  }

  // Register callback with SimpleGATTprofile
  VOID SimpleProfile_RegisterAppCBs( &hidapp_SimpleProfileCBs );

  // Enable clock divide on halt
  // This reduces active current while radio is active and CC254x MCU
  // is halted
  HCI_EXT_ClkDivOnHaltCmd( HCI_EXT_ENABLE_CLK_DIVIDE_ON_HALT );

  // Setup a delayed profile startup
  osal_set_event( hidappTaskId, HIDAPP_START_DEVICE_EVT );
}

/*********************************************************************
 * @fn      Hidapp_ProcessEvent
 *
 * @brief   This function processes the OSAL events and messages for the
 *          Sample Application.
 *
 * input parameters
 *
 * @param   taskId - Task ID assigned to this app by OSAL at system initialization.
 * @param   events - Bit mask of the pending event(s).
 *
 * output parameters
 *
 * None.
 *
 * @return  The events bit map received via parameter with the bits cleared
 *          which correspond to the event(s) that were processed on this invocation.
 */
uint16 Hidapp_ProcessEvent(uint8 taskId, uint16 events)
{
  (void) taskId; // Unused argument

  if ( events & SYS_EVENT_MSG )
  {
    osal_event_hdr_t  *pMsg;

    while ((pMsg = (osal_event_hdr_t *) osal_msg_receive(hidappTaskId)) != NULL)
    {
      switch (pMsg->event)
      {
        case GATT_MSG_EVENT:
          {
            hidappProcessGATTMsg( (gattMsgEvent_t *)pMsg );
          }
          break;
        case KEY_CHANGE:
          {
            hidappHandleKeys( ((keyChange_t *)pMsg)->keys, ((keyChange_t *)pMsg)->state );
          }
          break;

        default:
          break;
      }

      VOID osal_msg_deallocate((uint8 *) pMsg);
    }

    return (events ^ SYS_EVENT_MSG);
  }

  if ( events & HIDAPP_START_DEVICE_EVT )
  {
    // Start the Device
    VOID GAPRole_StartDevice( &hidapp_PeripheralCBs );

    // Start Bond Manager
    VOID GAPBondMgr_Register( &hidapp_BondMgrCBs );

    // Set timer for first periodic event
    osal_start_timerEx( hidappTaskId, HIDAPP_PERIODIC_EVT, HID_PERIODIC_EVT_PERIOD );

    return (events ^ HIDAPP_START_DEVICE_EVT);
  }

  if ( events & HIDAPP_EVT_REPORT_RETRY )
  {
    // Report retries event
    if ( ( hidappSendInReport( &lastInReport ) == FALSE ) &&
         ( reportRetries < USB_HID_INPUT_RETRY_COUNT ) )
    {
      reportRetries++;
      osal_start_timerEx( hidappTaskId, HIDAPP_EVT_REPORT_RETRY,
                          (reportRetries*HIDAPP_INPUT_RETRY_TIMEOUT) );
    }
    else
    {
      // Done retrying      
      reportRetries = 0;
      
      // Free last report's payload
      GATT_bm_free( (gattMsg_t *)&lastInReport, ATT_HANDLE_VALUE_NOTI );
      
      // Zero out all fields of last report
      VOID osal_memset( &lastInReport, 0, sizeof( attHandleValueNoti_t ) );
    }

    return (events ^ HIDAPP_EVT_REPORT_RETRY);
  }

  if ( events & HIDAPP_PERIODIC_EVT )
  {
    // Restart timer
    if ( HID_PERIODIC_EVT_PERIOD )
    {
      osal_start_timerEx( hidappTaskId, HIDAPP_PERIODIC_EVT, HID_PERIODIC_EVT_PERIOD );
    }

    // Perform periodic application task
    performPeriodicTask();

    return (events ^ HIDAPP_PERIODIC_EVT);
  }
  
  return ( 0 );  /* Discard unknown events. */
}

/*********************************************************************
 *
 * @fn      hidappHandleKey
 *
 * @brief   Handle service for keys
 *
 * @param   keys  - key that was pressed (i.e. the scanned row/col index)
 *          state - shifted
 *
 * @return  void
 */
static void hidappHandleKeys( uint8 keys, uint8 state )
{
  // Unused arguments
  (void) state;

  if (keys & HAL_KEY_SW_1)
  {

  }

  if (keys & HAL_KEY_SW_2)
  {
   
  }
}

///*********************************************************************
// *
// * @fn      hidappOutputReport
// *
// * @brief   Handles output report from host
// *
// * @param   None
// *
// * @return  None
// */
//static void hidappOutputReport( void )
//{
//  // Decode the output report
//  if (HIDAPP_OUTPUT_REPORT_ID == hidappOutBuf[0])
//  {
//    // correct report ID
//    uint8 cmdId = hidappOutBuf[1];
//    switch (cmdId)
//    {
//      // Process output buffer here
//    }
//  }
//}

/*********************************************************************
 *
 * @fn      hidappSuspendEnter
 *
 * @brief   Hook function to be called upon entry into USB suspend mode
 *
 * @param   none
 *
 * @return  none
 */
void hidappSuspendEnter( void )
{
  // not supported
}

/*********************************************************************
 *
 * @fn      hidappSuspendExit
 *
 * @brief   Hook function to be called upon exit from USB suspend mode
 *
 * @param   none
 *
 * @return  none
 */
void hidappSuspendExit( void )
{
  // not supported
}

/*********************************************************************
 * @fn      hidappProcessGATTMsg
 *
 * @brief   Process incoming GATT messages.
 *
 * @param   pPkt - pointer to message.
 *
 * @return  none
 */

static void hidappProcessGATTMsg( gattMsgEvent_t *pPkt )
{
  // Build the message first
  switch ( pPkt->method )
  {
    case ATT_HANDLE_VALUE_NOTI://!!!!!!!!
      // First try to send out pending HID report
      if ( reportRetries > 0 )
      {
        hidappSendInReport( &lastInReport );
        
        // Free last report's payload
        GATT_bm_free( (gattMsg_t *)&lastInReport, ATT_HANDLE_VALUE_NOTI );
        
        // Zero out all fields of last report
        VOID osal_memset( &lastInReport, 0, sizeof( attHandleValueNoti_t ) );
      
        reportRetries = 0;
        osal_stop_timerEx( hidappTaskId, HIDAPP_EVT_REPORT_RETRY );
      }

      // Send incoming HID report
      if ( hidappSendInReport( &(pPkt->msg.handleValueNoti) ) == FALSE )
      {
        // Save report for retries later
        osal_memcpy( &lastInReport, &(pPkt->msg.handleValueNoti), sizeof( attHandleValueNoti_t ) );

        // Report's payload is part of lastInReport now so no need to free it yet
        pPkt->msg.handleValueNoti.pValue = NULL;
        
        reportRetries = 1;
        osal_start_timerEx( hidappTaskId, HIDAPP_EVT_REPORT_RETRY, HIDAPP_INPUT_RETRY_TIMEOUT );
      }
      break;

    default:
      // Unknown event
      break;
  }
  
  GATT_bm_free( &pPkt->msg, pPkt->method );
}

/*********************************************************************
 * @fn      hidappSendInReport
 *
 * @brief   Send out an incoming HID report (GATT indication).
 *
 * @param   pNoti - report to be sent
 *
 * @return  TRUE if report was sent; FALSE otherwise.
 */
static uint8 hidappSendInReport( attHandleValueNoti_t *pNoti )
{
  /*uint8 endPoint;

  if( pNoti->handle == keyCharHandle )
  {
    // Keyboard report
    endPoint = USB_HID_KBD_EP;
  }
  else if (pNoti->handle == mouseCharHandle )
  {
    // Mouse report
    endPoint = USB_HID_MOUSE_EP;
  }
  else if ( pNoti->handle == consumerCtrlCharHandle )
  {
    // Consumer Control report
    endPoint = USB_HID_CC_EP;
  }
  else
  {
    // Maybe we're still in discovery phase
    return ( FALSE );
  }

  HalLedSet( HAL_LED_2, HAL_LED_MODE_BLINK );

  return ( hidSendHidInReport(pNoti->pValue, endPoint, pNoti->len) );*/
  return 1;//!!!!!!!!
}



/*********************************************************************
 * @fn      peripheralStateNotificationCB
 *
 * @brief   Notification from the profile of a state change.
 *
 * @param   newState - new state
 *
 * @return  none
 */
static void peripheralStateNotificationCB( gaprole_States_t newState )
{
  switch ( newState )
  {
    case GAPROLE_STARTED:
      {
        uint8 ownAddress[B_ADDR_LEN];
        uint8 systemId[DEVINFO_SYSTEM_ID_LEN];

        GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddress);

        // use 6 bytes of device address for 8 bytes of system ID value
        systemId[0] = ownAddress[0];
        systemId[1] = ownAddress[1];
        systemId[2] = ownAddress[2];

        // set middle bytes to zero
        systemId[4] = 0x00;
        systemId[3] = 0x00;

        // shift three bytes up
        systemId[7] = ownAddress[5];
        systemId[6] = ownAddress[4];
        systemId[5] = ownAddress[3];

        DevInfo_SetParameter(DEVINFO_SYSTEM_ID, DEVINFO_SYSTEM_ID_LEN, systemId);

      }
      break;

    case GAPROLE_ADVERTISING:
      {
      }
      break;

      
      
    case GAPROLE_CONNECTED:
      {        

      }
      break;

    case GAPROLE_CONNECTED_ADV:
      {

      }
      break;      
    case GAPROLE_WAITING:
      {

      }
      break;

    case GAPROLE_WAITING_AFTER_TIMEOUT:
      {

      }
      break;

    case GAPROLE_ERROR:
      {

      }
      break;

    default:
      {

      }
      break;

  }

  gapProfileState = newState;

#if !defined( CC2540_MINIDK )
  VOID gapProfileState;     // added to prevent compiler warning with
                            // "CC2540 Slave" configurations
#endif

}

/*********************************************************************
 * @fn      performPeriodicTask
 *
 * @brief   Perform a periodic application task. This function gets
 *          called every five seconds as a result of the SBP_PERIODIC_EVT
 *          OSAL event. In this example, the value of the third
 *          characteristic in the SimpleGATTProfile service is retrieved
 *          from the profile, and then copied into the value of the
 *          the fourth characteristic.
 *
 * @param   none
 *
 * @return  none
 */
static void performPeriodicTask( void )
{
  uint8 valueToCopy;
  uint8 stat;

  // Call to retrieve the value of the third characteristic in the profile
  stat = SimpleProfile_GetParameter( SIMPLEPROFILE_CHAR3, &valueToCopy);

  if( stat == SUCCESS )
  {
    /*
     * Call to set that value of the fourth characteristic in the profile. Note
     * that if notifications of the fourth characteristic have been enabled by
     * a GATT client device, then a notification will be sent every time this
     * function is called.
     */
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR4, sizeof(uint8), &valueToCopy);
  }
}

/*********************************************************************
 * @fn      simpleProfileChangeCB
 *
 * @brief   Callback from SimpleBLEProfile indicating a value change
 *
 * @param   paramID - parameter ID of the value that was changed.
 *
 * @return  none
 */
static void simpleProfileChangeCB( uint8 paramID )
{
  uint8 newValue;

  switch( paramID )
  {
    case SIMPLEPROFILE_CHAR1:
      SimpleProfile_GetParameter( SIMPLEPROFILE_CHAR1, &newValue );

      #if (defined HAL_LCD) && (HAL_LCD == TRUE)
        HalLcdWriteStringValue( "Char 1:", (uint16)(newValue), 10,  HAL_LCD_LINE_3 );
      #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)

      break;

    case SIMPLEPROFILE_CHAR3:
      SimpleProfile_GetParameter( SIMPLEPROFILE_CHAR3, &newValue );

      #if (defined HAL_LCD) && (HAL_LCD == TRUE)
        HalLcdWriteStringValue( "Char 3:", (uint16)(newValue), 10,  HAL_LCD_LINE_3 );
      #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)

      break;

    default:
      // should not reach here!
      break;
  }
}


/**************************************************************************************************
**************************************************************************************************/
