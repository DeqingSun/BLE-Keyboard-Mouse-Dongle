/******************************************************************************

 @file  simpleGATTprofile.c

 @brief This file contains the Simple GATT profile sample GATT service profile
        for use with the BLE sample application.

 Group: WCS, BTS
 Target Device: CC2540, CC2541

 ******************************************************************************
 
 Copyright (c) 2010-2016, Texas Instruments Incorporated
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
 PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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

/*********************************************************************
 * INCLUDES
 */
#include <string.h>

#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "att.h"
#include "gatt.h"
#include "gatt_uuid.h"
#include "gattservapp.h"
#include "gapbondmgr.h"

#include "simpleGATTprofile.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

#define SERVAPP_NUM_ATTR_SUPPORTED        17

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
// Keyboard Dongle Service UUID: 0x0000
CONST uint8 keyboardDongleServUUID[ATT_UUID_SIZE] =
{ 
  KBD_DONGLE_SERVICE_BASE_UUID_128(KBD_DONGLE_SERV_UUID),
};

// Keyboard Press Characteristic UUID: 0x0001
CONST uint8 keyboardPressCharUUID[ATT_UUID_SIZE] =
{ 
  KBD_DONGLE_SERVICE_BASE_UUID_128(KEYBOARD_PRESS_CHAR_UUID),
};

// Keyboard Type Characteristic UUID: 0x0002             
CONST uint8 keyboardTypeCharUUID[ATT_UUID_SIZE] =
{ 
  KBD_DONGLE_SERVICE_BASE_UUID_128(KEYBOARD_TYPE_CHAR_UUID),
};

// Keyboard Report Characteristic UUID: 0x0003
CONST uint8 keyboardReportCharUUID[ATT_UUID_SIZE] =
{ 
  KBD_DONGLE_SERVICE_BASE_UUID_128(KEYBOARD_REPORT_CHAR_UUID),
};

// Keyboard LED Characteristic UUID: 0x0004
CONST uint8 keyboardLEDCharUUID[ATT_UUID_SIZE] =
{ 
  KBD_DONGLE_SERVICE_BASE_UUID_128(KEYBOARD_LED_CHAR_UUID),
};

// Mouse Move Characteristic UUID: 0x0005
CONST uint8 mouseMoveCharUUID[ATT_UUID_SIZE] =
{ 
  KBD_DONGLE_SERVICE_BASE_UUID_128(MOUSE_MOVE_CHAR_UUID),
};

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

static keyboardDongleProfileCBs_t *simpleProfile_AppCBs = NULL;

/*********************************************************************
 * Profile Attributes - variables
 */

// Keyboard Dongle Service attribute
static CONST gattAttrType_t keyboardDongleService = { ATT_UUID_SIZE, keyboardDongleServUUID };


// Keyboard Press Characteristic Properties
static uint8 keyboardPressCharProps = GATT_PROP_WRITE;

// Keyboard Press Characteristic Value
static uint8 keyboardPressChar = 0;

// Keyboard Press Characteristic User Description
static uint8 keyboardPressCharUserDesp[13] = "KBD type chr";


// Keyboard Type Characteristic Properties
static uint8 keyboardTypeCharProps = GATT_PROP_WRITE;

// Keyboard Type Characteristic Value
static uint8 keyboardTypeChar[9] = {0};

// Keyboard Type Characteristic User Description
static uint8 keyboardTypeCharUserDesp[16] = "KBD type str 16";


// Simple Profile Characteristic 3 Properties
static uint8 simpleProfileChar3Props = GATT_PROP_WRITE;

// Characteristic 3 Value
static uint8 simpleProfileChar3 = 0;

// Simple Profile Characteristic 3 User Description
static uint8 simpleProfileChar3UserDesp[17] = "Characteristic 3";


// Simple Profile Characteristic 4 Properties
static uint8 simpleProfileChar4Props = GATT_PROP_NOTIFY;

// Characteristic 4 Value
static uint8 simpleProfileChar4 = 0;

// Simple Profile Characteristic 4 Configuration Each client has its own
// instantiation of the Client Characteristic Configuration. Reads of the
// Client Characteristic Configuration only shows the configuration for
// that client and writes only affect the configuration of that client.
static gattCharCfg_t *simpleProfileChar4Config;
                                        
// Simple Profile Characteristic 4 User Description
static uint8 simpleProfileChar4UserDesp[17] = "Characteristic 4";


// Simple Profile Characteristic 5 Properties
static uint8 simpleProfileChar5Props = GATT_PROP_READ;

// Characteristic 5 Value
static uint8 simpleProfileChar5[MOUSE_MOVE_CHAR_LEN] = { 0, 0, 0, 0, 0 };

