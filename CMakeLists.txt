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

add_executable(sample
  main.c
  generated/model.c
  $<$<STREQUAL:${CMAKE_SYSTEM_NAME},Linux>:generated/eeprom.S>
  $<$<STREQUAL:${CMAKE_SYSTEM_NAME},rt-kernel>:generated/eeprom.S>
)

target_compile_options(sample
  PRIVATE
  $<$<STREQUAL:${CMAKE_SYSTEM_NAME},Linux>:-Wa,--noexecstack>
)

target_include_directories(sample
  PRIVATE
  generated
  ${UPHY_BINARY_DIR}
)

target_link_libraries(sample
  PRIVATE
  build-flags
  uphy
)

install(
  TARGETS sample
  EXPORT UphyConfig
)
