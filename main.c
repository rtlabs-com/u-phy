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

#include "options.h"
#include "up_api.h"

#include "math.h"
#include "osal.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#if defined(__rtk__)
#include "bsp.h"
#include "gpio.h"
#include "shell.h"
#endif

#undef ENABLE_IODEMO_SLOT

static struct
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
} my_slot_data;

void * up_vars[] = {
   &my_slot_data.i8.i8,       /* ix = 0*/
   &my_slot_data.o8.o8,       /* ix = 1*/
   &my_slot_data.io8.i8,      /* ix = 2*/
   &my_slot_data.io8.o8,      /* ix = 3*/
   &my_slot_data.io8.param_1, /* ix = 4*/
   &my_slot_data.io8.param_2, /* ix = 5*/

#ifdef ENABLE_IODEMO_SLOT
   &my_slot_data.iodemo.in_u8,   /* ix = 6*/
   &my_slot_data.iodemo.in_i8,   /* ix = 7*/
   &my_slot_data.iodemo.in_u16,  /* ix = 8*/
   &my_slot_data.iodemo.in_i16,  /* ix = 9*/
   &my_slot_data.iodemo.in_u32,  /* ix = 10*/
   &my_slot_data.iodemo.in_i32,  /* ix = 11*/
   &my_slot_data.iodemo.in_f,    /* ix = 12*/
   &my_slot_data.iodemo.out_u8,  /* ix = 13*/
   &my_slot_data.iodemo.out_i8,  /* ix = 14*/
   &my_slot_data.iodemo.out_u16, /* ix = 15*/
   &my_slot_data.iodemo.out_i16, /* ix = 16*/
   &my_slot_data.iodemo.out_u32, /* ix = 17*/
   &my_slot_data.iodemo.out_i32, /* ix = 18*/
   &my_slot_data.iodemo.out_f,   /* ix = 19*/
#endif
   NULL};

static up_signal_t slot_I8_inputs[] = {
   {
      .name = "I8 Input 8 bits",
      .ix = 0, // Into up_vars
      .datatype = UP_DTYPE_UINT8,
      .bitlength = 8,
      .frame_offset = 0,
   },
};

static up_signal_t slot_O8_outputs[] = {
   {
      .name = "O8 Output 8 bits",
      .ix = 1,
      .datatype = UP_DTYPE_UINT8,
      .bitlength = 8,
      .frame_offset = 0,
   },
};

static up_signal_t slot_IO8_inputs[] = {
   {
      .name = "IO8 Input 8 bits",
      .ix = 2,
      .datatype = UP_DTYPE_UINT8,
      .bitlength = 8,
      .frame_offset = 1,
   },
};

static up_signal_t slot_IO8_outputs[] = {
   {
      .name = "IO8 Output 8 bits",
      .ix = 3,
      .datatype = UP_DTYPE_UINT8,
      .bitlength = 8,
      .frame_offset = 1,
   },
};

static int32_t param_1_default_value = 12;
static float param_2_default_value = 1.1;

static up_param_t slot_IO8_parameters[] = {
   {
      .name = "IO8 Demo parameter int",
      .ix = 4,
      .pn_param_index = 123,
      .datatype = UP_DTYPE_INT32,
      .bitlength = 32,
      .default_value.dataLength = 4,
      .default_value.data = (void *)&param_1_default_value,
      .value.dataLength = 4,
      .value.data = (void *)&param_1_default_value,
   },
   {
      .name = "IO8 Demo parameter float",
      .ix = 5,
      .pn_param_index = 124,
      .datatype = UP_DTYPE_FLOAT32,
      .bitlength = 32,
      .default_value.dataLength = 4,
      .default_value.data = (void *)&param_2_default_value,
      .value.dataLength = 4,
      .value.data = (void *)&param_2_default_value,
   },
};

#ifdef ENABLE_IODEMO_SLOT
static up_signal_t slot_IODemo_inputs[] = {
   {
      .name = "Input uint8",
      .ix = 6,
      .datatype = UP_DTYPE_UINT8,
      .bitlength = 8,
      .frame_offset = 2,
   },
   {
      .name = "Input int8",
      .ix = 7,
      .datatype = UP_DTYPE_INT8,
      .bitlength = 8,
      .frame_offset = 3,
   },
   {
      .name = "Input uint16",
      .ix = 8,
      .datatype = UP_DTYPE_UINT16,
      .bitlength = 16,
      .frame_offset = 4,
   },
   {
      .name = "Input int16",
      .ix = 9,
      .datatype = UP_DTYPE_INT16,
      .bitlength = 16,
      .frame_offset = 6,
   },
   {
      .name = "Input uint32",
      .ix = 10,
      .datatype = UP_DTYPE_UINT32,
      .bitlength = 32,
      .frame_offset = 8,
   },
   {
      .name = "Input int32",
      .ix = 11,
      .datatype = UP_DTYPE_INT32,
      .bitlength = 32,
      .frame_offset = 12,
   },
   {
      .name = "Input float",
      .ix = 12,
      .datatype = UP_DTYPE_FLOAT32,
      .bitlength = 32,
      .frame_offset = 16,
   },
};

