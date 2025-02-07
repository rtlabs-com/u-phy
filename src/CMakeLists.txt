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

include(UPhyModel)

add_executable(sample
  application.c
)

target_model(sample
  ${PROJECT_SOURCE_DIR}/models/${OPTION_MODEL}
  OUTPUT_DIR
  ${PROJECT_SOURCE_DIR}/generated
)

target_include_directories(sample
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(sample
  PRIVATE
  build-flags
  uphy
)

# Platform configuration
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/${CMAKE_SYSTEM_NAME}.cmake)
