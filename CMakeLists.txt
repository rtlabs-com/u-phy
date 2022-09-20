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

enable_language(ASM)

# Set default options for all targets
add_compile_options(
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
)
endif()
