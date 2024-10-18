#********************************************************************
#        _       _         _
#  _ __ | |_  _ | |  __ _ | |__   ___
# | '__|| __|(_)| | / _` || '_ \ / __|
# | |   | |_  _ | || (_| || |_) |\__ \
# |_|    \__|(_)|_| \__,_||_.__/ |___/
#
# www.rt-labs.com
# Copyright 2024 rt-labs AB, Sweden.
# See LICENSE file in the project root for full license information.
#*******************************************************************/

enable_language(ASM)

option(ENABLE_IO_FILES "" ON)

target_sources(sample
  PRIVATE
  generated/eeprom.S
  $<$<BOOL:${OPTION_MONO}>:ports/linux/mono.c>
  $<$<NOT:$<BOOL:${OPTION_MONO}>>:ports/linux/client.c>
)

target_compile_definitions(sample
  PRIVATE
  $<$<BOOL:${ENABLE_IO_FILES}>:ENABLE_IO_FILES=1>
)

target_compile_options(sample
  PRIVATE
  $<$<STREQUAL:${CMAKE_SYSTEM_NAME},Linux>:-Wa,--noexecstack>
)