// Simple Profile Characteristic 5 User Description
static uint8 simpleProfileChar5UserDesp[17] = "Characteristic 5";

/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t simpleProfileAttrTbl[SERVAPP_NUM_ATTR_SUPPORTED] = 
{
  // Keyboard Dongle Profile Service
  { 
    { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
    GATT_PERMIT_READ,                         /* permissions */
    0,                                        /* handle */
    (uint8 *)&keyboardDongleService           /* pValue */
  },

    // keyboard Press Characteristic Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &keyboardPressCharProps 
    },

      // keyboard Press Characteristic Value
      { 
        { ATT_UUID_SIZE, keyboardPressCharUUID },
        GATT_PERMIT_WRITE, 
        0, 
        &keyboardPressChar 
      },

      // keyboard Press Characteristic User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        keyboardPressCharUserDesp 
      },      

    // keyboard Type Characteristic Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &keyboardTypeCharProps 
    },

      // keyboard Type Characteristic Value
      { 
        { ATT_UUID_SIZE, keyboardTypeCharUUID },
        GATT_PERMIT_WRITE, 
        0, 
        keyboardTypeChar 
      },

      // keyboard Type Characteristic User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        keyboardTypeCharUserDesp 
      },           
      
    // Characteristic 3 Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &simpleProfileChar3Props 
    },

      // Characteristic Value 3
      { 
        { ATT_UUID_SIZE, keyboardReportCharUUID },
        GATT_PERMIT_WRITE, 
        0, 
        &simpleProfileChar3 
      },

      // Characteristic 3 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        simpleProfileChar3UserDesp 
      },

    // Characteristic 4 Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &simpleProfileChar4Props 
    },

      // Characteristic Value 4
      { 
        { ATT_UUID_SIZE, keyboardLEDCharUUID },
        0, 
        0, 
        &simpleProfileChar4 
      },

      // Characteristic 4 configuration
      { 
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE, 
        0, 
        (uint8 *)&simpleProfileChar4Config 
      },
      
      // Characteristic 4 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        simpleProfileChar4UserDesp 
      },
      
    // Characteristic 5 Declaration
    { 
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ, 
      0,
      &simpleProfileChar5Props 
    },

      // Characteristic Value 5
      { 
        { ATT_UUID_SIZE, mouseMoveCharUUID },
        GATT_PERMIT_AUTHEN_READ, 
        0, 
        simpleProfileChar5 
      },

      // Characteristic 5 User Description
      { 
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ, 
        0, 
        simpleProfileChar5UserDesp 
      },
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t simpleProfile_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
                                           uint8 *pValue, uint8 *pLen, uint16 offset,
                                           uint8 maxLen, uint8 method );
static bStatus_t simpleProfile_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                            uint8 *pValue, uint8 len, uint16 offset,
                                            uint8 method );

/*********************************************************************
 * PROFILE CALLBACKS
 */