static up_signal_t slot_IODemo_outputs[] = {

   {
      .name = "Output uint8",
      .ix = 13,
      .datatype = UP_DTYPE_UINT8,
      .bitlength = 8,
      .frame_offset = 2,
   },
   {
      .name = "Output int8",
      .ix = 14,
      .datatype = UP_DTYPE_INT8,
      .bitlength = 8,
      .frame_offset = 3,
   },

   {
      .name = "Output uint16",
      .ix = 15,
      .datatype = UP_DTYPE_UINT16,
      .bitlength = 16,
      .frame_offset = 4,
   },
   {
      .name = "Output int16",
      .ix = 16,
      .datatype = UP_DTYPE_INT16,
      .bitlength = 16,
      .frame_offset = 6,
   },
   {
      .name = "Output uint32",
      .ix = 17,
      .datatype = UP_DTYPE_UINT32,
      .bitlength = 32,
      .frame_offset = 8,
   },
   {
      .name = "Output int32",
      .ix = 18,
      .datatype = UP_DTYPE_INT32,
      .bitlength = 32,
      .frame_offset = 12,
   },
   {
      .name = "Output float",
      .ix = 19,
      .datatype = UP_DTYPE_FLOAT32,
      .bitlength = 32,
      .frame_offset = 16,
   },
};
#endif

static up_slot_t slots[] = {
   {
      .name = "I8",
      .profinet_module_id = 0x100,
      .profinet_submodule_id = 0x101,
      .input_bitlength = 8,
      .n_inputs = NELEMENTS (slot_I8_inputs),
      .inputs = slot_I8_inputs,
   },
   {
      .name = "O8",
      .profinet_module_id = 0x200,
      .profinet_submodule_id = 0x201,
      .output_bitlength = 8,
      .n_outputs = NELEMENTS (slot_O8_outputs),
      .outputs = slot_O8_outputs,
   },
   {
      .name = "IO8",
      .profinet_module_id = 0x300,
      .profinet_submodule_id = 0x301,
      .input_bitlength = 8,
      .output_bitlength = 8,
      .n_inputs = NELEMENTS (slot_IO8_inputs),
      .inputs = slot_IO8_inputs,
      .n_outputs = NELEMENTS (slot_IO8_outputs),
      .outputs = slot_IO8_outputs,
      .n_params = NELEMENTS (slot_IO8_parameters),
      .params = slot_IO8_parameters,
   },
#ifdef ENABLE_IODEMO_SLOT
   {
      .name = "IODemo",
      .profinet_module_id = 0x400,
      .profinet_submodule_id = 0x401,
      .input_bitlength = 144,
      .output_bitlength = 144,
      .n_inputs = NELEMENTS (slot_IODemo_inputs),
      .inputs = slot_IODemo_inputs,
      .n_outputs = NELEMENTS (slot_IODemo_outputs),
      .outputs = slot_IODemo_outputs,
   },
#endif
};

static up_device_t device = {
   .name = "U-PHY MOD01",
   .serial_number = "20220112_A12",
   .bustype = UP_BUSTYPE_PROFINET,
   .busconf.profinet.vendor_id = 0x0493,
   .busconf.profinet.device_id = 0x0003,
   .busconf.profinet.profile_id = 0x1234,
   .busconf.profinet.profile_specific_type = 0x5678,
   .busconf.profinet.min_device_interval = 32,
   .busconf.profinet.default_stationname = "u-phy-dev",
   .busconf.profinet.order_id = "MOD01",
   .busconf.profinet.hw_revision = 1,
   .busconf.profinet.sw_revision_prefix = 'V',
   .busconf.profinet.sw_revision_functional_enhancement = 0,
   .busconf.profinet.sw_revision_bug_fix = 1,
   .busconf.profinet.sw_revision_internal_change = 27,
   .busconf.profinet.revision_counter = 0,
   .n_slots = NELEMENTS (slots),
   .slots = slots,
};

static void cb_avail (up_t * up)
{
   up_read_outputs (up);

   /* Process outputs */
#if 0
   my_slot_data.io8.o8 = my_filter (my_slot_data.io8.o8);
#endif
}

