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

#include <inttypes.h>
#include <stdio.h>

/* Enable synchronous operation mode. In this mode, the device data is
   updated synchronously to the fieldbus cycle. If not enabled, the
   device data updates asynchronously to the fieldbus cycle
   (free-running mode). */
#ifndef APPLICATION_MODE_SYNCHRONOUS
#define APPLICATION_MODE_SYNCHRONOUS 0
#endif

/* Enable watchdog to detect U-Phy communication failures. The sample
   application enables the watchdog by default.  Set
   ENABLE_UP_COMMUNICATION_WATCHDOG to 0 to keep the watchdog
   disabled. */
#ifndef ENABLE_UP_COMMUNICATION_WATCHDOG
#define ENABLE_UP_COMMUNICATION_WATCHDOG 1
#endif

/* Enable feature to watch outputs / set inputs and to interactively
   control U-Phy. This feature is currently available on Linux
   only. */
#ifndef ENABLE_IO_FILES
#define ENABLE_IO_FILES 0
#endif

static void get_inputs (void * user_arg)
{
   /* Use this function to read inputs (sensors) */
#if 0
   int status = read_sensor (&up_data.I8.Input_8_bits.value);
   up_data.I8.Input_8_bits.status = (status == SUCCESS ? UP_STATUS_OK : 0);
#endif

#if ENABLE_IO_FILES
   up_util_read_input_file ("/tmp/u-phy-input.txt");
#endif
}

static void set_outputs (void * user_arg)
{
   /* Use this function to set outputs (actuators) */
#if 0
   if (up_data.O8.Output_8_bits.status & UP_STATUS_OK)
   {
      set_actuator (up_data.O8.Output_8_bits.value);
   }
#endif

#if ENABLE_IO_FILES
   up_util_write_status_file ("/tmp/u-phy-status.txt");
   up_util_poll_cmd_file ("/tmp/u-phy-command.txt");
#endif
}

static void cb_avail (up_t * up, void * user_arg)
{
   /* Called when core has received outputs from
      controller. Synchronous mode only. */

   /* Receive outputs from fieldbus controller */
   up_read_outputs (up);

   /* Activate outputs */
   set_outputs (user_arg);
}

static void cb_sync (up_t * up, void * user_arg)
{
   /* Called when core is about to send inputs to
      controller. Synchronous mode only. */

   /* Latch inputs */
   get_inputs (user_arg);

   /* Send inputs to fieldbus controller */
   up_write_inputs (up);
}

static void cb_param_write_ind (up_t * up, void * user_arg)
{
   uint16_t slot_ix;
   uint16_t param_ix;
   binary_t data;
   up_param_t * p;

   /* Called when controller requests write to a parameter */

   while (up_param_get_write_req (up, &slot_ix, &param_ix, &data) == 0)
   {
      p = &up_device.slots[slot_ix].params[param_ix];
      memcpy (up_vars[p->ix].value, data.data, data.dataLength);
      free (data.data);
   }
}

static void cb_status_ind (up_t * up, uint32_t status, void * user_arg)
{
   /* Called when device status changes */
}

static void cb_error_ind (up_t * up, up_error_t error_code, void * user_arg)
{
   /* Called when device error occurs */
   printf (
      "ERROR: error_code=%" PRIi16 " %s\n",
      error_code,
      up_error_to_str (error_code));
}

static void cb_profinet_signal_led_ind (up_t * up, void * user_arg)
{
   /* Called when profinet controller requests LED indication. */
   printf ("Flash Profinet signal LED for 3s at 1Hz\n");
}

static void cb_loop_ind (up_t * up, void * user_arg)
{
   /* Called every 100 ms. Used to implement free-running (i.e. not
      synchronous) mode.  */

#if !(APPLICATION_MODE_SYNCHRONOUS)
   /* Read and activate outputs */
   up_read_outputs (up);
   set_outputs (user_arg);

   /* Latch and write inputs */
   get_inputs (user_arg);
   up_write_inputs (up);
#endif
}

/**
 * Initialize utility files.
 * - Generate template input file and initialize up data
 *  - Generate default status file
 */
#if ENABLE_IO_FILES
static void init_util_files (void)
{
   up_util_write_input_file ("/tmp/u-phy-input.txt");
   up_util_read_input_file ("/tmp/u-phy-input.txt");
   up_util_write_status_file ("/tmp/u-phy-status.txt");
}
#endif

up_busconf_t app_busconf;
up_cfg_t app_cfg =
{
   .device = &up_device,
   .busconf = &app_busconf,
   .vars = up_vars,
   .sync = cb_sync,
   .avail = cb_avail,
   .param_write_ind = cb_param_write_ind,
   .status_ind = cb_status_ind,
   .error_ind = cb_error_ind,
   .profinet_signal_led_ind = cb_profinet_signal_led_ind,
   .poll_ind = cb_loop_ind,
   .cb_arg = NULL,
};

void app_main (up_t * up)
{
#if ENABLE_IO_FILES
   static bool first_run = true;
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

#if APPLICATION_MODE_SYNCHRONOUS
   if (up_write_event_mask(up, UP_EVENT_MASK_SYNCHRONOUS_MODE) != 0)
   {
      printf ("Failed to write eventmask mode\n");
      exit (EXIT_FAILURE);
   }
#endif

#if ENABLE_UP_COMMUNICATION_WATCHDOG
   if (up_enable_watchdog (up, true) != 0)
   {
      printf ("Failed to enable watchdog\n");
      exit (EXIT_FAILURE);
   }
#endif

   /* Latch and write input signals to set initial values and status */
   get_inputs (app_cfg.cb_arg);
   up_write_inputs (up);

   while (up_worker (up) == true)
      ;
}
