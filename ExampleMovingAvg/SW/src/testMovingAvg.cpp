#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>
#include <stdint.h>
#include <inttypes.h>
#include <map>
#include <unistd.h>

#include "CAccelProxy.hpp"
#include "CXADCProxy.hpp"
#include "CMovingAvgProxy.hpp"
#include "util.hpp"

const uint32_t MAX_PRINT_LENGTH = 32;
const uint32_t LENGTH = 8000000;
const uint32_t WINDOW_LENGTH = 7;
const uint32_t REPETITIONS = 100;

// Defines whether the vector memory is cacheable by the processors or not.
const uint32_t MEM_IS_CACHEABLE = 0;

///////////////////////////////////////
// Address constants
const uint32_t MAP_SIZE = 64*1024;
const uint32_t MOVINGAVG_HW_ADDR = 0x40000000; // From Vivado's address editor
const uint32_t XADC_HW_ADDR = 0x44A00000; // From Vivado's address editor

const uint32_t MAX_POWER_VALUES = 10000000;
float powerValues[MAX_POWER_VALUES];
uint64_t timestamps[MAX_POWER_VALUES];

uint32_t iPower = 0; // Number of valid power entries.


// These vectors are for testing the speed in SW - Not used with the accelerator.
// The reason is that the processor would normally use cacheable memory thru the virtual memory system, not the DMA allocated for the device.
uint32_t inputSW[LENGTH], outputSW[LENGTH];

///////////////////////////////////////////////////////////////////////////////
bool InitXADCDevice(CXADCProxy & xadc, bool log=false)
{
    if ( xadc.Open(XADC_HW_ADDR, MAP_SIZE) != CAccelProxy::OK ) {
        printf("Error mapping device at physical address 0x%08X\n", XADC_HW_ADDR);
        return false;
    }
    if (log)
        printf("Device at physical address 0x%08X successfully mapped into the application virtual address space\n\n",
            XADC_HW_ADDR);

    return true;
}


///////////////////////////////////////////////////////////////////////////////
bool InitDevice(CMovingAvgProxy & movingAvgAccel, uint32_t * &input, uint32_t * &output, uint32_t length, bool log=true)
{
  if ( movingAvgAccel.Open(MOVINGAVG_HW_ADDR, MAP_SIZE) != CAccelProxy::OK ) {
    printf("Error mapping device at physical address 0x%08X\n", MOVINGAVG_HW_ADDR);
    return false;
  }
  if (log)
    printf("Device at physical address 0x%08X successfully mapped into the application virtual address space\n\n",
            MOVINGAVG_HW_ADDR);

  // Allocate DMA memory for use by the device. We receive addresses in the *virtual* address space of the application.
  if (log)
    printf("Allocating DMA memory...\n");

  input = (uint32_t *)movingAvgAccel.AllocDMACompatible(length * sizeof(uint32_t));
  output = (uint32_t *)movingAvgAccel.AllocDMACompatible(length * sizeof(uint32_t));

  if ( (input == NULL) || (output == NULL) ) {
    printf("Error allocating DMA memory.\n");
    return false;
  }
 
  if (log) {
    printf("DMA memory allocated.\n");
    printf("Input: Virtual address: 0x%08X (%u)\n", (uint32_t)input, (uint32_t)input);
    printf("Output: Virtual address: 0x%08X (%u)\n", (uint32_t)output, (uint32_t)output);
  }

  return true;
}


///////////////////////////////////////////////////////////////////////////////
//  In this example, instead than defining in this code the offset with an enumerated type,
// we use the macros defined by Vitis HLS. These macros refer to absolute offsets, so 
// we have to divide them by 4 to use uint32_t pointer arithmetic.
bool CallAccel(CMovingAvgProxy & movingAvgAccel, uint32_t *input, uint32_t *output, uint32_t length,
              uint64_t & elapsed, CXADCProxy & powerRanger, double & energy)
{
  struct timespec start, end;
  struct timespec now;
  double energyStart, energyEnd;
  iPower = 0;

  energyStart = powerRanger.GetEnergy();
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);
  
  for (uint32_t ii = 0; ii < REPETITIONS; ++ ii) {
    movingAvgAccel.MovingAvg_HW(input, output, length, false);
    while ( !movingAvgAccel.CheckEnd() ) {
      if (iPower < MAX_POWER_VALUES) {
        clock_gettime(CLOCK_MONOTONIC_RAW, &now);
        timestamps[iPower] = CalcTimeDiff(now, start);
        powerValues[iPower++] = powerRanger.GetLastPower();
      }
      usleep(1000);
    }
  }

  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  energyEnd = powerRanger.GetEnergy();
  elapsed = CalcTimeDiff(end, start);
  energy = energyEnd - energyStart;

  return true;
}

