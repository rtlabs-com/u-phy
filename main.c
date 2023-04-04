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
#endif

#if defined(__linux__)
#define ENABLE_IO_FILES
#endif /* defined(__linux__) */

/*
 * By default the sample application enables the watchdog.
 * Set ENABLE_UP_COMMUNICATION_WATCHDOG to 0 to keep the
 * watchdog disabled.
 */
#ifndef ENABLE_UP_COMMUNICATION_WATCHDOG
#define ENABLE_UP_COMMUNICATION_WATCHDOG 1
#endif

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
   up_data.I8.Input_8_bits.value++;
   up_data.I8O8.Input_8_bits.value++;
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
      memcpy (up_vars[p->ix].value, data.data, data.dataLength);
#if !defined(OPTION_MONO)
      free (data.data);
#endif
   }
}

static void cb_status_ind (up_t * up, uint32_t status, void * user_arg)
{
   printf (
      "Core status: 0x%04" PRIX32 " [%s|%s|%s]\n",
      status,
      (status & UP_CORE_RUNNING) ? "RUNNING" : "-",
      (status & UP_CORE_CONFIGURED) ? "CONFIGURED" : "-",
      (status & UP_CORE_CONNECTED) ? "CONNECTED" : "-");
}

static const char * error_code_to_str (up_error_t error_code)
{
   switch (error_code)
   {
   case UP_ERROR_NONE:
      return "UP_ERROR_NONE";
   case UP_ERROR_CORE_COMMUNICATION:
      return "UP_ERROR_CORE_COMMUNICATION";
   case UP_ERROR_PARAMETER_WRITE:
      return "UP_ERROR_PARAMETER_WRITE";
   case UP_ERROR_PARAMETER_READ:
      return "UP_ERROR_PARAMETER_READ";
   case UP_ERROR_INVALID_PROFINET_MODULE_ID:
      return "UP_ERROR_INVALID_PROFINET_MODULE_ID";
   case UP_ERROR_INVALID_PROFINET_SUBMODULE_ID:
      return "UP_ERROR_INVALID_PROFINET_SUBMODULE_ID";
   case UP_ERROR_INVALID_PROFINET_PARAMETER_INDEX:
      return "UP_ERROR_INVALID_PROFINET_PARAMETER_INDEX";
   default:
      return "UNKNOWN";
   }
}

static void cb_error_ind (up_t * up, up_error_t error_code, void * user_arg)
{
   printf (
      "ERROR: error_code=%" PRIi16 " %s\n",
      error_code,
      error_code_to_str (error_code));
}

static void cb_profinet_signal_led_ind (up_t * up, void * user_arg)
{
   printf ("Flash Profinet signal LED for 3s at 1Hz\n");
}

#ifdef ENABLE_IO_FILES
static void cb_loop_ind (up_t * up, void * user_arg)
{
   up_util_poll_cmd_file ("/tmp/u-phy-command.txt");
   up_util_read_input_file ("/tmp/u-phy-input.txt");
   up_util_write_status_file ("/tmp/u-phy-status.txt");
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
   .status_ind = cb_status_ind,
   .error_ind = cb_error_ind,
   .profinet_signal_led_ind = cb_profinet_signal_led_ind,
#ifdef ENABLE_IO_FILES
   .poll_ind = cb_loop_ind,
#endif
};

int _cmd_start (int argc, char * argv[])
{
   int error;
#ifdef ENABLE_IO_FILES
   bool first_run = true;
#endif

   /* Check command line arguments */
   if (argc != 3)
   {
      printf ("usage: %s <transport cfg> <fieldbus>\n", argv[0]);
      printf ("The <fieldbus> can be profinet, ethercat or mock\n");
      printf ("example: %s /dev/ttyUSB0 profinet\n", argv[0]);
      return -1;
   }

#ifdef OPTION_MONO
   extern int core_main (void);
   core_main();
#endif

   if (strcmp (argv[2], "profinet") == 0)
   {
#ifdef UP_DEVICE_PROFINET_SUPPORTED
      cfg.device->bustype = UP_BUSTYPE_PROFINET,
      up_busconf.profinet = up_profinet_config;
#else
      printf ("The device model has no Profinet configuration\n");
      return -1;
#endif
   }
   else if (strcmp (argv[2], "ethercat") == 0)
   {
#ifdef UP_DEVICE_ETHERCAT_SUPPORTED
      cfg.device->bustype = UP_BUSTYPE_ECAT,
      up_busconf.ecat = up_ethercat_config;
#else
      printf ("The device model has no EtherCAT configuration\n");
      return -1;
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
      return -1;
   }

   /* Initialise U-Phy and set up transport */

   printf ("Starting sample host application\n");
   up_t * up = up_init (&cfg);

#if !defined(OPTION_MONO)
#if defined(OPTION_TRANSPORT_TCP)
   error = up_tcp_transport_init (up, argv[1], 5150);
   if (error)
   {
      printf ("Failed to bring up transport\n");
      exit (EXIT_FAILURE);
   }
#endif

#if defined(__rtk__) && defined(OPTION_TRANSPORT_UART)
   error = up_uart_transport_init (up, argv[1]);
   if (error)
   {
      printf ("Failed to bring up transport\n");
      exit (EXIT_FAILURE);
   }
#endif

#if defined(__linux__) && defined(OPTION_TRANSPORT_UART)
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
      printf ("Failed to init rpc transport\n");
      exit (EXIT_FAILURE);
   }
#endif /* !defined(OPTION_MONO) */

   while (true)
   {

      /* Delay startup so that there is time to restore the second serial
       * connection, used for logging, before the device configuration is
       * sent to the core.
       */
      os_usleep (1000 * 1000);

#if !defined(OPTION_MONO)
      error = up_rpc_start (up, true);
      if (error)
      {
         printf ("Failed to connect to u-phy core\n");
         exit (EXIT_FAILURE);
      }
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

#ifdef ENABLE_IO_FILES
      if (first_run)
      {
         first_run = false;

         /* Generate template input file and initialize up data */
         up_util_write_input_file ("/tmp/u-phy-input.txt");
         up_util_read_input_file ("/tmp/u-phy-input.txt");

         /* Generate default status file */
         up_util_write_status_file ("/tmp/u-phy-status.txt");
      }
#endif

      if (up_start_device (up) != 0)
      {
         printf ("Failed to start device\n");
         exit (EXIT_FAILURE);
      }

#if (ENABLE_UP_COMMUNICATION_WATCHDOG == 1)
      if (up_enable_watchdog (up, true) != 0)
      {
         printf ("Failed to enable watchdog\n");
         exit (EXIT_FAILURE);
      }
#endif

      /* Write input signals to set initial values and status */
      up_write_inputs (up);

      extern bool up_worker (up_t * up);
      while (up_worker (up) == true)
         ;

      printf ("Restart application\n");
      printf ("Reset Core and reconfigure device\n");
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
   if (_cmd_start (argc, argv) != 0)
   {
      exit (EXIT_FAILURE);
   }
}

#endif
