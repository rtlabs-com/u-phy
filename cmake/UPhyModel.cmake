#********************************************************************
#        _       _         _
#  _ __ | |_  _ | |  __ _ | |__   ___
# | '__|| __|(_)| | / _` || '_ \ / __|
# | |   | |_  _ | || (_| || |_) |\__ \
# |_|    \__|(_)|_| \__,_||_.__/ |___/
#
# www.rt-labs.com
# Copyright 2023 rt-labs AB, Sweden.
#
# This software is licensed under the terms of the BSD 3-clause
# license. See the file LICENSE distributed with this software for
# full license information.
#*******************************************************************/

find_program(UPGEN NAMES upgen PATHS ${CMAKE_BINARY_DIR}/bin)
find_program(UV NAMES uv PATHS ${CMAKE_BINARY_DIR}/bin)

if(NOT UPGEN AND NOT UV)
  message(STATUS "uv binary not found, downloading and installing...")

  set(UV_SCRIPT_URL "https://github.com/astral-sh/uv/releases/download/0.5.20")

  # Download the shell script
  file(DOWNLOAD ${UV_SCRIPT_URL} ${UV_SCRIPT})

  if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(UV_SCRIPT_URL "${UV_SCRIPT_URL}/uv-installer.ps1")
    set(UV_SCRIPT ${CMAKE_BINARY_DIR}/uv-installer.ps1)

    # Download the shell script
    file(DOWNLOAD ${UV_SCRIPT_URL} ${UV_SCRIPT})

    # Run the installation script
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E env
      UV_UNMANAGED_INSTALL=bin
      powershell ${UV_SCRIPT}
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
  else()
    set(UV_SCRIPT_URL "${UV_SCRIPT_URL}/uv-installer.sh")
    set(UV_SCRIPT ${CMAKE_BINARY_DIR}/uv-installer.sh)

    # Download the shell script
    file(DOWNLOAD ${UV_SCRIPT_URL} ${UV_SCRIPT})

    # Make the script executable
    execute_process(COMMAND chmod +x ${UV_SCRIPT})

    # Run the installation script
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E env
      UV_UNMANAGED_INSTALL=bin
      ${UV_SCRIPT} -q
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
  endif()

  find_program(UV NAMES uv PATHS ${CMAKE_BINARY_DIR}/bin REQUIRED)
endif()

if (NOT UPGEN)
  message(STATUS "upgen binary not found, downloading and installing...")

  # Run the uv tool installer
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
    UV_PRERELEASE=allow
    UV_TOOL_DIR=${CMAKE_BINARY_DIR}/tools
    UV_TOOL_BIN_DIR=${CMAKE_BINARY_DIR}/bin
    ${UV} tool install -q uphy-upgen # ~=1.0.0
  )

  find_program(UPGEN NAMES upgen PATHS ${CMAKE_BINARY_DIR}/bin REQUIRED)
endif()

function(target_model target model)
  set(flags)
  set(args OUTPUT_DIR)
  set(listArgs)

  cmake_parse_arguments(arg "${flags}" "${args}" "${listArgs}" ${ARGN})

  if (arg_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "target_model: ${_arg_UNPARSED_ARGUMENTS}: unexpected arguments")
  endif()

  if (NOT arg_OUTPUT_DIR)
    message(FATAL_ERROR "target_model: OUTPUT_DIR is a required argument")
  endif()

  add_custom_command (
    OUTPUT
    ${arg_OUTPUT_DIR}/model.c
    ${arg_OUTPUT_DIR}/model.h
    ${arg_OUTPUT_DIR}/eeprom.bin
    DEPENDS ${model}
    COMMAND ${UPGEN} -d ${arg_OUTPUT_DIR} export ${model}
    VERBATIM
  )

  target_sources(${target}
    PRIVATE
    ${arg_OUTPUT_DIR}/model.c
  )

  target_include_directories(${target}
    PRIVATE
    ${arg_OUTPUT_DIR}/
  )

endfunction()
