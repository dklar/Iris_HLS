############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2019 Xilinx, Inc. All Rights Reserved.
############################################################
open_project Gabor
set_top top_level_Gabor_FIX
add_files Gabor/src/gabor.hpp
add_files Gabor/src/top_level.cpp
add_files Gabor/src/toplevel.hpp
add_files -tb Gabor/tb/test_bench.cpp
open_solution "solution1"
set_part {xc7vx485t-ffg1157-1}
create_clock -period 10 -name default
#source "./Gabor/solution1/directives.tcl"
csim_design -clean
csynth_design
cosim_design
export_design -format ip_catalog
