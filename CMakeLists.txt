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

cmake_minimum_required(VERSION 3.28)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
project(SAMPLE VERSION 1.0.1)

if(NOT TARGET uphy)
  find_package(UPhy REQUIRED)
endif()

set(OPTION_MODEL "digio.json" CACHE STRING "U-Phy model file")

add_subdirectory(src)
