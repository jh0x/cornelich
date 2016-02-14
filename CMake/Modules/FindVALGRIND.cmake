# NOTE: Copied from the opencog project, where it is distributed under the
#       terms of the New BSD License.

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 
# 1. Redistributions of source code must retain the copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Find Valgrind.
#
# This module defines:
#  VALGRIND_INCLUDE_DIR, where to find valgrind/memcheck.h, etc.
#  VALGRIND_PROGRAM, the valgrind executable.
#  VALGRIND_FOUND, If false, do not try to use valgrind.
#
# If you have valgrind installed in a non-standard place, you can define
# VALGRIND_PREFIX to tell cmake where it is.
#

message(STATUS "Valgrind Prefix: ${VALGRIND_PREFIX}")

find_path(VALGRIND_INCLUDE_DIR memcheck.h
  /usr/include /usr/include/valgrind /usr/local/include /usr/local/include/valgrind
  ${VALGRIND_PREFIX}/include ${VALGRIND_PREFIX}/include/valgrind)
find_program(VALGRIND_PROGRAM NAMES valgrind PATH /usr/bin /usr/local/bin ${VALGRIND_PREFIX}/bin)

find_package_handle_standard_args(VALGRIND DEFAULT_MSG
    VALGRIND_INCLUDE_DIR
    VALGRIND_PROGRAM)

mark_as_advanced(VALGRIND_INCLUDE_DIR VALGRIND_PROGRAM)

