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

target_sources(sample
  PRIVATE
  $<$<BOOL:${OPTION_MONO}>:ports/windows/mono.c>
  $<$<NOT:$<BOOL:${OPTION_MONO}>>:ports/windows/client.c>
)
