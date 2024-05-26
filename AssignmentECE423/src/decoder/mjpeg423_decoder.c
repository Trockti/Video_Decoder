//
//  mjpeg423_decoder.c
//  mjpeg423app
//
//  Created by Sonia Navas and Javier Madrid
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../common/mjpeg423_types.h"
#include "mjpeg423_decoder.h"
#include "../common/util.h"
#include  "ff.h"
#include "xtime_l.h"
#include "xil_cache.h"
#include "xil_cache_l.h"
#include "../spinlock1.h"
#include "../spinlockAdd.h"

#define NUM_BUF 2
//#define TIMING_COUNTER

XTime start;
XTime end;

int profile = 1;
rgb_pixel_t* *ADDRESS = (rgb_pixel_t* *)RGB_ADDRESS;


static TimingData *timing;
static SpaceData *outsize;

 UINTPTR *spinlock_start = ( UINTPTR *)SPINLOCK_ADDR1;
int *CC_ADD = (int *)CC_SEM;
//*CC_ADD = 0;
void mjpeg423_decode_init(FIL *file_in, file_data_struct *file_data)
{

//	DEBUG_PRINT_ARG("ALO %u\n",CC_ADD)
	XTime_GetTime(&start);
	XTime_GetTime(&end);
	timing -> null_time = end - start;
    if(f_read(file_in , (void*)&file_data->num_frames, sizeof( uint32_t ) , (UINT*)&file_data->NumBytesRead) != FR_OK ) error_and_exit("cannot read input file");
    DEBUG_PRINT_ARG("Decoder start. Num frames #%u\n", file_data->num_frames);
    // frame width
    if(f_read(file_in , (void*)&file_data->w_size, sizeof( uint32_t ) , (UINT*)&file_data->NumBytesRead) != FR_OK) error_and_exit("cannot read input file");
    DEBUG_PRINT_ARG("Width %u\n", file_data->w_size);
    // frame height
    if(f_read(file_in , (void*)&file_data->h_size, sizeof( uint32_t ) , (UINT*)&file_data->NumBytesRead) != FR_OK) error_and_exit("cannot read input file");
    DEBUG_PRINT_ARG("Height %u\n", file_data->h_size);
    // #i-frames
    if(f_read(file_in , (void*)&file_data->num_iframes, sizeof( uint32_t ) , (UINT*)&file_data->NumBytesRead) != FR_OK ) error_and_exit("cannot read input file");
    DEBUG_PRINT_ARG("Num i frames %u\n", file_data->num_iframes);
    // #payload
    if(f_read(file_in , (void*)&file_data->payload_size, sizeof( uint32_t ) , (UINT*)&file_data->NumBytesRead) != FR_OK ) error_and_exit("cannot read input file");

    file_data->hCb_size = file_data->h_size/8;           //number of chrominance blocks
    file_data->wCb_size = file_data->w_size/8;
    file_data->hYb_size = file_data->h_size/8;           //number of luminance blocks. Same as chrominance in the sample app
    file_data->wYb_size = file_data->w_size/8;


    //trailer structure
    file_data->trailer = malloc(sizeof(iframe_trailer_t)*file_data->num_iframes);

    //main data structures. See lab manual for explanation
    // color_block_t* Yblock;
    if((file_data->Yblock = malloc(file_data->hYb_size * file_data->wYb_size * 64))==NULL) error_and_exit("cannot allocate Yblock");
    // color_block_t* Cbblock;
    if((file_data->Cbblock = malloc(file_data->hCb_size * file_data->wCb_size * 64))==NULL) error_and_exit("cannot allocate Cbblock");
    // color_block_t* Crblock;
    if((file_data->Crblock = malloc(file_data->hCb_size * file_data->wCb_size * 64))==NULL) error_and_exit("cannot allocate Crblock");;
    // dct_block_t* YDCAC;
    if((file_data->YDCAC = malloc(file_data->hYb_size * file_data->wYb_size * 64 * sizeof(DCTELEM)))==NULL) error_and_exit("cannot allocate YDCAC");
    // dct_block_t* CbDCAC;
    if((file_data->CbDCAC = malloc(file_data->hCb_size * file_data->wCb_size * 64 * sizeof(DCTELEM)))==NULL) error_and_exit("cannot allocate CbDCAC");
    // dct_block_t* CrDCAC;
    if((file_data->CrDCAC = malloc(file_data->hCb_size * file_data->wCb_size * 64 * sizeof(DCTELEM)))==NULL) error_and_exit("cannot allocate CrDCAC");
    //Ybitstream is assigned a size sufficient to hold all bistreams
    //the bitstream is then read from the file into Ybitstream
    //the remaining pointers simply point to the beginning of the Cb and Cr streams within Ybitstream
    // uint8_t* Ybitstream;
    if((file_data->Ybitstream = malloc(file_data->hYb_size * file_data->wYb_size * 64 * sizeof(DCTELEM) + 2 * file_data->hCb_size * file_data->wCb_size * 64 * sizeof(DCTELEM)))==NULL) error_and_exit("cannot allocate bitstream");
    // uint8_t* Cbbitstream;
    // uint8_t* Crbitstream;

    //read trailer. Note: the trailer information is not used in the sample decoder app
    //set file to beginning of trailer
    if(f_lseek(file_in, 5 * sizeof(uint32_t) + file_data->payload_size) != FR_OK) error_and_exit("cannot seek into file");
    for(int count = 0; count < file_data->num_iframes; count++){
    	if(f_read(file_in,(void*)&(file_data->trailer[count].frame_index), sizeof(uint32_t),(UINT*)&file_data->NumBytesRead ) != FR_OK) error_and_exit("cannot read input file");
    	if(f_read(file_in,(void*)&(file_data->trailer[count].frame_position), sizeof(uint32_t),(UINT*)&file_data->NumBytesRead ) != FR_OK) error_and_exit("cannot read input file");
    	DEBUG_PRINT_ARG("I frame index %u, ", file_data->trailer[count].frame_index)
    	DEBUG_PRINT_ARG("position %u\n", file_data->trailer[count].frame_position)
    }
    //set it back to beginning of payload
    if(f_lseek(file_in,5 * sizeof(uint32_t)) != FR_OK) error_and_exit("cannot seek into file");

    file_data->frame_index = 0;
}