// Simple Profile Service Callbacks
CONST gattServiceCBs_t simpleProfileCBs =
{
  simpleProfile_ReadAttrCB,  // Read callback function pointer
  simpleProfile_WriteAttrCB, // Write callback function pointer
  NULL                       // Authorization callback function pointer
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

bStatus_t utilExtractUuid16(gattAttribute_t *pAttr, uint16 *pUuid)
{
  bStatus_t status = SUCCESS;

  if (pAttr->type.len == ATT_BT_UUID_SIZE )
  {
    // 16-bit UUID direct
    *pUuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
  }
  else if (pAttr->type.len == ATT_UUID_SIZE)
  {
    // 16-bit UUID extracted bytes 12 and 13 for this service
    *pUuid = BUILD_UINT16( pAttr->type.uuid[12], pAttr->type.uuid[13]);
  } else {
    *pUuid = 0xFFFF;
    status = FAILURE;
  }

  return status;
}

/*********************************************************************
 * @fn      SimpleProfile_AddService
 *
 * @brief   Initializes the Simple Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 *
 * @return  Success or Failure
 */
bStatus_t SimpleProfile_AddService( uint32 services )
{
  uint8 status;
  
  // Allocate Client Characteristic Configuration table
  simpleProfileChar4Config = (gattCharCfg_t *)osal_mem_alloc( sizeof(gattCharCfg_t) *
                                                              linkDBNumConns );
  if ( simpleProfileChar4Config == NULL )
  {     
    return ( bleMemAllocError );
  }
  
  // Initialize Client Characteristic Configuration attributes
  GATTServApp_InitCharCfg( INVALID_CONNHANDLE, simpleProfileChar4Config );
  
  if ( services & KBD_DONGLE_SERVICE )
  {
    // Register GATT attribute list and CBs with GATT Server App
    status = GATTServApp_RegisterService( simpleProfileAttrTbl, 
                                          GATT_NUM_ATTRS( simpleProfileAttrTbl ),
                                          GATT_MAX_ENCRYPT_KEY_SIZE,
                                          &simpleProfileCBs );
  }
  else
  {
    status = SUCCESS;
  }
  
  return ( status );
}

/*********************************************************************
 * @fn      SimpleProfile_RegisterAppCBs
 *
 * @brief   Registers the application callback function. Only call 
 *          this function once.
 *
 * @param   callbacks - pointer to application callbacks.
 *
 * @return  SUCCESS or bleAlreadyInRequestedMode
 */
bStatus_t SimpleProfile_RegisterAppCBs( keyboardDongleProfileCBs_t *appCallbacks )
{
  if ( appCallbacks )
  {
    simpleProfile_AppCBs = appCallbacks;
    
    return ( SUCCESS );
  }
  else
  {
    return ( bleAlreadyInRequestedMode );
  }
}

/*********************************************************************
 * @fn      SimpleProfile_SetParameter
 *
 * @brief   Set a Simple Profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   len - length of data to write
 * @param   value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t SimpleProfile_SetParameter( uint8 param, uint8 len, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case KEYBOARD_PRESS_CHAR:
      if ( len == sizeof ( uint8 ) ) 
      {
        keyboardPressChar = *((uint8*)value);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case KEYBOARD_TYPE_CHAR:
      if ( len <= (sizeof(keyboardTypeChar)) ) 
      {
        (void)memcpy(keyboardTypeChar, value, len);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case KEYBOARD_REPORT_CHAR:
      if ( len == sizeof ( uint8 ) ) 
      {
        simpleProfileChar3 = *((uint8*)value);
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case KEYBOARD_LED_CHAR:
      if ( len == sizeof ( uint8 ) ) 
      {
        simpleProfileChar4 = *((uint8*)value);
        
        // See if Notification has been enabled
        GATTServApp_ProcessCharCfg( simpleProfileChar4Config, &simpleProfileChar4, FALSE,
                                    simpleProfileAttrTbl, GATT_NUM_ATTRS( simpleProfileAttrTbl ),
                                    INVALID_TASK_ID, simpleProfile_ReadAttrCB );
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;

    case MOUSE_MOVE_CHAR:
      if ( len == MOUSE_MOVE_CHAR_LEN ) 
      {
        VOID memcpy( simpleProfileChar5, value, MOUSE_MOVE_CHAR_LEN );
      }
      else
      {
        ret = bleInvalidRange;
      }
      break;
      
    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
}

/*********************************************************************
 * @fn      SimpleProfile_GetParameter
 *
 * @brief   Get a Simple Profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   value - pointer to data to put.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t SimpleProfile_GetParameter( uint8 param, void *value )
{
  bStatus_t ret = SUCCESS;
  switch ( param )
  {
    case KEYBOARD_PRESS_CHAR:
      *((uint8*)value) = keyboardPressChar;
      break;

    case KEYBOARD_TYPE_CHAR:
      {
        int stringLen = strlen((const char *)keyboardTypeChar);
        if (stringLen>(sizeof(keyboardTypeChar))) stringLen=(sizeof(keyboardTypeChar));
        (void)memcpy(value, keyboardTypeChar, stringLen);
      }
      break;      

    case KEYBOARD_REPORT_CHAR:
      *((uint8*)value) = simpleProfileChar3;
      break;  

    case KEYBOARD_LED_CHAR:
      *((uint8*)value) = simpleProfileChar4;
      break;

    case MOUSE_MOVE_CHAR:
      VOID memcpy( value, simpleProfileChar5, MOUSE_MOVE_CHAR_LEN );
      break;      
      
    default:
      ret = INVALIDPARAMETER;
      break;
  }
  
  return ( ret );
}

/*********************************************************************
 * @fn          simpleProfile_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 * @param       method - type of read message
 *
 * @return      SUCCESS, blePending or Failure
 */
static bStatus_t simpleProfile_ReadAttrCB( uint16 connHandle, gattAttribute_t *pAttr, 
                                           uint8 *pValue, uint8 *pLen, uint16 offset,
                                           uint8 maxLen, uint8 method )
{
  bStatus_t status = SUCCESS;

  // If attribute permissions require authorization to read, return error
  if ( gattPermitAuthorRead( pAttr->permissions ) )
  {
    // Insufficient authorization
    return ( ATT_ERR_INSUFFICIENT_AUTHOR );
  }
  
  // Make sure it's not a blob operation (no attributes in the profile are long)
  if ( offset > 0 )
  {
    return ( ATT_ERR_ATTR_NOT_LONG );
  }
 
  if ( pAttr->type.len == ATT_BT_UUID_SIZE )
  {
    // 16-bit UUID
    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
    switch ( uuid )
    {
      // No need for "GATT_SERVICE_UUID" or "GATT_CLIENT_CHAR_CFG_UUID" cases;
      // gattserverapp handles those reads

      // characteristics 1 and 2 have read permissions
      // characteritisc 3 does not have read permissions; therefore it is not
      //   included here
      // characteristic 4 does not have read permissions, but because it
      //   can be sent as a notification, it is included here


      case KEYBOARD_LED_CHAR_UUID:
        *pLen = 1;
        pValue[0] = *pAttr->pValue;
        break;

      case MOUSE_MOVE_CHAR_UUID:
        *pLen = MOUSE_MOVE_CHAR_LEN;
        VOID memcpy( pValue, pAttr->pValue, MOUSE_MOVE_CHAR_LEN );
        break;
        
      default:
        // Should never get here! (characteristics 3 and 4 do not have read permissions)
        *pLen = 0;
        status = ATT_ERR_ATTR_NOT_FOUND;
        break;
    }
  }
  else
  {
    // 128-bit UUID
    *pLen = 0;
    status = ATT_ERR_INVALID_HANDLE;
  }

  return ( status );
}

/*********************************************************************
 * @fn      simpleProfile_WriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 * @param   method - type of write message
 *
 * @return  SUCCESS, blePending or Failure
 */
static bStatus_t simpleProfile_WriteAttrCB( uint16 connHandle, gattAttribute_t *pAttr,
                                            uint8 *pValue, uint8 len, uint16 offset,
                                            uint8 method )
{
  uint16 uuid;
  bStatus_t status = SUCCESS;
  uint8 notifyApp = 0xFF;
  
  // If attribute permissions require authorization to write, return error
  if ( gattPermitAuthorWrite( pAttr->permissions ) )
  {
    // Insufficient authorization
    return ( ATT_ERR_INSUFFICIENT_AUTHOR );
  }
  
  // Make sure it's not a blob operation (no attributes in the profile are long)
  if ( offset > 0 ){
    return ( ATT_ERR_ATTR_NOT_LONG );
  }
  
  if (utilExtractUuid16(pAttr,&uuid) == FAILURE) {                                      
    // Invalid handle
    return ATT_ERR_INVALID_HANDLE;                                                      
  }
  
  switch ( uuid ){
    case KEYBOARD_PRESS_CHAR_UUID:
      //Validate the value
      // Make sure it's not a blob oper
      if ( offset == 0 ){
        if ( len != 1 ){
          status = ATT_ERR_INVALID_VALUE_SIZE;
        }
      }else{
        status = ATT_ERR_ATTR_NOT_LONG;
      }
      //Write the value
      if ( status == SUCCESS )
      {
        uint8 *pCurValue = (uint8 *)pAttr->pValue;
        *pCurValue = pValue[0];
        notifyApp = KEYBOARD_PRESS_CHAR;        
      }
      break;
    case KEYBOARD_TYPE_CHAR_UUID:
      //Validate the value
      // Make sure it's not a blob oper
      if ( offset == 0 ){
        if ( len >(sizeof(keyboardTypeChar)-1) ){
          status = ATT_ERR_INVALID_VALUE_SIZE;
        }
      }else{
        status = ATT_ERR_ATTR_NOT_LONG;
      }
      //Write the value
      if ( status == SUCCESS )
      {
        uint8 *pCurValue = (uint8 *)pAttr->pValue;
        (void)memcpy(pCurValue, pValue, len); 
        pCurValue[len]='\0';
        notifyApp = KEYBOARD_TYPE_CHAR;        
      }
      break;
      
    case GATT_CLIENT_CHAR_CFG_UUID:
      status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                               offset, GATT_CLIENT_CFG_NOTIFY );
      break;
      
    default:
      // Should never get here! (characteristics 2 and 4 do not have write permissions)
      status = ATT_ERR_ATTR_NOT_FOUND;
      break;
  }


  // If a charactersitic value changed then callback function to notify application of change
  if ( (notifyApp != 0xFF ) && simpleProfile_AppCBs && simpleProfile_AppCBs->pfnKeyboardDongleProfileChange )
  {
    simpleProfile_AppCBs->pfnKeyboardDongleProfileChange( notifyApp );  
  }
  
  return ( status );
}

/*********************************************************************
*********************************************************************/
