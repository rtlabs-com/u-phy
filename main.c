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

#define STORAGE_ROOT "/disk1" /* No trailing slash */
#define APP_TASK_PRIO       5
#define APP_TASK_STACK_SIZE 6000

#endif /* defined(__rtk__) */

#if defined(__linux__)
#define ENABLE_IO_FILES
#endif /* defined(__linux__) */

/* Enable to run synchronous operation mode */
#undef APPLICATION_MODE_SYNCHRONOUS

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

#if defined(__rtk__)
#if defined(OPTION_TRANSPORT_SPI) || defined (OPTION_TRANSPORT_UART)
void shield_event_isr (void * arg)
{
   up_event_ind();
}
#endif
#endif

static void cb_avail (up_t * up, void * user_arg)
{
   up_read_outputs (up);
}

static void cb_sync (up_t * up, void * user_arg)
{
   up_write_inputs (up);
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
   /* Ignore */
}

static int str_to_bus_config (const char * str, up_bustype_t * bustype)
{
   if (str == NULL || bustype == NULL)
   {
      return -1;
   }

#if UP_DEVICE_PROFINET_SUPPORTED
   if (strcmp (str, "profinet") == 0)
   {
      *bustype = UP_BUSTYPE_PROFINET;
      return 0;
   }
#endif

#if UP_DEVICE_ETHERCAT_SUPPORTED
   if (strcmp (str, "ethercat") == 0)
   {
      *bustype = UP_BUSTYPE_ECAT;
      return 0;
   }
#endif

#if UP_DEVICE_ETHERNETIP_SUPPORTED
   if (strcmp (str, "ethernetip") == 0)
   {
      *bustype = UP_BUSTYPE_ETHERNETIP;
      return 0;
   }
#endif
   if (strcmp (str, "mock") == 0)
   {
      *bustype = UP_BUSTYPE_MOCK;
      return 0;
   }

   printf ("Unsupported fieldbus \"%s\". Is it supported by device model?\n", str);
   return -1;
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

static void cb_loop_ind (up_t * up, void * user_arg)
{
#if defined(ENABLE_IO_FILES)
   up_util_read_input_file ("/tmp/u-phy-input.txt");
#endif

#if !defined(APPLICATION_MODE_SYNCHRONOUS)
   up_write_inputs (up);
   up_read_outputs (up);
#endif

#if defined(ENABLE_IO_FILES)
   up_util_write_status_file ("/tmp/u-phy-status.txt");
   up_util_poll_cmd_file ("/tmp/u-phy-command.txt");
#endif
}

/**
 * Initialize utility files.
 * - Generate template input file and initialize up data
 *  - Generate default status file
 */
#if defined(ENABLE_IO_FILES)
static void init_util_files (void)
{
   up_util_write_input_file ("/tmp/u-phy-input.txt");
   up_util_read_input_file ("/tmp/u-phy-input.txt");
   up_util_write_status_file ("/tmp/u-phy-status.txt");
}
#endif

#if !defined(OPTION_MONO)
static int init_rpc_transport (up_t * up, const char * cfg)
{
#if defined(OPTION_TRANSPORT_TCP)
   if (up_tcp_transport_init (up, cfg, 5150) != 0)
   {
      printf ("Failed to bring up transport\n");
      return -1;
   }
#endif

#if defined(__rtk__) && defined(OPTION_TRANSPORT_SPI)
   if (up_spi_master_transport_init (up, cfg) != 0)
   {
      printf ("Failed to bring up transport\n");
      return -1;
   }
#endif

#if defined(__rtk__) && defined(OPTION_TRANSPORT_UART)
   if (up_uart_transport_init (up, cfg) != 0)
   {
      printf ("Failed to bring up transport\n");
      return -1;
   }
#endif

#if defined(__linux__) && defined(OPTION_TRANSPORT_UART)
   if (up_serial_transport_init (up, cfg) != 0)
   {
      printf ("Failed to bring up transport\n");
      return -1;
   }
#endif

   return 0;
}
#endif /* !defined(OPTION_MONO) */

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
   .poll_ind = cb_loop_ind,
};

void up_app_main (void * arg)
{
   up_t * up = (up_t *)arg;
#if defined(ENABLE_IO_FILES)
   bool first_run = true;
#endif

   /* Delay startup so that there is time to restore the second serial
    * connection, used for logging, before the device configuration is
    * sent to the core.
    */
   os_usleep (2000 * 1000);

   while (true)
   {
#if !defined(OPTION_MONO)
      if (up_rpc_start (up, true) != 0)
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
         if (
            up_write_ecat_eeprom (
               up,
               &_eeprom_bin_start,
               &_eeprom_bin_end - &_eeprom_bin_start) != 0)
         {
            printf ("Failed to write EtherCAT eeprom \n");
            return;
         }
      }

      if (up_util_init (&up_device, up, up_vars) != 0)
      {
         printf ("Failed to init up utils\n");
         exit (EXIT_FAILURE);
      }

#if defined(ENABLE_IO_FILES)
      if (first_run)
      {
         first_run = false;
         init_util_files();
      }
#endif

      if (up_start_device (up) != 0)
      {
         printf ("Failed to start device\n");
         exit (EXIT_FAILURE);
      }

#if defined(APPLICATION_MODE_SYNCHRONOUS)
      if (up_write_event_mask(up, UP_EVENT_MASK_SYNCHRONOUS_MODE) != 0)
      {
         printf ("Failed to write eventmask mode\n");
         exit (EXIT_FAILURE);
      }
#endif

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
}