static void cb_sync (up_t * up)
{
#if 0
   /* Activate outputs */
   gpio_set (GPIO_LED_D6, my_slot_data.io8.o8 & BIT (0));
   gpio_set (GPIO_LED_D7, my_slot_data.io8.o8 & BIT (1));
   gpio_set (GPIO_LED_D8, my_slot_data.io8.o8 & BIT (2));
   gpio_set (GPIO_LED_D9, my_slot_data.io8.o8 & BIT (3));

   /* Latch inputs (read back leds for now) */
   my_slot_data.i8.i8 =
      gpio_get (GPIO_LED_D6) << 0 |
      gpio_get (GPIO_LED_D7) << 1 |
      gpio_get (GPIO_LED_D8) << 2 |
      gpio_get (GPIO_LED_D9) << 3;
   my_slot_data.io8.i8 =
      gpio_get (GPIO_LED_D6) << 0 |
      gpio_get (GPIO_LED_D7) << 1 |
      gpio_get (GPIO_LED_D8) << 2 |
      gpio_get (GPIO_LED_D9) << 3;

   /* Process inputs */
   /* my_slot_data.io8.i8 = my_filter (my_slot_data.io8.i8); */

#if 0
   /* Simulate inputs */
   double t = tick_get();
   my_slot_data.i8.i8 = 100 + 100 * sin (t/1000.0);
   my_slot_data.io8.i8 = 80 + 80 * cos (t/800.0);
#endif
#endif
   my_slot_data.i8.i8 += 1;
   my_slot_data.io8.i8 += 1;

#ifdef ENABLE_IODEMO_SLOT
   my_slot_data.iodemo.in_u8 = my_slot_data.iodemo.out_u8;
   my_slot_data.iodemo.in_i8 = my_slot_data.iodemo.out_i8;
   my_slot_data.iodemo.in_u16 = my_slot_data.iodemo.out_u16;
   my_slot_data.iodemo.in_i16 = my_slot_data.iodemo.out_i16;
   my_slot_data.iodemo.in_u32 = my_slot_data.iodemo.out_u32;
   my_slot_data.iodemo.in_i32 = my_slot_data.iodemo.out_i32;
   my_slot_data.iodemo.in_f = my_slot_data.iodemo.out_f;

#endif

   /*printf (
      "Data to PLC: 0x%02X   Data from PLC: 0x%02X\n",
      my_slot_data.io8.i8,
      my_slot_data.io8.o8);*/

   up_write_inputs (up);
}

static void cb_param_write_ind (up_t * up)
{
   uint16_t slot_ix;
   uint16_t param_ix;
   binary_t data;
   up_param_t * p;

   printf ("Param write indication - poll pending write requests\n");

   while (up_param_get_write_req (&slot_ix, &param_ix, &data) == 0)
   {
      printf (
         "param_write_req slot=%" PRIu16 " param=%" PRIu16 "\n",
         slot_ix,
         param_ix);

      /* TODO - Application updates parameters */
      p = &slots[slot_ix].params[param_ix];
      memcpy (up_vars[p->ix], data.data, data.dataLength);

      if (param_ix == 0)
      {
         printf ("  %s: %" PRIi32 "\n", p->name, *(int32_t *)up_vars[p->ix]);
      }

      if (param_ix == 1)
      {
         printf ("  %s: %3.3f\n", p->name, *(float *)up_vars[p->ix]);
      }
   }
}

static up_cfg_t cfg = {
   .device = &device,
   .vars = up_vars,
   .sync = cb_sync,
   .avail = cb_avail,
   .param_write_ind = cb_param_write_ind,
};

int _cmd_start (int argc, char * argv[])
{
   int error;

   printf ("Starting sample host application\n");

   up_t * up = up_init (&cfg);
   memset (&my_slot_data, 0, sizeof (my_slot_data));

#if defined(OPTION_TRANSPORT_TCP)
   if (argc != 2)
   {
      printf ("usage: %s <ip of uphycore>\n", argv[0]);
      exit (EXIT_FAILURE);
   }

   error = up_tcp_transport_init (up, argv[1], 5150);
   if (error)
   {
      printf ("Failed to bring up transport\n");
      exit (EXIT_FAILURE);
   }
#endif

#if defined(__rtk__) && defined(OPTION_TRANSPORT_UART)
   if (argc != 2)
   {
      printf ("usage: %s <port>\n", argv[0]);
      exit (EXIT_FAILURE);
   }

   error = up_uart_transport_init (up, argv[1]);
   if (error)
   {
      printf ("Failed to bring up transport\n");
      exit (EXIT_FAILURE);
   }
#endif

#if defined(__linux__) && defined(OPTION_TRANSPORT_UART)
   if (argc != 2)
   {
      printf ("usage: %s <port>\n", argv[0]);
      exit (EXIT_FAILURE);
   }

   error = up_serial_transport_init (up, argv[1]);
   if (error)
   {
      printf ("Failed to bring up transport\n");
      exit (EXIT_FAILURE);
   }
#endif

   error = up_rpc_init (up);
   if (error)
   {
      printf ("Failed to connect to u-phy core\n");
      exit (EXIT_FAILURE);
   }

   if (up_init_device (up) != 0)
   {
      printf ("Failed to configure device\n");
      exit (EXIT_FAILURE);
   }

   while (1)
   {
      extern void up_worker (up_t * up);
      up_worker (up);
   }

   return 0;
}

#if defined(__rtk__)

const shell_cmd_t cmd_start = {
   .cmd = _cmd_start,
   .name = "start",
   .help_short = "start uphy",
   .help_long = ""};

SHELL_CMD (cmd_start);

int main (int argc, char * argv[])
{
   return 0;
}

#else

int main (int argc, char * argv[])
{
   setvbuf (stdout, NULL, _IONBF, 0);
   _cmd_start (argc, argv);
}

#endif
