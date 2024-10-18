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

static int init_rpc_transport (up_t * up, const char * cfg)
{
#if defined(OPTION_TRANSPORT_TCP)
   if (up_tcp_transport_init (up, cfg, 5150) != 0)
   {
      printf ("Failed to bring up transport\n");
      return -1;
   }
#endif

#if defined(OPTION_TRANSPORT_UART)
   if (up_serial_transport_init (up, cfg) != 0)
   {
      printf ("Failed to bring up transport\n");
      return -1;
   }
#endif

   return 0;
}

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
   char * transport = NULL;
   char * fieldbus;

   /* Check command line arguments */
   if (argc != 3)
   {
      return -1;
   }
   transport = argv[1];
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

   main_entry (up);

   return 0;
}

static char cmd_start_help_long[] =
   "Start u-phy host device.\n"
   "Usage: up_start <transport> <fieldbus>\n"
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
      printf ("Example:\n%s /dev/ttyACM0 profinet\n", argv[0]);
      exit (EXIT_FAILURE);
   }
}
