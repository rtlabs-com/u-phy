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
#include "up_util.h"
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
#else
#include <unistd.h>
#endif

#define ENABLE_IO_FILES

/* Start and end tags for the generated EtherCAT SII eeprom.
 * Defined in eeprom.S.
 */
extern const uint8_t _eeprom_bin_start;
extern const uint8_t _eeprom_bin_end;

static void cb_avail (up_t * up, void * user_arg)
{
   up_read_outputs (up);

   /* Process outputs */
#if 0
   up_data.io8.o8 = my_filter (up_data.io8.o8);
#endif
}

static void cb_sync (up_t * up, void * user_arg)
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
   up_util_read_input_file ("/tmp/u-phy-input.txt");
   up_util_write_status_file ("/tmp/u-phy-status.txt");
#else
   up_data.i8.i8++;
   up_data.io8.i8++;
#endif
}

static void cb_param_write_ind (up_t * up, void * user_arg)
{
   uint16_t slot_ix;
   uint16_t param_ix;
   binary_t data;
   up_param_t * p;

   while (up_param_get_write_req (up, &slot_ix, &param_ix, &data) == 0)
   {
      p = &up_device.slots[slot_ix].params[param_ix];
      memcpy (up_vars[p->ix], data.data, data.dataLength);
   }
}

#if defined(ENABLE_IO_FILES) && !defined(__rtk__)
static void cb_loop_ind (up_t * up, void * user_arg)
{
   up_util_poll_cmd_file ("/tmp/u-phy-command.txt");
}
#endif

static up_busconf_t up_busconf;

static up_cfg_t cfg = {
   .device = &up_device,
   .busconf = &up_busconf,
   .vars = up_vars,
   .sync = cb_sync,
   .avail = cb_avail,
   .param_write_ind = cb_param_write_ind,
#if defined(ENABLE_IO_FILES) && !defined(__rtk__)
   .poll_ind = cb_loop_ind,
#endif
};

int _cmd_start (int argc, char * argv[])
{
   int error;

   if (argc != 3)
   {
      printf ("usage: %s <transport cfg> <fieldbus>\n", argv[0]);
      printf ("example: %s /dev/ttyUSB0 profinet\n", argv[0]);
      exit (EXIT_FAILURE);
   }

   if (strcmp (argv[2], "profinet") == 0)
   {
#ifdef UP_DEVICE_PROFINET_SUPPORTED
      cfg.device->bustype = UP_BUSTYPE_PROFINET,
      up_busconf.profinet = up_profinet_config;
#else
      printf ("The device model has no Profinet configuration\n");
      exit (EXIT_FAILURE);
#endif
   }
   else if (strcmp (argv[2], "ethercat") == 0)
   {
#ifdef UP_DEVICE_ETHERCAT_SUPPORTED
      cfg.device->bustype = UP_BUSTYPE_ECAT,
      up_busconf.ecat = up_ethercat_config;
#else
      printf ("The device model has no EtherCAT configuration\n");
      exit (EXIT_FAILURE);
#endif
   }
   else if (strcmp (argv[2], "mock") == 0)
   {
      cfg.device->bustype = UP_BUSTYPE_MOCK;
      up_busconf.mock = up_mock_config;
   }
   else
   {
      printf ("Unsupported fieldbus \"%s\", abort\n", argv[2]);
      exit (EXIT_FAILURE);
   }

   printf ("Starting sample host application\n");
   up_t * up = up_init (&cfg);

#if defined(OPTION_TRANSPORT_TCP)
   if (argc != 2)
   {
      printf ("usage: %s <ip of u-phy core> <fieldbus>\n", argv[0]);
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
   if (argc != 3)
   {
      printf ("usage: %s <port> <fieldbus>\n", argv[0]);
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
   if (argc != 3)
   {
      printf ("usage: %s <port> <fieldbus>\n", argv[0]);
      exit (EXIT_FAILURE);
   }

   error = up_serial_transport_init (up, argv[1]);
   if (error)
   {
      printf ("Failed to bring up transport\n");
      exit (EXIT_FAILURE);
   }
#endif

   error = up_rpc_init (up, true);
   if (error)
   {
      printf ("Failed to connect to u-phy core\n");
      exit (EXIT_FAILURE);
   }

#if !defined(__rtk__)
   /* Delay startup so that there is time to restore the second serial
    * connection, used for logging, before the device configuration is
    * sent to the core.
    */
   sleep (1);
#endif

   if (up_init_device (up) != 0)
   {
      printf ("Failed to configure device\n");
      exit (EXIT_FAILURE);
   }

   if (cfg.device->bustype == UP_BUSTYPE_ECAT)
   {
      error = up_write_ecat_eeprom (
         up,
         &_eeprom_bin_start,
         &_eeprom_bin_end - &_eeprom_bin_start);

      if (error != 0)
      {
         printf ("Failed to write EtherCAT eeprom \n");
         return error;
      }
   }

   if (up_util_init (&up_device, up, up_vars) != 0)
   {
      printf ("Failed to init up utils\n");
      exit (EXIT_FAILURE);
   }

   if (up_start_device (up) != 0)
   {
      printf ("Failed to start device\n");
      exit (EXIT_FAILURE);
   }

#ifdef ENABLE_IO_FILES
   /* Generate template input file */
   up_util_write_input_file ("/tmp/u-phy-input.txt");
   /* Generate default status file */
   up_util_write_status_file ("/tmp/u-phy-status.txt");
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
