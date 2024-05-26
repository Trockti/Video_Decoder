//
//  mjpeg423_decoder.h
//  mjpeg423app
//
//  Created by Rodolfo Pellizzoni on 12/24/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#ifndef mjpeg423app_mjpeg423_decoder_h
#define mjpeg423app_mjpeg423_decoder_h

#include "../common/mjpeg423_types.h"
#include "xtime_l.h"
typedef unsigned int UINT;


typedef struct {

    UINT NumBytesRead;

    // Header
    uint32_t num_frames;
    uint32_t w_size;
    uint32_t h_size;
    uint32_t num_iframes;
    uint32_t payload_size;


    //trailer
    iframe_trailer_t *trailer;


    int hCb_size;
    int wCb_size;
    int hYb_size;
    int wYb_size;

    color_block_t* Yblock;
    color_block_t* Cbblock;
    color_block_t* Crblock;
    dct_block_t* YDCAC;
    dct_block_t* CbDCAC;
    dct_block_t* CrDCAC;

    uint8_t* Ybitstream;
    uint8_t* Cbbitstream;
    uint8_t* Crbitstream;


    int frame_index;


} file_data_struct;


typedef struct {
    XTime sd_read_frames_I_time[1730];
    XTime sd_read_frames_P_time[1730];
    XTime lossless_Y_I_time[1730];
    XTime lossless_Y_P_time[1730];
    XTime lossless_Cb_I_time[1730];
    XTime lossless_Cb_P_time[1730];
    XTime lossless_Cr_I_time[1730];
    XTime lossless_Cr_P_time[1730];
    XTime flush_cache_L1[1730];
    XTime flush_cache_L2[1730];
    XTime idct_time[1730];
    XTime ycbcr_to_rgb_time[1730];
    XTime null_time;
} TimingData;

typedef struct {
	uint32_t sd_read_frames_I_size[1730];
    uint32_t sd_read_frames_P_size[1730];
    uint32_t lossless_Y_one_frame_time[1730];
    uint32_t lossless_Cb_one_frame_time[1730];
    uint32_t lossless_Cr_one_frame_time[1730];
    UINT idct_one_color_component_space[1730];
    UINT ycbcr_to_rgb_one_frame_space[1730];
} SpaceData;



void free_file_data(file_data_struct* file_data);

void ycbcr_to_rgb(int h, int w, uint32_t w_size, pcolor_block_t Y, pcolor_block_t Cb, pcolor_block_t Cr, rgb_pixel_t* rgbblock);
void idct(dct_block_t DCAC[], color_block_t block[], int size, uint32_t frame_type, int counter);
void dma_init();
void lossless_decode(int num_blocks, void* bitstream, dct_block_t* DCACq, dct_block_t quant, BOOL P);
void calculate_stats(XTime attribute[1730], int type);
void print_timing_stats();
#endif
