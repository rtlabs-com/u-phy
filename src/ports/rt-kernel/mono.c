/*********************************************************************
 *        _       _         _
 *  _ __ | |_  _ | |  __ _ | |__   ___
 * | '__|| __|(_)| | / _` || '_ \ / __|
 * | |   | |_  _ | || (_| || |_) |\__ \
 * |_|    \__|(_)|_| \__,_||_.__/ |___/
 *
 * http://www.rt-labs.com
 * Copyright 2024 rt-labs AB, Sweden.
 * See LICENSE file in the project root for full license information.
 ********************************************************************/

#include "application.h"
#include "options.h"
#include "up_api.h"
#include "up_util.h"
#include "model.h"
#include "core.h"

#include "osal.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "shell.h"

#define STORAGE_ROOT        "/disk1" /* No trailing slash */
#define APP_TASK_PRIO       5
#define APP_TASK_STACK_SIZE 6000

static void main_entry (void * arg)
{
   up_t * up = (up_t *)arg;

   while (true)
   {
      if (up_init_device (up) != 0)
      {
         printf ("Failed to configure device\n");
         exit (EXIT_FAILURE);
      }

#if defined (UP_DEVICE_ETHERCAT_SUPPORTED)
      if (app_cfg.device->bustype == UP_BUSTYPE_ECAT)
      {
         /* Start and end tags for the generated EtherCAT SII eeprom.
          * Defined in eeprom.S.
          */
         extern const uint8_t _eeprom_bin_start;
         extern const uint8_t _eeprom_bin_end;

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
#endif

      if (up_util_init (&up_device, up, up_vars) != 0)
      {
         printf ("Failed to init up utils\n");
         exit (EXIT_FAILURE);
      }

      app_main (up);

      printf ("Restart application\n");
      printf ("Reset Core and reconfigure device\n");
   }
}

static int _cmd_start (int argc, char * argv[])
{
   up_t * up;
   char * fieldbus;

   /* Check command line arguments */
   if (argc != 2)
   {
      shell_usage (argv[0], "wrong number of arguments");
      return -1;
   }
   fieldbus = argv[1];

   app_cfg.device->bustype = up_str_to_bustype (fieldbus);
   switch (app_cfg.device->bustype)
   {
   case UP_BUSTYPE_PROFINET:
#if UP_DEVICE_PROFINET_SUPPORTED
      app_busconf.profinet = up_profinet_config;
#endif
      break;
   case UP_BUSTYPE_ECAT:
#if UP_DEVICE_ETHERCAT_SUPPORTED
      app_busconf.ecat = up_ethercat_config;
#endif
      break;
   case UP_BUSTYPE_ETHERNETIP:
#if UP_DEVICE_ETHERNETIP_SUPPORTED
      app_busconf.ethernetip = up_ethernetip_config;
#endif
      break;
   case UP_BUSTYPE_MODBUS:
#if UP_DEVICE_MODBUS_SUPPORTED
      app_busconf.modbus = up_modbus_config;
#endif
      break;
   case UP_BUSTYPE_MOCK:
      app_busconf.mock = up_mock_config;
      break;
   case UP_BUSTYPE_INVALID:
      printf ("Unsupported fieldbus \"%s\", abort\n", argv[2]);
      return -1;
   }

   printf ("Starting sample application\n");
   up = up_init (&app_cfg);

   os_thread_create (
      "app_main",
      APP_TASK_PRIO,
      APP_TASK_STACK_SIZE,
      main_entry,
      up);

   return 0;
}

static char cmd_start_help_long[] =
   "Start monolithic u-phy device including core and device model.\n"
   "Usage: up_start <fieldbus>\n"
   "where fieldbus can be one of:\n"
#if UP_DEVICE_ETHERCAT_SUPPORTED
   "  - ethercat\n"
#endif
#if UP_DEVICE_PROFINET_SUPPORTED
   "  - profinet\n"
#endif
#if UP_DEVICE_ETHERNETIP_SUPPORTED
   "  - ethernetip\n"
#endif
#if UP_DEVICE_MODBUS_SUPPORTED
   "  - modbus\n"
#endif
   "  - mock\n";

static const shell_cmd_t cmd_start = {
   .cmd = _cmd_start,
   .name = "up_start",
   .help_short = "start u-phy device",
   .help_long = cmd_start_help_long
};

SHELL_CMD (cmd_start);

static int _cmd_autostart (int argc, char * argv[])
{
   up_bustype_t bustype = UP_BUSTYPE_INVALID;
   char * fieldbus;

   if (argc < 2)
   {
      shell_usage (argv[0], "wrong number of arguments");
      return -1;
   }

   fieldbus = argv[1];
   if (argc == 2)
   {
      bustype = up_str_to_bustype (fieldbus);
   }

   if (bustype != UP_BUSTYPE_INVALID)
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

static const shell_cmd_t cmd_autostart = {
   .cmd = _cmd_autostart,
   .name = "up_autostart",
   .help_short = "configure u-phy device autostart",
   .help_long =
      "Set u-phy autostart configuration \n"
      "Usage: up_autostart <fieldbus>\n"
      "where fieldbus can be one of:\n"
#if UP_DEVICE_ETHERCAT_SUPPORTED
      "  - ethercat\n"
#endif
#if UP_DEVICE_PROFINET_SUPPORTED
      "  - profinet\n"
#endif
#if UP_DEVICE_ETHERNETIP_SUPPORTED
      "  - ethernetip\n"
#endif
#if UP_DEVICE_MODBUS_SUPPORTED
      "  - modbus\n"
#endif
      "  - mock\n"
      "\n"
      "If no valid fieldbus is given, autostart is disabled.\n"
};

SHELL_CMD (cmd_autostart);

/**
 * Read auto start configuration file and start the
 * device configuration by calling the start command.
 *
 * @return 0 if a device configuration was started.
 *         -1 if no device configuration exists or on error
 */
static int auto_start (void)
{
   up_bustype_t bustype;
   char buf[32];
   int f = open (STORAGE_ROOT "/autostart", O_RDONLY);
   if (f > 0)
   {
      read (f, buf, sizeof (buf));
      close (f);

      bustype = up_str_to_bustype (buf);
      if (bustype != UP_BUSTYPE_INVALID)
      {
         char * argv[2] = {"up_start", buf};
         return _cmd_start (2, argv);
      }
   }
   return -1;
}

int main (int argc, char * argv[])
{
   /* Initialise U-Phy */
   up_core_init();

   if (auto_start() != 0)
   {
      core_bringup_network();
   }
}
