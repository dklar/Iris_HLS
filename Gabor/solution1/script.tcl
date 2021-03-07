############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2019 Xilinx, Inc. All Rights Reserved.
############################################################
open_project Gabor
set_top top_gabor
add_files Iris/types.hpp
add_files Gabor/top_gabor.cpp
add_files Gabor/gabor.hpp
add_files Gabor/cordic.hpp
open_solution "solution1"
set_part {xc7vx485t-ffg1157-1}
create_clock -period 10 -name default
#source "./Gabor/solution1/directives.tcl"
#csim_design
csynth_design
#cosim_design
export_design -format ip_catalog
