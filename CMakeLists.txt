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

# Set default options for all targets
add_compile_options(
  -Wall
  -Wextra
  -Werror
  -Wno-unused-parameter
)

add_executable(sample
  main.c
)

target_link_libraries(sample
  ${BSP}
  core
  up
)