void read_frame(FIL *file_in, file_data_struct *file_data, rgb_pixel_t* rgbblock){
	DEBUG_PRINT_ARG("frame index %d\n", file_data->frame_index)
	    uint32_t Ysize, Cbsize, frame_size, frame_type;
        //read frame payload

		#ifdef TIMING_COUNTER
			XTime_GetTime(&start);
		#endif
		if(f_read(file_in , (void*)&frame_size, sizeof( uint32_t ) , (UINT*)&file_data->NumBytesRead) != FR_OK) error_and_exit("cannot read input file");
        if(f_read(file_in , (void*)&frame_type, sizeof( uint32_t ) , (UINT*)&file_data->NumBytesRead) != FR_OK) error_and_exit("cannot read input file");

        if(f_read(file_in , (void*)&Ysize, sizeof( uint32_t ) , (UINT*)&file_data->NumBytesRead) != FR_OK) error_and_exit("cannot read input file");
        if(f_read(file_in , (void*)&Cbsize, sizeof( uint32_t ) , (UINT*)&file_data->NumBytesRead) != FR_OK) error_and_exit("cannot read input file");
        if(f_read(file_in , (void*)file_data->Ybitstream,  frame_size - 4 * sizeof(uint32_t) , (UINT*)&file_data->NumBytesRead) != FR_OK) error_and_exit("cannot read input file");
		#ifdef TIMING_COUNTER
			XTime_GetTime(&end);
			if (frame_type == 0){
				timing -> sd_read_frames_P_time[file_data->frame_index] = 0;
				timing -> sd_read_frames_I_time[file_data->frame_index] = end - start - timing -> null_time;
			}
			else{
				timing -> sd_read_frames_I_time[file_data->frame_index] = 0;
				timing -> sd_read_frames_P_time[file_data->frame_index] = end - start - timing -> null_time;
			}
		#endif
        #ifdef SIZE_PROF
            if (frame_type == 0){
            	outsize -> sd_read_frames_P_size[file_data->frame_index] = 0;
            	outsize -> sd_read_frames_I_size[file_data->frame_index] = frame_size;
			}
			else{
				outsize -> sd_read_frames_I_size[file_data->frame_index] = 0;
				outsize -> sd_read_frames_P_size[file_data->frame_index] = frame_size;
			}
        #endif
        //set the Cb and Cr bitstreams to point to the right location
        file_data->Cbbitstream = file_data->Ybitstream + Ysize;
        file_data->Crbitstream = file_data->Cbbitstream + Cbsize;

        //lossless decoding

		#ifdef TIMING_COUNTER
			XTime_GetTime(&start);
		#endif
		lossless_decode(file_data->hYb_size*file_data->wYb_size, file_data->Ybitstream, file_data->YDCAC, Yquant, frame_type);
		#ifdef TIMING_COUNTER
			XTime_GetTime(&end);
			if (frame_type == 0){
				timing -> lossless_Y_I_time[file_data->frame_index] = end - start - timing -> null_time;
			}
			else{
				timing -> lossless_Y_P_time[file_data->frame_index] = end - start - timing -> null_time;
			}
		#endif
		#ifdef SIZE_PROF
			outsize -> lossless_Y_one_frame_time[file_data->frame_index] = Ysize;
			DEBUG_PRINT_ARG("Size of lossless_Y_one_frame_time: %lu\n", outsize -> lossless_Y_one_frame_time[file_data->frame_index]);

		#endif

        #ifdef TIMING_COUNTER
			XTime_GetTime(&start);
		#endif
//        lossless_decode(file_data->hCb_size*file_data->wCb_size, file_data->Cbbitstream, file_data->CbDCAC, Cquant, frame_type);
		#ifdef TIMING_COUNTER
			XTime_GetTime(&end);
			if (frame_type == 0){
				timing -> lossless_Cb_I_time[file_data->frame_index] = end - start - timing -> null_time;
			}
			else{
				timing -> lossless_Cb_P_time[file_data->frame_index] = end - start - timing -> null_time;
			}
		#endif
		#ifdef SIZE_PROF
			outsize -> lossless_Cb_one_frame_time[file_data->frame_index] =  Cbsize;
			DEBUG_PRINT_ARG("Size of lossless_Cb_one_frame_time: %lu\n", outsize -> lossless_Cb_one_frame_time[file_data->frame_index]);
		#endif
        #ifdef TIMING_COUNTER
			XTime_GetTime(&start);
		#endif
//        lossless_decode(file_data->hCb_size*file_data->wCb_size, file_data->Crbitstream, file_data->CrDCAC, Cquant, frame_type);
		#ifdef TIMING_COUNTER
			XTime_GetTime(&end);
			if (frame_type == 0){
				timing -> lossless_Cr_I_time[file_data->frame_index] = end - start - timing -> null_time;
			}
			else{
				timing -> lossless_Cr_P_time[file_data->frame_index] = end - start - timing -> null_time;
			}
		#endif
		#ifdef SIZE_PROF
			outsize -> lossless_Cr_one_frame_time[file_data->frame_index] = frame_size - 4*sizeof( uint32_t ) - Cbsize - Ysize;
			DEBUG_PRINT_ARG("Size of lossless_Cr_one_frame_time: %lu\n", outsize -> lossless_Cr_one_frame_time[file_data->frame_index]);

		#endif
        //fdct
		#ifdef TIMING_COUNTER
			XTime_GetTime(&start);
		#endif
		Xil_L1DCacheFlush();
		#ifdef TIMING_COUNTER
			XTime_GetTime(&end);
			timing -> flush_cache_L1[file_data->frame_index] = end - start - timing -> null_time;
		#endif
		#ifdef TIMING_COUNTER
			XTime_GetTime(&start);
		#endif
		Xil_L2CacheFlush();
		#ifdef TIMING_COUNTER
			XTime_GetTime(&end);
			timing -> flush_cache_L2[file_data->frame_index] = end - start - timing -> null_time;
		#endif


		#ifdef TIMING_COUNTER
			XTime_GetTime(&start);
		#endif
        #ifdef SIZE_PROF
			outsize -> idct_one_color_component_space[file_data->frame_index] = (file_data->hYb_size*file_data->wYb_size)*sizeof(color_block_t);
        #endif
//        for(int b = 0; b < file_data->hYb_size*file_data->wYb_size; b++) idct(file_data->YDCAC[b], file_data->Yblock[b]);

		idct(file_data->YDCAC, file_data->Yblock, file_data->hYb_size*file_data->wYb_size, frame_type, 0);

        #ifdef TIMING_COUNTER
			XTime_GetTime(&end);
			timing -> idct_time[file_data->frame_index] = end - start - timing -> null_time;
		#endif
//        for(int b = 0; b < file_data->hCb_size*file_data->wCb_size; b++) idct(file_data->CbDCAC[b], file_data->Cbblock[b]);
//        for(int b = 0; b < file_data->hCb_size*file_data->wCb_size; b++) idct(file_data->CrDCAC[b], file_data->Crblock[b]);
		Xil_L1DCacheFlush();
		Xil_L2CacheFlush();
		idct(file_data->CbDCAC, file_data->Cbblock, file_data->hCb_size*file_data->wCb_size, frame_type, 1);
		Xil_L1DCacheFlush();
		Xil_L2CacheFlush();
		idct(file_data->CrDCAC, file_data->Crblock, file_data->hCb_size*file_data->wCb_size, frame_type, 2);

        //ybcbr to rgb conversion
		#ifdef TIMING_COUNTER
			XTime_GetTime(&start);
		#endif
        #ifdef SIZE_PROF
			outsize -> ycbcr_to_rgb_one_frame_space[file_data->frame_index] = (file_data->hCb_size*file_data->wCb_size)*sizeof(rgb_pixel_t);
        #endif
		*(ADDRESS) = rgbblock;
//		DEBUG_PRINT("HERE\n")
//		DEBUG_PRINT_ARG("VALUE 3 %lu\n",rgbblock);
//    	DEBUG_PRINT_ARG("VALUE 0 %lu\n",*ADDRESS);
		spin_unlock(spinlock_start);
        for(int b = 0; b < (file_data->hCb_size*file_data->wCb_size)/2; b++)
            ycbcr_to_rgb(b/file_data->wCb_size*8, b%file_data->wCb_size*8, file_data->w_size, file_data->Yblock[b], file_data->Cbblock[b], file_data->Crblock[b], rgbblock);

		#ifdef TIMING_COUNTER
			XTime_GetTime(&end);
			timing -> ycbcr_to_rgb_time[file_data->frame_index] = end - start - timing -> null_time;
		#endif
}

