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

typedef struct up_data
{
   struct
   {
      uint8_t i8;
   } i8;
   struct
   {
      uint8_t o8;
   } o8;
   struct
   {
      uint8_t i8;
      uint8_t o8;
      int32_t param_1;
      float param_2;
   } io8;
#ifdef ENABLE_IODEMO_SLOT
   struct
   {
      int8_t in_i8;
      uint8_t in_u8;
      int16_t in_i16;
      uint16_t in_u16;
      int32_t in_i32;
      uint32_t in_u32;
      float in_f;

      int8_t out_i8;
      uint8_t out_u8;
      int16_t out_i16;
      uint16_t out_u16;
      int32_t out_i32;
      uint32_t out_u32;
      float out_f;
   } iodemo;
#endif
} up_data_t;

extern up_data_t up_data;
extern void * up_vars[];
extern up_device_t up_device;
extern up_busconf_t up_busconf;

/* EtherCAT adapter configuration */
void up_ecat_configure (up_device_t * device, up_busconf_t * busconf);

#ifdef __cplusplus
}
#endif

#endif /* MODEL_H */
