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

#ifndef NELEMENTS
#define NELEMENTS(a) (sizeof(a) / sizeof((a)[0]))
#endif

#include <stdint.h>

up_data_t up_data;

up_signal_info_t up_vars[] = {
   {.value = (void *)&up_data.I8.Input_8_bits.value,
    .status = &up_data.I8.Input_8_bits.status},
   {.value = (void *)&up_data.O8.Output_8_bits.value,
    .status = &up_data.O8.Output_8_bits.status},
   {.value = (void *)&up_data.I8O8.Input_8_bits.value,
    .status = &up_data.I8O8.Input_8_bits.status},
   {.value = (void *)&up_data.I8O8.Output_8_bits.value,
    .status = &up_data.I8O8.Output_8_bits.status},
   {.value = (void *)&up_data.I8O8.Parameter_1,
    .status = NULL},
};

static up_signal_t inputs_I8[] = {
   {
      .name = "Input 8 bits",
      .ix = 0,
      .datatype = UP_DTYPE_UINT8,
      .bitlength = 8,
      .flags = 0,
      .frame_offset = 0,
   },
};

static up_signal_t outputs_O8[] = {
   {
      .name = "Output 8 bits",
      .ix = 1,
      .datatype = UP_DTYPE_UINT8,
      .bitlength = 8,
      .flags = 0,
      .frame_offset = 0,
   },
};

static up_signal_t inputs_I8O8[] = {
   {
      .name = "Input 8 bits",
      .ix = 2,
      .datatype = UP_DTYPE_UINT8,
      .bitlength = 8,
      .flags = 0,
      .frame_offset = 1,
   },
};

static up_signal_t outputs_I8O8[] = {
   {
      .name = "Output 8 bits",
      .ix = 3,
      .datatype = UP_DTYPE_UINT8,
      .bitlength = 8,
      .flags = 0,
      .frame_offset = 1,
   },
};

static up_param_t parameters_I8O8[] = {
   {
      .name = "Parameter 1",
      .ix = 4,
      .datatype = UP_DTYPE_UINT32,
      .bitlength = 32,
      .frame_offset = 0,
   },
};

up_slot_t slots[] = {
   {
      .name = "I8",
      .input_bitlength = 8,
      .output_bitlength = 0,
      .n_inputs = NELEMENTS (inputs_I8),
      .inputs = inputs_I8,
   },
   {
      .name = "O8",
      .input_bitlength = 0,
      .output_bitlength = 8,
      .n_outputs = NELEMENTS (outputs_O8),
      .outputs = outputs_O8,
   },
   {
      .name = "I8O8",
      .input_bitlength = 8,
      .output_bitlength = 8,
      .n_inputs = NELEMENTS (inputs_I8O8),
      .inputs = inputs_I8O8,
      .n_outputs = NELEMENTS (outputs_I8O8),
      .outputs = outputs_I8O8,
      .n_params = NELEMENTS (parameters_I8O8),
      .params = parameters_I8O8,
   },
};

up_device_t up_device = {
   .name = "U-Phy DIGIO Sample",
   .cfg.serial_number = "1234",
   .cfg.webgui_enable = true,
   .bustype = UP_BUSTYPE_MOCK,
   .n_slots = NELEMENTS (slots),
   .slots = slots,
};

up_pn_param_t pn_I8O8_parameters[] = {
   {
      .pn_index = 123,
   },
};

up_pn_module_t pn_modules[] = {
   {
      .module_id = 0x00000100,
      .submodule_id = 0x00000101,
   },
   {
      .module_id = 0x00000200,
      .submodule_id = 0x00000201,
   },
   {
      .module_id = 0x00000300,
      .submodule_id = 0x00000301,
      .n_params = 1,
      .params = pn_I8O8_parameters,
   },
};