///////////////////////////////////////////////////////////////////////////////
void PrintVector(uint32_t * vector, uint32_t length)
{
  for (uint32_t ii = 0; ii < length; ++ii)
    printf("%u ", vector[ii]);
  printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
void PrintVectors(uint32_t * input, uint32_t * output, uint32_t * outputSW, uint32_t length)
{
  if (length < MAX_PRINT_LENGTH) {
    printf("Input:\n");
    PrintVector(input, length);
    printf("Output:\n");
    PrintVector(output, length);
    printf("OutputSW:\n");
    PrintVector(outputSW, length);
  } else {
    printf("Vectors too long to be printed.\n");
  }
}

///////////////////////////////////////////////////////////////////////////////
void InitVector(uint32_t * v, uint32_t length, uint32_t value, bool autoInc = false, bool random = false)
{
  if (autoInc) {
    for (uint32_t ii = 0; ii < length; ++ii)
      v[ii] = ii;
  } 
  else if (random) {
    for (uint32_t ii = 0; ii < length; ++ii)
      v[ii] = rand();
  }
  else {
    for (uint32_t ii = 0; ii < length; ++ii)
      v[ii] = value;
  }
}

///////////////////////////////////////////////////////////////////////////////
void ComputeSW(uint32_t * input, uint32_t * output, uint32_t length, 
            uint64_t & elapsed, CXADCProxy & powerRanger, double & energy)
{
  struct timespec start, end;
  double energyStart, energyEnd;

  energyStart = powerRanger.GetEnergy();
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);

  for (uint32_t ii = 0; ii < REPETITIONS; ++ ii) {
	  for (uint32_t external = 0; external < (LENGTH - WINDOW_LENGTH + 1); ++ external) {
		  uint32_t accum;
		  accum = 0;
		  for (uint32_t internal = 0; internal < WINDOW_LENGTH; ++ internal) {
			  accum += input[external + internal];
		  }
		  output[external] = accum / WINDOW_LENGTH;
	  }
  }

  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  energyEnd = powerRanger.GetEnergy();
  elapsed = CalcTimeDiff(end, start);
  energy = energyEnd - energyStart;
}

///////////////////////////////////////////////////////////////////////////////
void ComputeSWFast(uint32_t * input, uint32_t * output, uint32_t length,
            uint64_t & elapsed, CXADCProxy & powerRanger, double & energy)
{
  struct timespec start, end;
  double energyStart, energyEnd;
  uint32_t external, outputIndex;

  energyStart = powerRanger.GetEnergy();
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);

  for (uint32_t ii = 0; ii < REPETITIONS; ++ ii) {
    uint32_t accum = 0;
    // Fill the accum with the first WINDOW_LENGTH - 1 values
    for (external = 0; external < WINDOW_LENGTH - 1; ++ external)
      accum += input[external];

    // Traverse the rest of the elements, adding the new, subtracting the oldest.
    outputIndex = 0;
    for (external = WINDOW_LENGTH - 1; external < length; ++ external) {
      accum += input[external];
      output[outputIndex++] = accum / WINDOW_LENGTH;
      accum -= input[external - (WINDOW_LENGTH - 1)]; // This operation is not associative bc it's unsigned.
    }
  }

  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  energyEnd = powerRanger.GetEnergy();
  elapsed = CalcTimeDiff(end, start);
  energy = energyEnd - energyStart;
}

///////////////////////////////////////////////////////////////////////////////
uint32_t CompareVectors(uint32_t * a, uint32_t * b, uint32_t length)
{
	uint32_t numErrors = 0;

	for (uint32_t ii = 0; ii < length; ++ ii) {
		if (a[ii] != b[ii])
			++ numErrors;
	}

	return numErrors;
}


