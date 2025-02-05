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

#include <stdio.h>

/* For POSIX compatibility */
#define strtok_r strtok_s

static void main_entry (up_t * up)
{
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
      return -1;
   }

   scheme = strtok_r (argv[1], ":", &saveptr);
   if (scheme == NULL)
      return -1;

   transport = strtok_r (NULL, ":", &saveptr);
   if (transport == NULL)
      return -1;

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
         return -1;
   }
   }
#endif

#if defined(OPTION_TRANSPORT_UART)
   if (strcmp (scheme, "uart") == 0)
   {
      scheme_found = true;
      if (up_serial_transport_init (up, transport) != 0)
      {
         printf ("Failed to bring up UART transport\n");
         return -1;
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

   main_entry (up);

   return 0;
}

static char cmd_start_help_long[] =
   "Start u-phy host device.\n"
   "\nUsage: up_start <scheme:transport> <fieldbus>\n"
   "\nwhere scheme:transport can be one of:\n"
#if defined(OPTION_TRANSPORT_TCP)
   "  - tcp:<network interface>\n"
#endif
#if defined(OPTION_TRANSPORT_UART)
   "  - uart:<serial port>\n"
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

int main (int argc, char * argv[])
{
   setvbuf (stdout, NULL, _IONBF, 0);
   if (_cmd_start (argc, argv) != 0)
   {
      puts (cmd_start_help_long);
      printf ("Example:\n%s uart:COM1 profinet\n", argv[0]);
      exit (EXIT_FAILURE);
   }
}