void check_Iframe(FIL *file_in, file_data_struct *file_data, int increment){
	file_data->frame_index += increment;
	int offset =0;
	int index =0;
	uint32_t minimum = ~0;
    for(int count = 0; count < file_data->num_iframes; count++){
    	if((file_data->frame_index - file_data->trailer[count].frame_index) < minimum){
        	minimum = file_data->frame_index - file_data->trailer[count].frame_index;
        	offset = file_data->trailer[count].frame_position;
        	index = file_data->trailer[count].frame_index;
        }
		if(file_data->frame_index < file_data->trailer[count].frame_index ){
			break;
		}

    }
    f_lseek(file_in, offset);
    file_data->frame_index = index;
    minimum = ~0;
    offset = 0;
}

void free_file_data(file_data_struct* file_data) {

    if (file_data->trailer != NULL) {
        free(file_data->trailer);
        file_data->trailer = NULL;
    }
    DEBUG_PRINT("HOLA");
    if (file_data->Yblock != NULL) {
        free(file_data->Yblock);
        file_data->Yblock = NULL;
    }
    if (file_data->Cbblock != NULL) {
        free(file_data->Cbblock);
        file_data->Cbblock = NULL;
    }
    if (file_data->Crblock != NULL) {
        free(file_data->Crblock);
        file_data->Crblock = NULL;
    }
    if (file_data->YDCAC != NULL) {
        free(file_data->YDCAC);
        file_data->YDCAC = NULL;
    }
    if (file_data->CbDCAC != NULL) {
        free(file_data->CbDCAC);
        file_data->CbDCAC = NULL;
    }
    if (file_data->CrDCAC != NULL) {
        free(file_data->CrDCAC);
        file_data->CrDCAC = NULL;
    }
    if (file_data->Ybitstream != NULL) {
        free(file_data->Ybitstream);
        file_data->Ybitstream = NULL;
    }
    if (file_data->Cbbitstream != NULL) {
        free(file_data->Cbbitstream);
        file_data->Cbbitstream = NULL;
    }
    if (file_data->Crbitstream != NULL) {
        free(file_data->Crbitstream);
        file_data->Crbitstream = NULL;
    }
}

