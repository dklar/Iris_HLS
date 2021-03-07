############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2019 Xilinx, Inc. All Rights Reserved.
############################################################
open_project Iris
set_top top_level_normalisation
add_files Iris/cordic.hpp
add_files Iris/normalisation.hpp
add_files Iris/segmentation.hpp
add_files Iris/top_level.cpp
add_files Iris/top_level.hpp
add_files Iris/types.hpp
add_files -tb Iris/draw.py
add_files -tb Iris/test_bench.cpp
open_solution "solution1"
set_part {xc7z020-clg400-1}
create_clock -period 10 -name default
#source "./Iris/solution1/directives.tcl"
csim_design
csynth_design
cosim_design
export_design -format ip_catalog
