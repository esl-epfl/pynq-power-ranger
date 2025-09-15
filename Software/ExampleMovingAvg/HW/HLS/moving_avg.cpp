#include <stdint.h>
#include <ap_int.h>

const uint32_t WINDOW_LENGTH = 7;

// We are computing the average adding on a 32-bit accumulator. To be totally correct, we should use a ~35-bit accumulator.

inline ap_uint<32> ReduceAddRegisters(ap_uint<32> buffer[WINDOW_LENGTH])
{
	ap_uint<32> res = 0;
	loop_reduceAdd: for (uint32_t ii = 0; ii < WINDOW_LENGTH; ++ ii)
	#pragma HLS UNROLL
		res += buffer[ii];
	return res;
}

void MovingAvg(ap_uint<32> *input, ap_uint<32> *output, ap_uint<32> length)
{
#pragma HLS INTERFACE s_axilite port=length
#pragma HLS INTERFACE s_axilite port=return
#pragma HLS INTERFACE m_axi depth=1024 port=input offset=slave max_read_burst_length=256 max_write_burst_length=256 latency=30 num_read_outstanding=2 num_write_outstanding=2 bundle=gmem
#pragma HLS INTERFACE m_axi depth=1024 port=output offset=slave max_read_burst_length=256 max_write_burst_length=256 latency=30 num_read_outstanding=2 num_write_outstanding=2 bundle=gmem

    ap_uint<32> sum;	// Datapath widths using arbitrary width integers
    ap_uint<32> buffer[WINDOW_LENGTH];
    uint32_t external;
    uint32_t outputIndex;

    if (length > WINDOW_LENGTH) {
		  // Fill the shift register with Window-1 elements.
		  loop_fill_buffer: for (external = 0; external < WINDOW_LENGTH - 1; ++ external) {
			  loop_shift1: for (uint32_t ss = WINDOW_LENGTH - 1; ss > 0; -- ss)
			  #pragma HLS UNROLL
				  buffer[ss] = buffer[ss-1];
			  buffer[0] = input[external];
		  }

		  outputIndex = 0;
		  loop_main: for (external = WINDOW_LENGTH - 1; external < length; ++ external) {
		  #pragma HLS PIPELINE II=1
			  loop_shift2: for (uint32_t ss = WINDOW_LENGTH - 1; ss > 0; -- ss)
			  #pragma HLS UNROLL
				  buffer[ss] = buffer[ss-1];
			  buffer[0] = input[external];
			  output[outputIndex++] = ReduceAddRegisters(buffer) / WINDOW_LENGTH;
		  }
    }
}