void calculate_stats(XTime attribute[1730], int type) {
	XTime max_time = 0;
    XTime min_time = 99999999999999;
    XTime avg_time = 0;


    // Compute the max, min and avg
    for (int i = 0; i < 1730; i++) {

        // Max
        if (attribute[i] > max_time && attribute[i] != 0) {
            max_time = attribute[i] ;
        }
        // Min
        if (attribute[i] < min_time && attribute[i] != 0) {
            min_time = attribute[i] ;
        }
        // Total
        avg_time += attribute[i];
    }


    // Avg
    switch(type){
		case 0:
			//I frame
			avg_time = avg_time/88;
			break;
		case 1:
			//P frame
			avg_time = avg_time/1642;
			break;
		default:
			break;
    }



    DEBUG_PRINT_ARG("MAXIMUM: %llu\n", max_time);
    DEBUG_PRINT_ARG("AVG: %llu\n", avg_time);
    DEBUG_PRINT_ARG("MINIMUM: %llu\n", min_time);
}

void calculate_stats_space(uint32_t attribute[1730]) {
	uint32_t max_time = 0;
	uint32_t min_time = 999999999;
	uint32_t total_time = 0;


    // Compute the max, min and avg
    for (int i = 0; i < 1730; i++) {
    	// Max
        if (attribute[i] > max_time && attribute[i] != 0) {
            max_time = attribute[i];
        }
        // Min
        if (attribute[i] < min_time && attribute[i] != 0) {
            min_time = attribute[i];
        }
        // Total
        total_time += attribute[i];
    }


    uint32_t avg_time = total_time / 1730;


    // Imprimir los resultados
    DEBUG_PRINT_ARG("MAXIMUM: %lu\n", max_time);
    DEBUG_PRINT_ARG("AVG: %lu\n", avg_time);
    DEBUG_PRINT_ARG("MINIMUM: %lu\n", min_time);
}


