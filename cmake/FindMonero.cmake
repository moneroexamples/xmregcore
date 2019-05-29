#------------------------------------------------------------------------------
# CMake helper for the majority of the cpp-ethereum modules.
#
# This module defines
#     Monero_XXX_LIBRARIES, the libraries needed to use ethereum.
#     Monero_FOUND, If false, do not try to use ethereum.
#
# File addetped from cpp-ethereum
#
# The documentation for cpp-ethereum is hosted at http://cpp-ethereum.org
#
# ------------------------------------------------------------------------------
# This file is part of cpp-ethereum.
#
# cpp-ethereum is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# cpp-ethereum is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>
#
# (c) 2014-2016 cpp-ethereum contributors.
#------------------------------------------------------------------------------

#set(LIBS        common;blocks;cryptonote_basic;cryptonote_core;
#		cryptonote_protocol;daemonizer;mnemonics;epee;lmdb;device;
#                blockchain_db;ringct;wallet;cncrypto;easylogging;version;checkpoints;
#                ringct_basic)


if (NOT MONERO_DIR)
    set(MONERO_DIR ~/monero)
endif()

message(STATUS MONERO_DIR ": ${MONERO_DIR}")

set(MONERO_SOURCE_DIR ${MONERO_DIR}
        CACHE PATH "Path to the root directory for Monero")

# set location of monero build tree
set(MONERO_BUILD_DIR ${MONERO_SOURCE_DIR}/build/release/
        CACHE PATH "Path to the build directory for Monero")


set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${MONERO_BUILD_DIR}"
        CACHE PATH "Add Monero directory for library searching")


set(LIBS  cryptonote_core
          blockchain_db
          #cryptonote_protocol
          cryptonote_basic
          #daemonizer
          blocks
          lmdb
          ringct
          ringct_basic
          common
          #mnemonics
          easylogging
          device
          epee
          checkpoints
          version
          cncrypto)

set(Xmr_INCLUDE_DIRS "${CPP_MONERO_DIR}")

# if the project is a subset of main cpp-ethereum project
# use same pattern for variables as Boost uses

set(Monero_LIBRARIES "")

foreach (l ${LIBS})

	string(TOUPPER ${l} L)

	find_library(Xmr_${L}_LIBRARY
			NAMES ${l}
			PATHS ${CMAKE_LIBRARY_PATH}
                        PATH_SUFFIXES "/src/${l}"
                                      "/src/"
                                      "/external/db_drivers/lib${l}"
                                      "/lib" "/src/crypto"
                                      "/contrib/epee/src"
                                      "/external/easylogging++/"
                                      "/src/ringct/"
			NO_DEFAULT_PATH
			)

	set(Xmr_${L}_LIBRARIES ${Xmr_${L}_LIBRARY})

	message(STATUS FindMonero " Xmr_${L}_LIBRARIES ${Xmr_${L}_LIBRARY}")

    add_library(${l} STATIC IMPORTED)
	set_property(TARGET ${l} PROPERTY IMPORTED_LOCATION ${Xmr_${L}_LIBRARIES})

    set(Monero_LIBRARIES ${Monero_LIBRARIES} ${l} CACHE INTERNAL "Monero LIBRARIES")

endforeach()


message("FOUND Monero_LIBRARIES: ${Monero_LIBRARIES}")

message(STATUS ${MONERO_SOURCE_DIR}/build)

#macro(target_include_monero_directories target_name)

    #target_include_directories(${target_name}
        #PRIVATE
        #${MONERO_SOURCE_DIR}/src
        #${MONERO_SOURCE_DIR}/external
        #${MONERO_SOURCE_DIR}/build
        #${MONERO_SOURCE_DIR}/external/easylogging++
        #${MONERO_SOURCE_DIR}/contrib/epee/include
        #${MONERO_SOURCE_DIR}/external/db_drivers/liblmdb)

#endmacro(target_include_monero_directories)


add_library(Monero::Monero INTERFACE IMPORTED GLOBAL)

# Requires to new cmake
#target_include_directories(Monero::Monero INTERFACE        
    #${MONERO_SOURCE_DIR}/src
    #${MONERO_SOURCE_DIR}/external
    #${MONERO_SOURCE_DIR}/build
    #${MONERO_SOURCE_DIR}/external/easylogging++
    #${MONERO_SOURCE_DIR}/contrib/epee/include
    #${MONERO_SOURCE_DIR}/external/db_drivers/liblmdb)

set_target_properties(Monero::Monero PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES 
            "${MONERO_SOURCE_DIR}/src;${MONERO_SOURCE_DIR}/external;${MONERO_SOURCE_DIR}/build;${MONERO_SOURCE_DIR}/external/easylogging++;${MONERO_SOURCE_DIR}/contrib/epee/include;${MONERO_SOURCE_DIR}/external/db_drivers/liblmdb")


target_link_libraries(Monero::Monero INTERFACE
    ${Monero_LIBRARIES})
