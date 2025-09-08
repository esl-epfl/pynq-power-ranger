#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <map>
#include "util.hpp"
#include "CAccelProxy.hpp"
#include "CMovingAvgProxy.hpp"
#include "xmovingavg_hw.h"

// Blocking operation: waits until accel finishes.
// Non-blocking: Returns immediately.
uint32_t CMovingAvgProxy::MovingAvg_HW(uint32_t * input, uint32_t * output, uint32_t length, bool blocking)
{
  volatile uint32_t * regs = (volatile uint32_t*)accelRegs;
  uint32_t phyInput, phyOutput;
  uint32_t status;

  if (logging)
    printf("CMovingAvgProxy::MovingAvg_HW(input=0x%08X, output=0x%08X, length=%u\n", 
          (uint32_t)input, (uint32_t)output, (uint32_t)length);

  if (accelRegs == NULL) {
    if (logging)
      printf("Error: Calling MovingAvg_HW() on a non-initialized accelerator.\n");
    return DEVICE_NOT_INITIALIZED;
  }

  // We need to obtain the physical addresses corresponding to each of the virtual addresses passed by the application.
  // The accelerator uses only the physical addresses (and only contiguous memory).
  phyInput = GetDMAPhysicalAddr(input);
  if (phyInput == 0) {
    if (logging)
      printf("Error: No physical address found for virtual address 0x%08X\n", (uint32_t)input);
    return VIRT_ADDR_NOT_FOUND;
  }
  phyOutput = GetDMAPhysicalAddr(output);
  if (phyOutput == 0) {
    if (logging)
      printf("Error: No physical address found for virtual address 0x%08X\n", (uint32_t)output);
    return VIRT_ADDR_NOT_FOUND;
  }

  *(regs + XMOVINGAVG_CONTROL_ADDR_INPUT_R_DATA/4)  = (uint32_t)phyInput;
  *(regs + XMOVINGAVG_CONTROL_ADDR_OUTPUT_R_DATA/4) = (uint32_t)phyOutput;
  *(regs + XMOVINGAVG_CONTROL_ADDR_LENGTH_R_DATA/4) = (uint32_t)length;

  //if (logging)
  //  printf("\nStarting accel...\n");

  status = *(regs + XMOVINGAVG_CONTROL_ADDR_AP_CTRL/4);
  status |= 1;  // Set to 1 ap_start
  *(regs + XMOVINGAVG_CONTROL_ADDR_AP_CTRL/4) = status;

  if (blocking) {
    do {
      status = *(regs + XMOVINGAVG_CONTROL_ADDR_AP_CTRL/4);
      usleep(1000);
    } while ( ( (status & 2) != 2) ); // wait until ap_done==1
  }

  return OK;
}


// Checks if the HW has finished.
bool CMovingAvgProxy::CheckEnd()
{
  volatile uint32_t * regs = (volatile uint32_t*)accelRegs;
  uint32_t status;
  
  if (accelRegs == NULL) {
    if (logging)
      printf("Error: Calling MovingAvg_HW() on a non-initialized accelerator.\n");
    //return DEVICE_NOT_INITIALIZED;
    return false;
  }

  status = *(regs + XMOVINGAVG_CONTROL_ADDR_AP_CTRL/4);
  return ( (status & 2) == 2); // Finihed if ap_done==1
}

