source "MovingAvg_HLS.tcl"
csynth_design
export_design -rtl verilog -format ip_catalog -output ./IP-catalog/MovingAvg_HLS.zip
quit
