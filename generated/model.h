/*********************************************************************
 *        _       _         _
 *  _ __ | |_  _ | |  __ _ | |__   ___
 * | '__|| __|(_)| | / _` || '_ \ / __|
 * | |   | |_  _ | || (_| || |_) |\__ \
 * |_|    \__|(_)|_| \__,_||_.__/ |___/
 *
 * http://www.rt-labs.com
 * Copyright 2022 rt-labs AB, Sweden.
 * See LICENSE file in the project root for full license information.
 ********************************************************************/

#ifndef MODEL_H
#define MODEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "up_types.h"

#define UP_DEVICE_PROFINET_SUPPORTED 1
#define UP_DEVICE_ETHERCAT_SUPPORTED 1
#define UP_DEVICE_ETHERNETIP_SUPPORTED 1

/* Alarm error codes */
#define UP_ERROR_CODE_I8O8_ALARM_400 400
#define UP_ERROR_CODE_I8O8_ALARM_500 500

typedef struct up_data
{
   struct
   {
      struct
      {
         uint8_t value;
         up_signal_status_t status;
      } Input_8_bits;
   } I8;
   struct
   {
      struct
      {
         uint8_t value;
         up_signal_status_t status;
      } Output_8_bits;
   } O8;
   struct
   {
      struct
      {
         uint8_t value;
         up_signal_status_t status;
      } Input_8_bits;
      struct
      {
         uint8_t value;
         up_signal_status_t status;
      } Output_8_bits;
      uint32_t Parameter_1;
   } I8O8;
} up_data_t;

extern up_data_t up_data;
extern up_signal_info_t up_vars[];
extern up_device_t up_device;
extern up_profinet_config_t up_profinet_config;
extern up_ecat_device_t up_ethercat_config;
extern up_ethernetip_config_t up_ethernetip_config;
extern up_mockadapter_config_t up_mock_config;

#ifdef __cplusplus
}
#endif

#endif /* MODEL_H */