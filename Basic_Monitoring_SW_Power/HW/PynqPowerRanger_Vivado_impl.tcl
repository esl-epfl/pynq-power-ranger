open_project PynqPowerRanger_HW_Vivado/PynqPowerRanger_HW_Vivado.xpr
update_compile_order -fileset sources_1

launch_runs impl_1 -to_step write_bitstream -jobs 10
wait_on_run impl_1