int _cmd_start (int argc, char * argv[])
{
   up_t * up;
#if !defined(OPTION_MONO)
   char * transport = NULL;
#endif
   char * fieldbus;

   /* Check command line arguments */
#if defined(OPTION_MONO)
   if (argc != 2)
   {
      printf ("error - try \"help %s\n", argv[0]);
      return -1;
   }
   fieldbus = argv[1];
#else  /* defined(OPTION_MONO) */
   if (argc != 3)
   {
      printf ("usage: %s <transport cfg> <fieldbus>\n", argv[0]);
      printf ("The <fieldbus> can be profinet, ethercat, ethernetip or mock\n");
      printf ("example: %s /dev/ttyUSB0 profinet\n", argv[0]);
      return -1;
   }
   transport = argv[1];
   fieldbus = argv[2];
#endif /* defined(OPTION_MONO) */

   /* Initialise U-Phy and set up transport */

#ifdef OPTION_MONO
   extern void up_core_init (void);
   up_core_init();
#endif

   if (str_to_bus_config (fieldbus, &cfg.device->bustype) == 0)
   {
      switch (cfg.device->bustype)
      {
      case UP_BUSTYPE_PROFINET:
#if UP_DEVICE_PROFINET_SUPPORTED
         up_busconf.profinet = up_profinet_config;
#endif
         break;
      case UP_BUSTYPE_ECAT:
#if UP_DEVICE_ETHERCAT_SUPPORTED
         up_busconf.ecat = up_ethercat_config;
#endif
         break;
      case UP_BUSTYPE_ETHERNETIP:
#if UP_DEVICE_ETHERNETIP_SUPPORTED
         up_busconf.ethernetip = up_ethernetip_config;
#endif
         break;
      case UP_BUSTYPE_MOCK:
         up_busconf.mock = up_mock_config;
         break;
      }
   }
   else
   {
      printf ("Unsupported fieldbus \"%s\", abort\n", argv[2]);
      return -1;
   }

   printf ("Starting sample application\n");
   up = up_init (&cfg);

#if !defined(OPTION_MONO)

   if (init_rpc_transport (up, transport) != 0)
   {
      printf ("Failed to init rpc transport\n");
      exit (EXIT_FAILURE);
   }

   if (up_rpc_init (up) != 0)
   {
      printf ("Failed to init rpc\n");
      exit (EXIT_FAILURE);
   }

#endif /* !defined(OPTION_MONO) */

#if defined(__rtk__)
   os_thread_create (
      "up_app_main",
      APP_TASK_PRIO,
      APP_TASK_STACK_SIZE,
      up_app_main,
      up);
#else
   up_app_main (up);
#endif

   return 0;
}

#if defined(__rtk__)

const shell_cmd_t cmd_start = {
   .cmd = _cmd_start,
   .name = "up_start",
   .help_short = "start u-phy device",
   .help_long =
#if defined(OPTION_MONO)
      "Start monolithic u-phy device including core and device model.\n"
      "Usage: up_start <fieldbus>\n"
      "where fieldbus can be ethercat, profinet or mock\n"};
#else
      "Start u-phy host device.\n"
      "Usage: up_start <transport> <fieldbus>\n"
      "where fieldbus can be ethercat, profinet or mock\n"};
#endif

SHELL_CMD (cmd_start);

#if defined(OPTION_MONO)
int _cmd_autostart (int argc, char * argv[])
{
   up_bustype_t bustype;
   char * fieldbus;

   if (argc != 2)
   {
      printf ("error - try \"help %s\n", argv[0]);
      return -1;
   }

   fieldbus = argv[1];
   if ((argc == 2) && (str_to_bus_config (fieldbus, &bustype) == 0))
   {
      int f = open (STORAGE_ROOT "/autostart", O_WRONLY | O_CREAT);
      write (f, fieldbus, strlen (fieldbus) + 1);
      close (f);

      printf ("%s autostart added\n", fieldbus);
   }
   else
   {
      printf ("autostart disabled\n");

      unlink (STORAGE_ROOT "/autostart");
      return -1;
   }

   return 0;
}

const shell_cmd_t cmd_autostart = {
   .cmd = _cmd_autostart,
   .name = "up_autostart",
   .help_short = "configure u-phy device autostart",
   .help_long = "Set u-phy autostart configuration \n"
                "Usage: up_autostart <fieldbus>\n"
                "where fieldbus can be ethercat | profinet | mock.\n"
                "If no valid fieldbus is given, autostart is disabled."};

SHELL_CMD (cmd_autostart);

/**
 * Read auto start configuration file and start the
 * device configuration by calling the start command.
 *
 * @return 0 if a device configuration was started.
 *         -1 if no device configuration exists or on error
 */
int auto_start (void)
{
   up_bustype_t bustype;
   char buf[32];
   int f = open (STORAGE_ROOT "/autostart", O_RDONLY);
   if (f > 0)
   {
      read (f, buf, sizeof (buf));
      close (f);

      if (str_to_bus_config (buf, &bustype) == 0)
      {
         char * argv[2] = {"up_start", buf};
         return _cmd_start (2, argv);
      }
   }
   return -1;
}

#endif /* defined(OPTION_MONO) */

int main (int argc, char * argv[])
{
#if defined(OPTION_MONO)
   if (auto_start() != 0)
   {
      extern void core_reconfigure_net_if (void);
      core_reconfigure_net_if();
   }
#endif
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
