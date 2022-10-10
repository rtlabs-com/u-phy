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

#include "model.h"

#include "options.h"
#include "osal.h" /* For NELEMENTS */

#include <stdint.h>

#undef ENABLE_IODEMO_SLOT

#undef OPTION_SAMPLE_PNET
#define OPTION_SAMPLE_ECAT
#undef OPTION_SAMPLE_MOCK

up_data_t up_data;

void * up_vars[] = {
   &up_data.i8.i8,       /* ix = 0 */
   &up_data.o8.o8,       /* ix = 1 */
   &up_data.io8.i8,      /* ix = 2 */
   &up_data.io8.o8,      /* ix = 3 */
   &up_data.io8.param_1, /* ix = 4 */
   &up_data.io8.param_2, /* ix = 5 */

#ifdef ENABLE_IODEMO_SLOT
   &up_data.iodemo.in_u8,   /* ix = 6 */
   &up_data.iodemo.in_i8,   /* ix = 7 */
   &up_data.iodemo.in_u16,  /* ix = 8 */
   &up_data.iodemo.in_i16,  /* ix = 9 */
   &up_data.iodemo.in_u32,  /* ix = 10 */
   &up_data.iodemo.in_i32,  /* ix = 11 */
   &up_data.iodemo.in_f,    /* ix = 12 */
   &up_data.iodemo.out_u8,  /* ix = 13 */
   &up_data.iodemo.out_i8,  /* ix = 14 */
   &up_data.iodemo.out_u16, /* ix = 15 */
   &up_data.iodemo.out_i16, /* ix = 16 */
   &up_data.iodemo.out_u32, /* ix = 17 */
   &up_data.iodemo.out_i32, /* ix = 18 */
   &up_data.iodemo.out_f,   /* ix = 19 */
#endif
   NULL};

static up_signal_t slot_I8_inputs[] = {
   {
      .name = "I8 Input 8 bits",
      .ix = 0,
      .datatype = UP_DTYPE_UINT8,
      .bitlength = 8,
      .frame_offset = 0,
   },
};

static up_signal_t slot_O8_outputs[] = {
   {
      .name = "O8 Output 8 bits",
      .ix = 1,
      .datatype = UP_DTYPE_UINT8,
      .bitlength = 8,
      .frame_offset = 0,
   },
};

static up_signal_t slot_IO8_inputs[] = {
   {
      .name = "IO8 Input 8 bits",
      .ix = 2,
      .datatype = UP_DTYPE_UINT8,
      .bitlength = 8,
      .frame_offset = 1,
   },
};

static up_signal_t slot_IO8_outputs[] = {
   {
      .name = "IO8 Output 8 bits",
      .ix = 3,
      .datatype = UP_DTYPE_UINT8,
      .bitlength = 8,
      .frame_offset = 1,
   },
};

static int32_t param_1_default_value = 12;
static float param_2_default_value = 1.1;

static up_param_t slot_IO8_parameters[] = {
   {
      .name = "IO8 Demo parameter int",
      .ix = 4,
      .pn_param_index = 123,
      .datatype = UP_DTYPE_INT32,
      .bitlength = 32,
      .frame_offset = 0,
      .default_value.dataLength = 4,
      .default_value.data = (void *)&param_1_default_value,
   },
   {
      .name = "IO8 Demo parameter float",
      .ix = 5,
      .pn_param_index = 124,
      .datatype = UP_DTYPE_FLOAT32,
      .bitlength = 32,
      .frame_offset = 4,
      .default_value.dataLength = 4,
      .default_value.data = (void *)&param_2_default_value,
   },
};

