#********************************************************************
#        _       _         _
#  _ __ | |_  _ | |  __ _ | |__   ___
# | '__|| __|(_)| | / _` || '_ \ / __|
# | |   | |_  _ | || (_| || |_) |\__ \
# |_|    \__|(_)|_| \__,_||_.__/ |___/
#
# www.rt-labs.com
# Copyright 2022 rt-labs AB, Sweden.
# See LICENSE file in the project root for full license information.
#*******************************************************************/

if(NOT WINDOWS_MONO)
enable_language(ASM)

# Set default options for all targets
target_compile_options(erpc
  PRIVATE
  -Wall
  -Wextra
  -Werror
  -Wno-unused-parameter
)

add_executable(sample
  main.c
  generated/model.c
  generated/eeprom.S
  $<$<BOOL:${OPTION_MONO}>:${CMAKE_CURRENT_SOURCE_DIR}/../core/core.c>
)

else()

# Set default options for all targets
target_compile_options(erpc
  PRIVATE
  /Wall
  /WX
  /wd4100 # unreferenced parameter
  /wd4242 # conversion from __int64 to uint16_t
  )

  add_executable(sample
  main.c
  generated/model.c
  $<$<BOOL:${OPTION_MONO}>:${CMAKE_CURRENT_SOURCE_DIR}/../core/core.c>
  )

  target_compile_options(sample
  PRIVATE
  /D_CRT_NONSTDC_NO_WARNINGS # * deprecated; use _*
  /Wall
  /WX
  /wd4061 # enumerator in switch not explicitly handled
  /wd4127 # conditional expression is constant
  /wd4100 # unreferenced parameter
  /wd4210 # function fiven file scope (extern)
  /wd4242 # conversion from uint16_t to uint8_t
  /wd4244 # conversion from __int64 to uint16_t
  /wd4668 # ERPC_ALLOCATION_POLICY* is not defined; replacing with 0
  /wd4820 # padding added
  /wd5045 # compiler will insert Spectre mitigation if /Qspectre specified
)

endif()

if(LINUX_MONO AND OPTION_ADAPTER_PNET)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/../p-net/src/ports/linux/set_network_parameters
  ${CMAKE_CURRENT_BINARY_DIR}/set_network_parameters
  COPYONLY
)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/../p-net/src/ports/linux/set_profinet_leds
  ${CMAKE_CURRENT_BINARY_DIR}/set_profinet_leds
  COPYONLY
)
endif()

target_include_directories(sample
  PRIVATE
  generated
  ${CMAKE_BINARY_DIR}
  $<$<BOOL:${OPTION_MONO}>:${CMAKE_CURRENT_BINARY_DIR}/../core/>
)

target_link_libraries(sample
  ${BSP}
  up
)

if(OPTION_MONO)
target_link_libraries(sample
  osal
  $<$<BOOL:${OPTION_HTTPD}>:httpd>
  $<$<BOOL:${OPTION_ADAPTER_MOCK}>:mock_adapter>
  $<$<BOOL:${OPTION_ADAPTER_PNET}>:pnadapter>
  $<$<BOOL:${OPTION_ADAPTER_ECAT}>:ecat>
  $<$<BOOL:${OPTION_ADAPTER_ENIP}>:enip_adapter>
  $<$<BOOL:${OPTION_ADAPTER_MBUS}>:mbus_adapter>
)
endif()
