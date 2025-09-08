#include <ap_int.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include "moving_avg.h"

const uint32_t MAX_PRINT_LENGTH = 32;

const uint32_t LENGTH = 16;
const uint32_t WINDOW_LENGTH = 7;

ap_uint<32> input[LENGTH], output[LENGTH], outputSW[LENGTH];

///////////////////////////////////////////////////////////////////////////////
void PrintVector(ap_uint<32> * vector, uint32_t length)
{
    for (uint32_t ii = 0; ii < length; ++ii)
        printf("%u ", (uint32_t)vector[ii]);
    printf("\n");
}

///////////////////////////////////////////////////////////////////////////////
void PrintVectors(ap_uint<32> * input, ap_uint<32> * output, ap_uint<32> * outputSW, ap_uint<32> length)
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
void InitVector(ap_uint<32> * v, ap_uint<32> length, ap_uint<32> value, bool autoInc = false, bool random = false)
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
void ComputeSW(ap_uint<32> * input, ap_uint<32> * output, ap_uint<32> length)
{
	for (uint32_t external = 0; external < (LENGTH - WINDOW_LENGTH + 1); ++ external) {
		uint32_t accum;
		accum = 0;
		for (uint32_t internal = 0; internal < WINDOW_LENGTH; ++ internal) {
			accum += input[external + internal];
		}
		output[external] = accum / WINDOW_LENGTH;
	}
}

///////////////////////////////////////////////////////////////////////////////
uint32_t CompareVectors(ap_uint<32> * a, ap_uint<32> * b, ap_uint<32> length)
{
	uint32_t numErrors = 0;

	for (uint32_t ii = 0; ii < length; ++ ii) {
		if (a[ii] != b[ii])
			++ numErrors;
	}

	return numErrors;
}

///////////////////////////////////////////////////////////////////////////////
int main(int, char **)
{
	uint32_t numErrors;
	srand(time(NULL));

	// TEST: Fixed value to 1.
	printf("\n======\n");
	InitVector(input, LENGTH, 1);
	memset(output, 0, LENGTH * sizeof(ap_uint<32>));
	memset(outputSW, 0, LENGTH * sizeof(ap_uint<32>));
	MovingAvg(input, output, LENGTH);
	ComputeSW(input, outputSW, LENGTH);
	numErrors = CompareVectors(output, outputSW, LENGTH); // Compare also the zeroes at the end of the vector
	if (numErrors)
		printf("Test of value=1: %u Errors!\n", numErrors);
	else
		printf("Test of value=1: OK!\n");
	PrintVectors(input, output, outputSW, LENGTH);

	// TEST: Fixed value to 2.
	printf("\n======\n");
	InitVector(input, LENGTH, 2);
	memset(output, 0, LENGTH * sizeof(ap_uint<32>));
	memset(outputSW, 0, LENGTH * sizeof(ap_uint<32>));
	MovingAvg(input, output, LENGTH);
	ComputeSW(input, outputSW, LENGTH);
	numErrors = CompareVectors(output, outputSW, LENGTH); // Compare also the zeroes at the end of the vector
	if (numErrors)
		printf("Test of value=2: %u Errors!\n", numErrors);
	else
		printf("Test of value=2: OK!\n");
	PrintVectors(input, output, outputSW, LENGTH);

	// TEST: Value = index.
	printf("\n======\n");
	InitVector(input, LENGTH, 0, true);
	memset(output, 0, LENGTH * sizeof(ap_uint<32>));
	memset(outputSW, 0, LENGTH * sizeof(ap_uint<32>));
	MovingAvg(input, output, LENGTH);
	ComputeSW(input, outputSW, LENGTH);
	numErrors = CompareVectors(output, outputSW, LENGTH); // Compare also the zeroes at the end of the vector
	if (numErrors)
		printf("Test of value=index: %u Errors!\n", numErrors);
	else
		printf("Test of value=index: OK!\n");
	PrintVectors(input, output, outputSW, LENGTH);

	// TEST: Random values.
	printf("\n======\n");
	InitVector(input, LENGTH, 0, false, true);
	memset(output, 0, LENGTH * sizeof(ap_uint<32>));
	memset(outputSW, 0, LENGTH * sizeof(ap_uint<32>));
	MovingAvg(input, output, LENGTH);
	ComputeSW(input, outputSW, LENGTH);
	numErrors = CompareVectors(output, outputSW, LENGTH); // Compare also the zeroes at the end of the vector
	if (numErrors)
		printf("Test of value=random: %u Errors!\n", numErrors);
	else
		printf("Test of value=random: OK!\n");
	PrintVectors(input, output, outputSW, LENGTH);

    return 0;
}