#ifdef ENABLE_IODEMO_SLOT
static up_signal_t slot_IODemo_inputs[] = {
   {
      .name = "Input uint8",
      .ix = 6,
      .datatype = UP_DTYPE_UINT8,
      .bitlength = 8,
      .frame_offset = 2,
   },
   {
      .name = "Input int8",
      .ix = 7,
      .datatype = UP_DTYPE_INT8,
      .bitlength = 8,
      .frame_offset = 3,
   },
   {
      .name = "Input uint16",
      .ix = 8,
      .datatype = UP_DTYPE_UINT16,
      .bitlength = 16,
      .frame_offset = 4,
   },
   {
      .name = "Input int16",
      .ix = 9,
      .datatype = UP_DTYPE_INT16,
      .bitlength = 16,
      .frame_offset = 6,
   },
   {
      .name = "Input uint32",
      .ix = 10,
      .datatype = UP_DTYPE_UINT32,
      .bitlength = 32,
      .frame_offset = 8,
   },
   {
      .name = "Input int32",
      .ix = 11,
      .datatype = UP_DTYPE_INT32,
      .bitlength = 32,
      .frame_offset = 12,
   },
   {
      .name = "Input float",
      .ix = 12,
      .datatype = UP_DTYPE_FLOAT32,
      .bitlength = 32,
      .frame_offset = 16,
   },
};

static up_signal_t slot_IODemo_outputs[] = {

   {
      .name = "Output uint8",
      .ix = 13,
      .datatype = UP_DTYPE_UINT8,
      .bitlength = 8,
      .frame_offset = 2,
   },
   {
      .name = "Output int8",
      .ix = 14,
      .datatype = UP_DTYPE_INT8,
      .bitlength = 8,
      .frame_offset = 3,
   },

   {
      .name = "Output uint16",
      .ix = 15,
      .datatype = UP_DTYPE_UINT16,
      .bitlength = 16,
      .frame_offset = 4,
   },
   {
      .name = "Output int16",
      .ix = 16,
      .datatype = UP_DTYPE_INT16,
      .bitlength = 16,
      .frame_offset = 6,
   },
   {
      .name = "Output uint32",
      .ix = 17,
      .datatype = UP_DTYPE_UINT32,
      .bitlength = 32,
      .frame_offset = 8,
   },
   {
      .name = "Output int32",
      .ix = 18,
      .datatype = UP_DTYPE_INT32,
      .bitlength = 32,
      .frame_offset = 12,
   },
   {
      .name = "Output float",
      .ix = 19,
      .datatype = UP_DTYPE_FLOAT32,
      .bitlength = 32,
      .frame_offset = 16,
   },
};
#endif

up_slot_t slots[] = {
   {
      .name = "I8",
      .profinet_module_id = 0x100,
      .profinet_submodule_id = 0x101,
      .input_bitlength = 8,
      .n_inputs = NELEMENTS (slot_I8_inputs),
      .inputs = slot_I8_inputs,
   },
   {
      .name = "O8",
      .profinet_module_id = 0x200,
      .profinet_submodule_id = 0x201,
      .output_bitlength = 8,
      .n_outputs = NELEMENTS (slot_O8_outputs),
      .outputs = slot_O8_outputs,
   },
   {
      .name = "IO8",
      .profinet_module_id = 0x300,
      .profinet_submodule_id = 0x301,
      .input_bitlength = 8,
      .output_bitlength = 8,
      .n_inputs = NELEMENTS (slot_IO8_inputs),
      .inputs = slot_IO8_inputs,
      .n_outputs = NELEMENTS (slot_IO8_outputs),
      .outputs = slot_IO8_outputs,
      .n_params = NELEMENTS (slot_IO8_parameters),
      .params = slot_IO8_parameters,
   },
#ifdef ENABLE_IODEMO_SLOT
   {
      .name = "IODemo",
      .profinet_module_id = 0x400,
      .profinet_submodule_id = 0x401,
      .input_bitlength = 144,
      .output_bitlength = 144,
      .n_inputs = NELEMENTS (slot_IODemo_inputs),
      .inputs = slot_IODemo_inputs,
      .n_outputs = NELEMENTS (slot_IODemo_outputs),
      .outputs = slot_IODemo_outputs,
   },
#endif
};

up_ciaobject_t ecat_I8_txpdo_entries[] = {{
   .index = 0x7000,
   .subindex = 0,
   .is_signal = true,
   .signal_or_param_ix = 0,
}};

up_ciapdo_t ecat_I8_txpdos[] = {{
   .name = "I8 Outputs",
   .index = 0x1600,
   .n_entries = 1,
   .entries = ecat_I8_txpdo_entries,
}};

up_ciaobject_t ecat_O8_rxpdo_entries[] = {{
   .index = 0x6000,
   .subindex = 0,
   .is_signal = true,
   .signal_or_param_ix = 0,
}};

