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

#include "up_api.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
/* #include "bsp.h" */
/* #include "gpio.h" */
#include "osal.h"
#include "math.h"

#include <stdio.h>

static struct
{
   struct
   {
      uint8_t i8;
   } i8;
   struct
   {
      uint8_t i8;
      uint8_t o8;
   } io8;
} my_slot_data;

void * up_vars[] =
{
   &my_slot_data.i8.i8,
   &my_slot_data.io8.i8,
   &my_slot_data.io8.o8,
   NULL
};

static up_signal_t slot_I8_inputs[] =
{
   {
      .name = "Input 8 bits",
      .ix = 0,
      .datatype = UP_DTYPE_UINT8,
      .bitlength = 8,
   },
};

static up_signal_t slot_IO8_inputs[] =
{
   {
      .name = "Input 8 bits",
      .ix = 1,
      .datatype = UP_DTYPE_UINT8,
      .bitlength = 8,
   },
};

static up_signal_t slot_IO8_outputs[] =
{
   {
      .name = "Output 8 bits",
      .ix = 2,
      .datatype = UP_DTYPE_UINT8,
      .bitlength = 8,
   },
};

static up_slot_t slots[] =
{
   {
      .name = "I8",
      .n_inputs = NELEMENTS (slot_I8_inputs),
      .inputs = slot_I8_inputs,
   },
   {
      .name = "IO8",
      .n_inputs = NELEMENTS (slot_IO8_inputs),
      .inputs = slot_IO8_inputs,
      .n_outputs = NELEMENTS (slot_I8_inputs),
      .outputs = slot_IO8_outputs,
   },
};

static up_device_t device =
{
   .name = "digio",
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

   my_slot_data.i8.i8++;
   up_write_inputs (up);
}

static up_cfg_t cfg =
{
   .device = &device,
   .vars = up_vars,
   .sync = cb_sync,
   .avail = cb_avail,
};

/* TODO: erpc callbacks have no arguments, keeping global instance for now */
static up_t * up;

void upi_avail (void)
{
   cfg.avail (up);
}

void upi_sync (void)
{
   cfg.sync (up);
}

int main (int argc, char * argv[])
{
   int error;

   if (argc != 2)
   {
      printf ("usage: %s <ip of uphycore>\n", argv[0]);
      exit (EXIT_FAILURE);
   }

   up = up_init (&cfg);

   error = up_tcp_transport_init (up, argv[1], 5150);
   if (error)
   {
      printf ("Failed to bring up transport\n");
      exit (EXIT_FAILURE);
   }

   error = up_rpc_init (up);
   if (error)
   {
      printf ("Failed to connect\n");
      exit (EXIT_FAILURE);
   }

   while(1)
   {
      os_usleep (1000 * 1000);
   }

   return 0;
}
