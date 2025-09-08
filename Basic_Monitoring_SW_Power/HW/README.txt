To recreate the bitstream, use the Makefile in this folder:

$ make help
  Prints make targets.
$ make vivado_project
  Generates the example Vivado project for the Pynq Z2 board, including the XADC connected to the AXI4 bus.
$ make bitstream
  Generates the Vivado project and synthesizes the bitstream.

------------------

To add the XADC module to an existing Vivado project, copy the contents of file "recreate_xadc.tcl" in Vivado's TCL console. Alternatively, run the script from the console.

------------------

The SW folder contains the SW modules necessary to read the XADC from an application running on the Pynq Z2 board.

The example application reads the instantaneous power from the XADC module and prints it on the console continuously. The application also reads the (accumulated) energy consumed since the application start.

If an application is run alongside in a different terminal, it will be possible to assess the increase of power required by the board while running that application, compared to an idle state, and the accumulated energy (since the start of the monitoring application).

-----------------
For more complex use cases, or in systems that include new hardware in the programmable logic, see the examples on integrating the XADC with a HW accelerator.




