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

#include "up_api.h"

extern up_busconf_t app_busconf; /**< Active fieldbus configuration */
extern up_cfg_t app_cfg;         /**< Application device configuration */

/**
 * Application entry point
 *
 * @param up            u-phy state
 */
void app_main (up_t * up);
