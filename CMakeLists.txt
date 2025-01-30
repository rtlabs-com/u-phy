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

add_executable(sample
  generated/model.c
  application.c
)

# Platform configuration
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/${CMAKE_SYSTEM_NAME}.cmake)

target_include_directories(sample
  PRIVATE
  generated
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${UPHY_BINARY_DIR}
)

target_link_libraries(sample
  PRIVATE
  build-flags
  uphy
)

install(
  TARGETS sample
  EXPORT UPhyConfig
)