up_pn_slot_t pn_slots[] = {
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

up_profinet_config_t up_profinet_config = {
   .vendor_id = 0x0493,
   .device_id = 0x0003,
   .dap_module_id = 0x00000001,
   .dap_identity_submodule_id = 0x00000001,
   .dap_interface_submodule_id = 0x00008000,
   .dap_port_1_submodule_id = 0x00008001,
   .dap_port_2_submodule_id = 0x00008002,
   .profile_id = 0x0000,
   .profile_specific_type = 0x0000,
   .min_device_interval = 32,
   .default_stationname = "u-phy-dev",
   .order_id = "MOD01",
   .hw_revision = 1,
   .sw_revision_prefix = 'V',
   .sw_revision_functional_enhancement = 0,
   .sw_revision_bug_fix = 1,
   .sw_revision_internal_change = 27,
   .revision_counter = 0,
   .n_modules = 3,
   .n_slots = 3,
   .modules = pn_modules,
   .slots = pn_slots,
};

up_ciaobject_t ecat_I8_Inputs_txpdo_entries[] = {
   {
      .index = 0x6000,
      .subindex = 0,
      .is_signal = true,
      .signal_or_param_ix = 0,
   },
};

up_ciapdo_t ecat_I8_txpdos[] = {
   {
      .name = "Inputs",
      .index = 0x1a00,
      .n_entries = 1,
      .entries = ecat_I8_Inputs_txpdo_entries,
   },
};

up_ciaobject_t ecat_O8_Outputs_rxpdo_entries[] = {
   {
      .index = 0x7000,
      .subindex = 0,
      .is_signal = true,
      .signal_or_param_ix = 0,
   },
};

up_ciapdo_t ecat_O8_rxpdos[] = {
   {
      .name = "Outputs",
      .index = 0x1600,
      .n_entries = 1,
      .entries = ecat_O8_Outputs_rxpdo_entries,
   },
};

up_ciaobject_t ecat_I8O8_Inputs_txpdo_entries[] = {
   {
      .index = 0x6000,
      .subindex = 0,
      .is_signal = true,
      .signal_or_param_ix = 0,
   },
};

up_ciapdo_t ecat_I8O8_txpdos[] = {
   {
      .name = "Inputs",
      .index = 0x1a00,
      .n_entries = 1,
      .entries = ecat_I8O8_Inputs_txpdo_entries,
   },
};

up_ciaobject_t ecat_I8O8_Outputs_rxpdo_entries[] = {
   {
      .index = 0x7000,
      .subindex = 0,
      .is_signal = true,
      .signal_or_param_ix = 0,
   },
};

up_ciapdo_t ecat_I8O8_rxpdos[] = {
   {
      .name = "Outputs",
      .index = 0x1600,
      .n_entries = 1,
      .entries = ecat_I8O8_Outputs_rxpdo_entries,
   },
};

up_ciaobject_t ecat_I8O8_objects[] = {
   {
      .index = 0x8000,
      .subindex = 0,
      .is_signal = false,
      .signal_or_param_ix = 0,
   },
};

up_ecat_module_t ecat_modules[] = {
   {
      .profile = 5001,
      .n_rxpdos = 0,
      .n_txpdos = 1,
      .n_objects = 0,
      .rxpdos = NULL,
      .txpdos = ecat_I8_txpdos,
      .objects = NULL,
   },
   {
      .profile = 5001,
      .n_rxpdos = 1,
      .n_txpdos = 0,
      .n_objects = 0,
      .rxpdos = ecat_O8_rxpdos,
      .txpdos = NULL,
      .objects = NULL,
   },
   {
      .profile = 5001,
      .n_rxpdos = 1,
      .n_txpdos = 1,
      .n_objects = 1,
      .rxpdos = ecat_I8O8_rxpdos,
      .txpdos = ecat_I8O8_txpdos,
      .objects = ecat_I8O8_objects,
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

up_ecat_device_t up_ethercat_config = {
   .profile = 5001,
   .vendor = 0x1337,
   .productcode = 0x1001,
   .revision = 1,
   .serial = 1,
   .hw_rev = "",
   .sw_rev = "",
   .pdo_increment = 16,       /* TODO */
   .index_increment = 0x0100, /* TODO */
   .n_modules = 3,
   .n_slots = 3,
   .modules = ecat_modules,
   .slots = ecat_slots,
};

up_ethernetip_config_t up_ethernetip_config = {
   .vendor_id = 1772,
   .device_type = 43,
   .product_code = 10,
   .major_revision = 1,
   .minor_revision = 1,
   .min_data_interval = 2000,
   .default_data_interval = 4000,
   .input_assembly_id = 100,
   .output_assembly_id = 101,
   .config_assembly_id = 102,
   .input_only_heartbeat_assembly_id = 103,
   .listen_only_heartbeat_assembly_id = 104,
};

up_mockadapter_config_t up_mock_config = {0};