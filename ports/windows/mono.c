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

extern void up_core_init (void);
extern void core_set_interface (char * iface, size_t size);
extern void core_bringup_network (void);

static void main_entry (up_t * up)
{
   while (true)
   {
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
   char * fieldbus;

   /* Check command line arguments */
   if (argc != 3)
   {
      return -1;
   }
   core_set_interface (argv[2], strlen (argv[2]));
   fieldbus = argv[1];

   /* Initialise U-Phy */

   up_core_init();

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

   main_entry (up);

   return 0;
}

static char cmd_start_help_long[] =
   "Start monolithic u-phy device including core and device model.\n"
   "Usage: up_start <fieldbus> <network interface>\n"
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

int main (int argc, char * argv[])
{
   setvbuf (stdout, NULL, _IONBF, 0);
   if (_cmd_start (argc, argv) != 0)
   {
      puts (cmd_start_help_long);
      printf ("Example:\n%s profinet Ethernet\n", argv[0]);
      exit (EXIT_FAILURE);
   }
}
