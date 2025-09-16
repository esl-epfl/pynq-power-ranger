open_project MovingAvg_HLS
set_top MovingAvg
add_files HLS/moving_avg.cpp
add_files HLS/moving_avg.h
add_files -tb HLS/testbench.cpp
open_solution "solution1" -flow_target vivado
set_part {xc7z020clg400-1}
create_clock -period 10 -name default
set_clock_uncertainty 1
config_export -display_name MovingAverage -format ip_catalog -output ./IP-catalog/MovingAvg_HLS.zip -rtl verilog -vendor EPFL -vivado_clock 10
config_interface -m_axi_addr64=0