void print_timing_stats() {
	#ifdef TIMING_COUNTER
    DEBUG_PRINT("sd_read_frames_I_time:\n");
    DEBUG_PRINT_ARG("%lf\n",timing->sd_read_frames_I_time);
    calculate_stats(timing->sd_read_frames_I_time,0);


    DEBUG_PRINT("\nsd_read_frames_P_time:\n");
    calculate_stats(timing->sd_read_frames_P_time,1);


    DEBUG_PRINT("\nlossless_Y_I_time:\n");
    calculate_stats(timing->lossless_Y_I_time,0);


    DEBUG_PRINT("\nlossless_Y_P_time:\n");
    calculate_stats(timing->lossless_Y_P_time,1);


    DEBUG_PRINT("\nlossless_Cb_I_time:\n");
    calculate_stats(timing->lossless_Cb_I_time,0);


    DEBUG_PRINT("\nlossless_Cb_P_time:\n");
    calculate_stats(timing->lossless_Cb_P_time,1);


    DEBUG_PRINT("\nlossless_Cr_I_time:\n");
    calculate_stats(timing->lossless_Cr_I_time,0);


    DEBUG_PRINT("\nlossless_Cr_P_time:\n");
    calculate_stats(timing->lossless_Cr_P_time,1);


    DEBUG_PRINT("\nidct_time:\n");
    calculate_stats(timing->idct_time,3);


    DEBUG_PRINT("\nycbcr_to_rgb_time:\n");
    calculate_stats(timing->ycbcr_to_rgb_time,2);


    DEBUG_PRINT("\nycbcr_to_rgb_time:\n");
    calculate_stats(timing->ycbcr_to_rgb_time,2);

    DEBUG_PRINT("sd_read_frames_I_time:\n");
    calculate_stats(timing->sd_read_frames_I_time,2);
    DEBUG_PRINT("flush_L1 time:\n");
    calculate_stats(timing->flush_cache_L1,2);
    DEBUG_PRINT("flush_L2:\n");
    calculate_stats(timing->flush_cache_L1,2);
    #endif TIMING_COUNTER

	#ifdef SIZE_PROF
    DEBUG_PRINT("Size of sd_read_frames_I_size:\n");
    calculate_stats_space(outsize->sd_read_frames_I_size);

    DEBUG_PRINT("Size of sd_read_frames_P_size:\n");
    calculate_stats_space(outsize->sd_read_frames_P_size);

    DEBUG_PRINT("Size of lossless_Y_one_frame_time:\n");
    calculate_stats_space(outsize->lossless_Y_one_frame_time);

    DEBUG_PRINT("Size of lossless_Cb_one_frame_time:\n");
    calculate_stats_space(outsize->lossless_Cb_one_frame_time);

    DEBUG_PRINT("Size of lossless_Cr_one_frame_time:\n");
    calculate_stats_space(outsize->lossless_Cr_one_frame_time);

    DEBUG_PRINT("Size of idct_one_color_component_space:\n");
    calculate_stats_space(outsize->idct_one_color_component_space);

    DEBUG_PRINT("Size of ycbcr_to_rgb_one_frame_space:\n");
    calculate_stats_space(outsize->ycbcr_to_rgb_one_frame_space);
	#endif SIZE_PROF
}
