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
#undef ENABLE_IO_FILES

#ifdef ENABLE_IO_FILES
void write_status_file (void);
void write_input_file (void);
void read_input_file (void);
#endif

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
      .frame_offset = 0,
      .default_value.dataLength = 4,
      .default_value.data = (void *)&param_1_default_value,
   },
   {
      .name = "IO8 Demo parameter float",
      .ix = 5,
      .pn_param_index = 124,
      .datatype = UP_DTYPE_FLOAT32,
      .bitlength = 32,
      .frame_offset = 4,
      .default_value.dataLength = 4,
      .default_value.data = (void *)&param_2_default_value,
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

   up_write_inputs (up);

#ifdef ENABLE_IO_FILES
   read_input_file();
   write_status_file();
#endif
}

static void cb_param_write_ind (up_t * up)
{
   uint16_t slot_ix;
   uint16_t param_ix;
   binary_t data;
   up_param_t * p;

   while (up_param_get_write_req (&slot_ix, &param_ix, &data) == 0)
   {
      p = &slots[slot_ix].params[param_ix];
      memcpy (up_vars[p->ix], data.data, data.dataLength);
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

#ifdef ENABLE_IO_FILES
   /* Generate template input file*/
   write_input_file();
#endif

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

#ifdef ENABLE_IO_FILES
void signal_val2str (char * value_string, size_t size, void * value, up_dtype_t dtype)
{
   switch (dtype)
   {
   case UP_DTYPE_INT8:
      snprintf (value_string, size, "%" PRIi8 "", *(int8_t *)value);
      break;
   case UP_DTYPE_UINT8:
      snprintf (value_string, size, "%" PRIu8 "", *(uint8_t *)value);
      break;
   case UP_DTYPE_INT16:
      snprintf (value_string, size, "%" PRIi16 "", *(int16_t *)value);
      break;
   case UP_DTYPE_UINT16:
      snprintf (value_string, size, "%" PRIu16 "", *(uint16_t *)value);
      break;
   case UP_DTYPE_INT32:
      snprintf (value_string, size, "%" PRIi32 "", *(int32_t *)value);
      break;
   case UP_DTYPE_UINT32:
      snprintf (value_string, size, "%" PRIu32 "", *(uint32_t *)value);
      break;
   case UP_DTYPE_FLOAT32:
      snprintf (value_string, size, "%f", *(float *)value);
      break;
   case UP_DTYPE_BYTE_ARRAY:

   default:
      snprintf (value_string, size, "unknown");
      break;
   }
}

void dump_device_status (FILE * f)
{
   up_slot_t * slot;
   up_signal_t * signal;
   up_param_t * param;
   char value_string[24];

   for (uint32_t s = 0; s < device.n_slots; s++)
   {
      slot = &device.slots[s];

      fprintf (f, "Slot %" PRIu32 " \"%s\"\n", s, slot->name);

      if (slot->n_inputs > 0)
      {
         fprintf (f, "  Inputs\n");
      }

      for (uint32_t ix = 0; ix < slot->n_inputs; ix++)
      {
         signal = &slot->inputs[ix];

         signal_val2str (
            value_string,
            sizeof (value_string),
            up_vars[signal->ix],
            signal->datatype);

         fprintf (f, "    %" PRIu32 " \"%s\" %s\n", ix, signal->name, value_string);
      }

      if (slot->n_outputs > 0)
      {
         fprintf (f, "  Outputs\n");
      }

      for (uint32_t ix = 0; ix < slot->n_outputs; ix++)
      {
         signal = &slot->outputs[ix];

         signal_val2str (
            value_string,
            sizeof (value_string),
            up_vars[signal->ix],
            signal->datatype);

         fprintf (f, "    %" PRIu32 " \"%s\" %s\n", ix, signal->name, value_string);
      }
      if (slot->n_params > 0)
      {
         fprintf (f, "  Parameters\n");
      }

      for (uint32_t ix = 0; ix < slot->n_params; ix++)
      {
         param = &slot->params[ix];

         signal_val2str (
            value_string,
            sizeof (value_string),
            up_vars[param->ix],
            param->datatype);

         fprintf (f, "    %" PRIu32 " \"%s\" %s\n", ix, param->name, value_string);
      }
   }
   fprintf (f, "\n");
}

void write_status_file (void)
{
   FILE * f = fopen ("/tmp/u-phy-sample.txt", "w+");
   if (f == NULL)
   {
      printf ("Failed to open \"/tmp/u-phy-sample.txt\"\n");
      return;
   }

   dump_device_status (f);

   fclose (f);
}

/* Generate and inital input file */
void write_input_file (void)
{
   up_slot_t * slot;
   up_signal_t * signal;
   char value_string[32];

   FILE * f = fopen ("/tmp/u-phy-input.txt", "w+");
   if (f == NULL)
   {
      printf ("Failed to open \"/tmp/u-phy-input.txt\"\n");
      return;
   }

   for (uint32_t s = 0; s < device.n_slots; s++)
   {
      slot = &device.slots[s];

      if (slot->n_inputs > 0)
      {
         for (uint32_t ix = 0; ix < slot->n_inputs; ix++)
         {
            signal = &slot->inputs[ix];

            signal_val2str (
               value_string,
               sizeof (value_string),
               up_vars[signal->ix],
               signal->datatype);

            fprintf (
               f,
               "%" PRIu16 " \"%s\" %s\n",
               signal->ix,
               signal->name,
               value_string);
         }
      }
   }
   fclose (f);
}

/* Write up_vars inputs from input file */
void set_input (char * ix_str, char * val_str)
{
   int ix;
   uint16_t i, j;
   up_slot_t * slot;
   up_signal_t * signal;

   if (ix_str == NULL || val_str == NULL)
   {
      return;
   }

   ix = atoi (ix_str);
   for (i = 0; i < device.n_slots; i++)
   {
      slot = &device.slots[i];
      for (j = 0; j < slot->n_inputs; j++)
      {
         signal = &slot->inputs[j];
         if (signal->ix == ix)
         {
            switch (signal->datatype)
            {
            case UP_DTYPE_UINT8:
            {
               uint8_t value = (uint8_t)atoi (val_str);
               *(uint8_t *)up_vars[ix] = value;
            }
            break;

            case UP_DTYPE_INT8:
            {
               int8_t value = (int8_t)atoi (val_str);
               *(int8_t *)up_vars[ix] = value;
            }
            break;
            case UP_DTYPE_INT16:
            {
               int16_t value = (int16_t)atoi (val_str);
               *(int16_t *)up_vars[ix] = value;
            }
            break;
            case UP_DTYPE_UINT16:
            {
               uint16_t value = (uint16_t)atoi (val_str);
               *(uint16_t *)up_vars[ix] = value;
            }
            break;
            case UP_DTYPE_INT32:
            {
               int32_t value = (int32_t)atoi (val_str);
               *(int32_t *)up_vars[ix] = value;
            }
            break;
            case UP_DTYPE_UINT32:
            {
               uint32_t value = (uint32_t)atoi (val_str);
               *(uint32_t *)up_vars[ix] = value;
            }
            break;
            case UP_DTYPE_FLOAT32:
            {
               float value = (float)atof (val_str);
               *(float *)up_vars[ix] = value;
            }
            break;
            default:
               /* Ignore*/
               break;
            }
            return;
         }
      }
   }
}

/* Read input file with expected format:
"ix name value" where
ix is index in up_vars
name is ignored
value is value.
*/
void read_input_file (void)
{
   char * p;
   char line[200];

   char * ix;
   char * token;
   char * value;

   FILE * f = fopen ("/tmp/u-phy-input.txt", "r");
   if (f == NULL)
   {
      printf ("Failed to open \"/tmp/u-phy-input.txt\"\n");
      return;
   }

   p = fgets (line, sizeof (line), f);
   while (p != NULL)
   {
      value = NULL;
      ix = NULL;

      if (p != NULL)
      {
         token = strtok_r (p, " ", &p);
         if (token != NULL)
         {
            ix = token;

            /* use last word in line as value */
            token = strtok_r (p, " ", &p);
            while (token != NULL)
            {
               value = token;
               token = strtok_r (p, " ", &p);
            }

            set_input (ix, value);
         }
      }
      p = fgets (line, sizeof (line), f);
   }

   fclose (f);
}

#endif