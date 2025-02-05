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

#include "osal.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "shell.h"

#define APP_TASK_PRIO       5
#define APP_TASK_STACK_SIZE 6000

#if defined(OPTION_TRANSPORT_SPI) || defined (OPTION_TRANSPORT_UART)
void shield_event_isr (void * arg)
{
   up_event_ind();
}
#endif


static void main_entry (void * arg)
{
   up_t * up = (up_t *)arg;

   while (true)
   {
      if (up_rpc_start (up, true) != 0)
      {
         printf ("Failed to connect to u-phy core\n");
         exit (EXIT_FAILURE);
      }

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
   char * saveptr;
   char * scheme;
   char * transport;
   char * fieldbus;
   bool scheme_found = false;

   /* Check command line arguments */
   if (argc != 3)
   {
      shell_usage (argv[0], "wrong number of arguments");
      return -1;
   }

   scheme = strtok_r (argv[1], ":", &saveptr);
   if (scheme == NULL)
   {
      shell_usage (argv[0], "bad scheme");
      return -1;
   }

   transport = strtok_r (NULL, ":", &saveptr);
   if (transport == NULL)
   {
      shell_usage (argv[0], "bad transport");
      return -1;
   }

   fieldbus = argv[2];

   /* Set up transport */

   app_cfg.device->bustype = up_str_to_bustype (fieldbus);
   switch (app_cfg.device->bustype)
   {
#if UP_DEVICE_PROFINET_SUPPORTED
   case UP_BUSTYPE_PROFINET:
      app_busconf.profinet = up_profinet_config;
      break;
#endif
#if UP_DEVICE_ETHERCAT_SUPPORTED
   case UP_BUSTYPE_ECAT:
      app_busconf.ecat = up_ethercat_config;
      break;
#endif
#if UP_DEVICE_ETHERNETIP_SUPPORTED
   case UP_BUSTYPE_ETHERNETIP:
      app_busconf.ethernetip = up_ethernetip_config;
      break;
#endif
#if UP_DEVICE_MODBUS_SUPPORTED
   case UP_BUSTYPE_MODBUS:
      app_busconf.modbus = up_modbus_config;
      break;
#endif
   case UP_BUSTYPE_MOCK:
      app_busconf.mock = up_mock_config;
      break;
   default:
   case UP_BUSTYPE_INVALID:
      printf ("Unsupported fieldbus \"%s\", abort\n", argv[2]);
      return -1;
   }

   /* Initialise U-Phy */

   printf ("Starting sample application\n");
   up = up_init (&app_cfg);

#if defined(OPTION_TRANSPORT_TCP)
   if (strcmp (scheme, "tcp") == 0)
   {
      scheme_found = true;
      if (up_tcp_transport_init (up, transport, 5150) != 0)
      {
         printf ("Failed to bring up TCP transport\n");
         exit (EXIT_FAILURE);
      }
   }
#endif

#if defined(OPTION_TRANSPORT_SPI)
   if (strcmp (scheme, "spi") == 0)
   {
      scheme_found = true;
      if (up_spi_master_transport_init (up, transport) != 0)
      {
         printf ("Failed to bring up SPI transport\n");
         exit (EXIT_FAILURE);
      }
   }
#endif

#if defined(OPTION_TRANSPORT_UART)
   if (strcmp (scheme, "uart") == 0)
   {
      scheme_found = true;
      if (up_uart_transport_init (up, transport) != 0)
      {
         printf ("Failed to bring up UART transport\n");
         exit (EXIT_FAILURE);
      }
   }
#endif

   if (!scheme_found)
   {
      printf ("Unknown scheme %s\n", scheme);
      return -1;
   }

   if (up_rpc_init (up) != 0)
   {
      printf ("Failed to init rpc\n");
      exit (EXIT_FAILURE);
   }

   os_thread_create (
      "app_main",
      APP_TASK_PRIO,
      APP_TASK_STACK_SIZE,
      main_entry,
      up);

   return 0;
}

static char cmd_start_help_long[] =
   "Start u-phy host device.\n"
   "\nUsage: up_start <scheme:transport> <fieldbus>\n"
   "\nwhere scheme:transport can be one of:\n"
#if defined(OPTION_TRANSPORT_TCP)
   "  - tcp:<network interface>\n"
#endif
#if defined(OPTION_TRANSPORT_SPI)
   "  - spi:<spi device>\n"
#endif
#if defined(OPTION_TRANSPORT_UART)
   "  - uart:<serial device>\n"
#endif
   "\nand fieldbus can be one of:\n"
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

int main (int argc, char * argv[])
{
}