up_ciapdo_t ecat_O8_rxpdos[] = {{
   .name = "O8 Outputs",
   .index = 0x1A00,
   .n_entries = 1,
   .entries = ecat_O8_rxpdo_entries,
}};

up_ciaobject_t ecat_IO8_txpdo_entries[] = {{
   .index = 0x7000,
   .subindex = 0,
   .is_signal = true,
   .signal_or_param_ix = 0,
}};

up_ciapdo_t ecat_IO8_txpdos[] = {{
   .name = "IO8 Inputs",
   .index = 0x1600,
   .n_entries = 1,
   .entries = ecat_IO8_txpdo_entries,
}};

up_ciaobject_t ecat_IO8_rxpdo_entries[] = {{
   .index = 0x6000,
   .subindex = 0,
   .is_signal = true,
   .signal_or_param_ix = 0,
}};

up_ciapdo_t ecat_IO8_rxpdos[] = {{
   .name = "IO8 Outputs",
   .index = 0x1A00,
   .n_entries = 1,
   .entries = ecat_IO8_rxpdo_entries,
}};

up_ciaobject_t ecat_IO8_objects[] = {
   {
      .index = 0x8000,
      .subindex = 0,
      .is_signal = false,
      .signal_or_param_ix = 0,
   },
   {
      .index = 0x8001,
      .subindex = 0,
      .is_signal = false,
      .signal_or_param_ix = 1,
   },
};

up_ecat_module_t ecat_modules[] = {
   {
      .n_rxpdos = 0,
      .n_txpdos = 1,
      .n_objects = 0,
      .rxpdos = NULL,
      .txpdos = ecat_I8_txpdos,
      .objects = NULL,
   },
   {
      .n_rxpdos = 1,
      .n_txpdos = 0,
      .n_objects = 0,
      .rxpdos = ecat_O8_rxpdos,
      .txpdos = NULL,
      .objects = NULL,
   },
   {
      .n_rxpdos = 1,
      .n_txpdos = 1,
      .n_objects = 2,
      .rxpdos = ecat_IO8_rxpdos,
      .txpdos = ecat_IO8_txpdos,
      .objects = ecat_IO8_objects,
   },
};

up_ecat_slot_t ecat_slots[] = {
   {
      .module_ix = 0,
   },
   {
      .module_ix = 1,
   },
   {
      .module_ix = 2,
   },
};

up_device_t up_device = {
   .name = "U-PHY MOD01",
   .serial_number = "20220112_A12",
#if defined(OPTION_SAMPLE_PNET)
   .bustype = UP_BUSTYPE_PROFINET,
#elif defined(OPTION_SAMPLE_ECAT)
   .bustype = UP_BUSTYPE_ECAT,
#elif defined(OPTION_SAMPLE_MOCK)
   .bustype = UP_BUSTYPE_MOCK,
#endif
   .n_slots = NELEMENTS (slots),
   .slots = slots,
};

up_busconf_t up_busconf = {
#if defined(OPTION_SAMPLE_PNET)
   .profinet.vendor_id = 0x0493,
   .profinet.device_id = 0x0003,
   .profinet.profile_id = 0x1234,
   .profinet.profile_specific_type = 0x5678,
   .profinet.min_device_interval = 32,
   .profinet.default_stationname = "u-phy-dev",
   .profinet.order_id = "MOD01",
   .profinet.hw_revision = 1,
   .profinet.sw_revision_prefix = 'V',
   .profinet.sw_revision_functional_enhancement = 0,
   .profinet.sw_revision_bug_fix = 1,
   .profinet.sw_revision_internal_change = 27,
   .profinet.revision_counter = 0,
#elif defined(OPTION_SAMPLE_ECAT)
   .ecat.profile = 0x1389,
   .ecat.vendor = 0x1337,
   .ecat.productcode = 1,
   .ecat.revision = 0x0101,
   .ecat.serial = 1,
   .ecat.hw_rev = "1.0.1",
   .ecat.sw_rev = "1.0",
   .ecat.pdo_increment = 16,
   .ecat.index_increment = 0x0100,
   .ecat.n_modules = 3,
   .ecat.n_slots = 3,
   .ecat.modules = ecat_modules,
   .ecat.slots = ecat_slots,
#elif defined(OPTION_SAMPLE_MOCK)
#endif
};
