.PHONY: clean cleanall vivado_project bitstream extract_bitstream help
PROJECT_NAME := PynqPowerRanger

help:
	@echo ""
	@echo "MAKEFILE targets"
	@echo ""
	@echo "VIVADO targets"
	@echo ""
	@echo "vivado_project: Just creates the Vivado project"
	@echo "bitstream: Creates the Vivado project and runs synthesis up to bitstream generation"
	@echo "extract_bitstream: If the Vivado bitstream has already been generated, extracts it to this folder"
	@echo ""
	@echo "Generic targets"
	@echo ""
	@echo "clean: Deletes log files and Vivado projects."
	@echo "cleanall: Additionally, deletes the bitstream files"
	@echo "help: Shows this help message"
	@echo ""


vivado_project: $(PROJECT_NAME)_HW_Vivado/

$(PROJECT_NAME)_HW_Vivado/:
	vivado -mode batch -source $(PROJECT_NAME)_Vivado.tcl -tclargs --project_name $(PROJECT_NAME)_HW_Vivado

bitstream: vivado_project $(PROJECT_NAME)_HW.bit $(PROJECT_NAME)_HW.hwh

$(PROJECT_NAME)_HW.bit $(PROJECT_NAME)_HW.hwh:
	vivado -mode batch -source $(PROJECT_NAME)_Vivado_impl.tcl -tclargs --project_name $(PROJECT_NAME)_HW_Vivado
	cp ./$(PROJECT_NAME)_HW_Vivado/$(PROJECT_NAME)_HW_Vivado.runs/impl_1/design_1_wrapper.bit $(PROJECT_NAME)_HW.bit
	cp ./$(PROJECT_NAME)_HW_Vivado/$(PROJECT_NAME)_HW_Vivado.gen/sources_1/bd/design_1/hw_handoff/design_1.hwh $(PROJECT_NAME)_HW.hwh
	#cp ./$(PROJECT_NAME)_HW_Vivado/$(PROJECT_NAME)_HW_Vivado.runs/impl_1/design_1_wrapper.ltx $(PROJECT_NAME)_HW.ltx
	@md5sum $(PROJECT_NAME)_HW.bit
	@md5sum $(PROJECT_NAME)_HW.hwh


extract_bitstream:
	cp ./$(PROJECT_NAME)_HW_Vivado/$(PROJECT_NAME)_HW_Vivado.runs/impl_1/design_1_wrapper.bit $(PROJECT_NAME)_HW.bit
	cp ./$(PROJECT_NAME)_HW_Vivado/$(PROJECT_NAME)_HW_Vivado.gen/sources_1/bd/design_1/hw_handoff/design_1.hwh $(PROJECT_NAME)_HW.hwh
	#cp ./$(PROJECT_NAME)_HW_Vivado/$(PROJECT_NAME)_HW_Vivado.runs/impl_1/design_1_wrapper.ltx $(PROJECT_NAME)_HW.ltx
	@md5sum $(PROJECT_NAME)_HW.bit
	@md5sum $(PROJECT_NAME)_HW.hwh


clean:
	rm -rf NA/ .Xil
	rm -f vivado*.jou vivado*.log vivado*.str
	rm -f $(PROJECT_NAME)_HW_Vivado_def_val.txt $(PROJECT_NAME)_HW_Vivado_dump.txt
	rm -rf $(PROJECT_NAME)_HW_Vivado

cleanall: clean
	rm -f *.bit *.hwh *.ltx

