source "MovingAvg_HLS.tcl"
add_files -tb HLS/testbench.cpp
csim_design -clean -O
quit
