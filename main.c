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
#include "model.h"

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

#undef ENABLE_IO_FILES

#ifdef ENABLE_IO_FILES
void write_status_file (void);
void write_input_file (void);
void read_input_file (void);
#endif

static void cb_avail (up_t * up)
{
   up_read_outputs (up);

   /* Process outputs */
#if 0
   up_data.io8.o8 = my_filter (up_data.io8.o8);
#endif
}

static void cb_sync (up_t * up)
{
#if 0
   /* Activate outputs */
   gpio_set (GPIO_LED_D6, up_data.io8.o8 & BIT (0));
   gpio_set (GPIO_LED_D7, up_data.io8.o8 & BIT (1));
   gpio_set (GPIO_LED_D8, up_data.io8.o8 & BIT (2));
   gpio_set (GPIO_LED_D9, up_data.io8.o8 & BIT (3));

   /* Latch inputs (read back leds for now) */
   up_data.i8.i8 =
      gpio_get (GPIO_LED_D6) << 0 |
      gpio_get (GPIO_LED_D7) << 1 |
      gpio_get (GPIO_LED_D8) << 2 |
      gpio_get (GPIO_LED_D9) << 3;
   up_data.io8.i8 =
      gpio_get (GPIO_LED_D6) << 0 |
      gpio_get (GPIO_LED_D7) << 1 |
      gpio_get (GPIO_LED_D8) << 2 |
      gpio_get (GPIO_LED_D9) << 3;

   /* Process inputs */
   /* up_data.io8.i8 = my_filter (up_data.io8.i8); */

#if 0
   /* Simulate inputs */
   double t = tick_get();
   up_data.i8.i8 = 100 + 100 * sin (t/1000.0);
   up_data.io8.i8 = 80 + 80 * cos (t/800.0);
#endif
#endif

   up_write_inputs (up);

#ifdef ENABLE_IO_FILES
   read_input_file();
   write_status_file();
#else
   up_data.i8.i8++;
   up_data.io8.i8++;
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
      p = &up_device.slots[slot_ix].params[param_ix];
      memcpy (up_vars[p->ix], data.data, data.dataLength);
   }
}

static up_cfg_t cfg = {
   .device = &up_device,
   .busconf = &up_busconf,
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

   for (uint32_t s = 0; s < up_device.n_slots; s++)
   {
      slot = &up_device.slots[s];

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

   for (uint32_t s = 0; s < up_device.n_slots; s++)
   {
      slot = &up_device.slots[s];

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
   for (i = 0; i < up_device.n_slots; i++)
   {
      slot = &up_device.slots[i];
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
