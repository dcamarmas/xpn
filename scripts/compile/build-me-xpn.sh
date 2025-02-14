#!/bin/bash
#set -x

#
#  Copyright 2020-2024 Felix Garcia Carballeira, Diego Camarmas Alonso, Alejandro Calderon Mateos
#
#  This file is part of Expand.
#
#  Expand is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Lesser General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  Expand is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public License
#  along with Expand.  If not, see <http://www.gnu.org/licenses/>.
#


function usage {
    echo ""
    echo " Usage:"
    echo " $0  -m <mpicc path> -l <libfabric path> -i <Install path>"
    echo " Where:"
    echo " * <mpicc   path> = full path where the mpicc is installed."
    echo " * <libfabric   path> = full path where the libfabric is installed."
    echo " * <Install path> = full path where XPN is going to be installed."
    echo ""
}

# Start
echo ""
echo " build-me"
echo " --------"
echo ""
echo " Begin."


# 1) Arguments...

## base path
BASE_PATH="$(dirname "$(readlink -f "$0")")"
LIBFABRIC_PATH=""
## get arguments
while getopts "m:l:i:" opt; do
    case "${opt}" in
          m) MPICC_PATH=${OPTARG}
             ;;
          l) LIBFABRIC_PATH=${OPTARG}
             ;;
          i) INSTALL_PATH=${OPTARG}
             ;;
          *) echo " Error:"
             echo " * Unknown option: ${opt}"
             usage
             exit
             ;;
    esac
done

## check arguments
if [ "$MPICC_PATH" == "" ]; then
   echo " Error:"
   echo " * Empty MPICC_PATH"
   usage
   exit
fi
if [ "$INSTALL_PATH" == "" ]; then
   echo " Error:"
   echo " * Empty INSTALL_PATH"
   usage
   exit
fi


# 2) XPN and dependencies...
if [ "$LIBFABRIC_PATH" == "" ]; then
   "$BASE_PATH"/software/xpn.sh    -m "$MPICC_PATH" -i "$INSTALL_PATH" -s "$BASE_PATH"/../../../xpn
else
   "$BASE_PATH"/software/xpn.sh    -m "$MPICC_PATH" -l "$LIBFABRIC_PATH" -i "$INSTALL_PATH" -s "$BASE_PATH"/../../../xpn
fi


echo " End."
