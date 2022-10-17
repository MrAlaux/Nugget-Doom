#!/bin/bash

# Generic CMake build and install script for *nix operating systems
# version: 1.1
# Made by: Zse00
# Contact: talkwithzse@riseup.net
# Published under: The MIT License.
# Feel free to use this script with any cmake project you wish.
# It builds all projects without any issue long as it's in the same folder as the 'CMakeLists.txt' and CMake is properly installed with all of it's dependencies.
# However it can't always install them. The install portion of this script attempts to use cmake's install flag, which only works if the project supports it.
#
# HELP FOR NEWBIES: To run the script open up a terminal in the same directory as the script and type the following command:
# chmod +x ./zse00s-generic-auto-cmake-script.sh & ./zse00s-generic-auto-cmake-script.sh

# color sheet to make the interactable important stuff look apart from the cmake build output
NCO='\033[0m' # No Color
YEL='\033[3;103m\033[2;30m' # Solid Yellow Background With Black Text
RED='\033[5;101m\033[2;93m' # Flashing Yellow Text on Red Background


# building (no sudo required)
cmake -B build -S ./
cmake --build build -j`nproc --ignore=1` # this compiles with your max number of cores minus 1 in order to not nuke weaker devices while building

# installing (sudo required)
printf "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n${NCO}\n${RED}ATTENTION!${NCO}"
printf "${YEL}This step is optional and are not required for the program to work. It's entirely just for user convenience. Feel free to decline the installation below!${NCO}\n"
printf "${YEL}For testing your builds run the executable located in the build directory. No need to install it to your system. As a malfunctioning test build can be destructive!${NCO}\n"
while true; do
    read -p "Proceed with installation? [y/N] " yn
    case $yn in
        [Yy]* ) sudo cmake --install build --strip; break;;
        [Nn]* ) exit;;
        * ) exit;;
    esac
done
