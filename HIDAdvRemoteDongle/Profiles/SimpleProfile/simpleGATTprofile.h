/******************************************************************************

 @file  simpleGATTprofile.h

 @brief This file contains the Simple GATT profile definitions and prototypes.

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

#ifndef SIMPLEGATTPROFILE_H
#define SIMPLEGATTPROFILE_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */

/*********************************************************************
 * CONSTANTS
 */

// Profile Parameters
#define KEYBOARD_PRESS_CHAR                   0  // RW uint8 - Profile Characteristic 1 value 
#define KEYBOARD_TYPE_CHAR                   1  // RW uint8 - Profile Characteristic 2 value
#define KEYBOARD_REPORT_CHAR                   2  // RW uint8 - Profile Characteristic 3 value
#define KEYBOARD_LED_CHAR                   3  // RW uint8 - Profile Characteristic 4 value
#define MOUSE_MOVE_CHAR                   4  // RW uint8 - Profile Characteristic 4 value
  
// Keyboard Dongle Profile Service UUID
#define KBD_DONGLE_SERV_UUID               0x0000
/* The 16 bit UUID listen above is only a part of the 
 * full DEVICE CONTROL 128 bit UUID:                       
 * a631XXXX-f090-4a97-8de8-4848ca7238eb */    
    
// Key Pressed UUID
  
#define KEYBOARD_PRESS_CHAR_UUID            0x0001    
#define KEYBOARD_TYPE_CHAR_UUID             0x0002 
#define KEYBOARD_REPORT_CHAR_UUID           0x0003 
#define KEYBOARD_LED_CHAR_UUID              0x0004   
#define MOUSE_MOVE_CHAR_UUID                0x0005  
  
// Keyboard Dongle Profile Profile Services bit fields
#define KBD_DONGLE_SERVICE               0x00000001

// Length of Characteristic in bytes
#define MOUSE_MOVE_CHAR_LEN           4 

/*********************************************************************
 * TYPEDEFS
 */

  
/*********************************************************************
 * MACROS
 */
  
// KBD_DONGLE_SERVICE Base 128-bit UUID: a631XXXX-f090-4a97-8de8-4848ca7238eb
#define KBD_DONGLE_SERVICE_BASE_UUID_128( uuid )  0xeb, 0x38, 0x72, 0xca, 0x48, 0x48, 0xe8, 0x8d, \
                                                  0x97, 0x4a, 0x90, 0xf0, LO_UINT16( uuid ), HI_UINT16( uuid ), 0x31, 0xa6

/*********************************************************************
 * Profile Callbacks
 */

// Callback when a characteristic value has changed
typedef void (*keyboardDongleProfileChange_t)( uint8 paramID );

typedef struct
{
  keyboardDongleProfileChange_t        pfnKeyboardDongleProfileChange;  // Called when characteristic value changes
} keyboardDongleProfileCBs_t;

    

/*********************************************************************
 * API FUNCTIONS 
 */


/*
 * SimpleProfile_AddService- Initializes the Simple GATT Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 */

extern bStatus_t SimpleProfile_AddService( uint32 services );

/*
 * SimpleProfile_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
extern bStatus_t SimpleProfile_RegisterAppCBs( keyboardDongleProfileCBs_t *appCallbacks );

/*
 * SimpleProfile_SetParameter - Set a Simple GATT Profile parameter.
 *
 *    param - Profile parameter ID
 *    len - length of data to right
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 */
extern bStatus_t SimpleProfile_SetParameter( uint8 param, uint8 len, void *value );
  
/*
 * SimpleProfile_GetParameter - Get a Simple GATT Profile parameter.
 *
 *    param - Profile parameter ID
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate 
 *          data type (example: data type of uint16 will be cast to 
 *          uint16 pointer).
 */
extern bStatus_t SimpleProfile_GetParameter( uint8 param, void *value );


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEGATTPROFILE_H */
