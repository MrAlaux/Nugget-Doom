#!/bin/bash


# building
cmake -B build -S ./
cmake --build build #-j5 #<--- insert your cpu cores+1 in there for faster compile times
# installing
sudo cmake --install build --strip