///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
  uint64_t timeAccel, timeSWSlow, timeSWFast;
  double energyAccel, energySWSlow, energySWFast;
	uint32_t numErrors;
  uint32_t * input, * output; // Pointers to memory allocated for the accelerator.

  // Verify that we are running with sudo.
  if (geteuid()) {
    printf("\n\n###### This program has to be run with sudo!!! ######\n\n\n");
    return -1;
  }

  printf("\n\nThis program requires that the bitstream is loaded in the FPGA.\n");
  printf("Press ENTER to confirm that the bitstream is loaded (proceeding without it can crash the board).\n\n");
  getchar();

  setlocale(LC_NUMERIC, "en_US.utf8");  // Enables printing human-readable numbers with %'
  srand(time(NULL));

  CXADCProxy powerRanger(false); // Deactivate logging
  if (!InitXADCDevice(powerRanger))
    return -1;
  powerRanger.StartMeasurements();

  // Initialize moving average accelerator and obtain memory for all the data arrays.
  CMovingAvgProxy movingAvgAccel(false); // Deactivate logging
  if (!InitDevice(movingAvgAccel, input, output, LENGTH))
    return -1;

 	// TEST: Random values.
  printf("\n====================\nTest of value=random\n\n");
 	InitVector(input, LENGTH, 0, false, true);
  memcpy(inputSW, input, LENGTH * sizeof(uint32_t));
 	memset(output, 0, LENGTH * sizeof(uint32_t));
 	memset(outputSW, 0, LENGTH * sizeof(uint32_t));
  printf("HW version...\n");
  CallAccel(movingAvgAccel, input, output, LENGTH, timeAccel, powerRanger, energyAccel);
  printf("SW (slow) version...\n");
 	ComputeSW(inputSW, outputSW, LENGTH, timeSWSlow, powerRanger, energySWSlow);
  printf("SW (fast) version...\n");
 	ComputeSWFast(inputSW, outputSW, LENGTH, timeSWFast, powerRanger, energySWFast);
 	numErrors = CompareVectors(output, outputSW, LENGTH); // Compare also the zeroes at the end of the vector
  if (numErrors)
	  printf("---> %u Errors!\n", numErrors);
 	else
  	printf("---> OK!\n");
	//PrintVectors(input, output, outputSW, LENGTH);
  printf("SW (slow) --> Time: %0.3lf s (%llu ns) - Energy: %0.3lf J\n", (timeSWSlow/1e9)/REPETITIONS, timeSWSlow/REPETITIONS, energySWSlow/REPETITIONS);
  printf("SW (fast) --> Time: %0.3lf s (%llu ns) - Energy: %0.3lf J\n", (timeSWFast/1e9)/REPETITIONS, timeSWFast/REPETITIONS, energySWFast/REPETITIONS);
  printf("HW Accel  --> Time: %0.3lf s (%llu ns) - Energy: %0.3lf J\n", (timeAccel/1e9)/REPETITIONS, timeAccel/REPETITIONS, energyAccel/REPETITIONS);
  printf("HW speed-up: %0.1lf X - HW energy: %0.1lf X\n\n", ((double)timeSWFast) / timeAccel, energyAccel / energySWFast);
  printf("Power values captured: %u\n", iPower);

  // Uncomment this to output a CSV table to stderr with all the power measurements and their timestamps.
  /*fprintf(stderr, "Time (ns), Power (W)\n");
  float avg = 0;
  for (uint32_t ii = 0; ii < iPower; ++ ii) {
    fprintf(stderr, "%llu, %0.3lf\n", timestamps[ii], powerValues[ii]);
    avg += powerValues[ii];
  }*/
  //avg = avg / iPower;
  //fprintf(stderr, "Current power: %0.3lf\n", powerRanger.GetLastPower());
  //fprintf(stderr, "Average power: %0.3lf\n", avg);

  powerRanger.StopMeasurements();

  // Free the DMA memory. ---IMPORTANT--- DMA memory is a system-wide resource!!!!!! It's not freed automatically when the app is closed.
  if (input != NULL)
    movingAvgAccel.FreeDMACompatible(input);
  if (output != NULL)
    movingAvgAccel.FreeDMACompatible(output);

  return 0;
}

