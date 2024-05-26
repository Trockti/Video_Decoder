
//
//  main.c
//
//  Created by Sonia Navas Rutete and Javier Madrid Hijosa
//

#include "xil_mmu.h"
#include "spinlock1.h"
#include "spinlockAdd.h"
#include "decoder/mjpeg423_decoder.h"
#include "ece423_vid_ctl/ece423_vid_ctl.h"
#include "stdio.h"
#include "common/util.h"
#include "string.h"
#include "decoder/timer_gpio.h"
#include "ece423_vid_ctl/ece423_vid_ctl.h"
#include "ff.h"
#include "xtime_l.h"


int dummyvar __attribute((section(".spinlock_section")));

//input bmp base filename for the encoder. Must end in 0000.bmp
#define BMP_INPUT_FILENAME "test0000.bmp"
//output bmp base filename for the decoder. Must end in 0000.bmp
#define BMP_OUTPUT_FILENAME "testout0000.bmp"
//video file
#define VIDEO_FILENAME "video.mpg"

//number of frames to encode
#define NUM_FRAMES 5
//starting bmp number for the encoder. I.e., if the first bmp to encode is "name0002.bmp", set it to 2.
#define START_BMP 0
//bmp input stride. The encoder encodes one bmp every BMP_STRIDE.
//I.e., if set to 1, every bmp is included; if set to 2, one bmp every 2; etc.
//This is useful to adjust the frame rate compared to the original bmp sequence.
#define BMP_STRIDE 1

//max separation between successive I frames. The encoder will insert an I frame at most every MAX_I_INTERVAL frames.
#define MAX_I_INTERVAL 24
//resolution: width
#define WIDTH 280
//resolution: height
#define HEIGHT 200


#define TIMER_1S 325000000 //1 second
#define TIMER_FPS 10

//#define TIMER
//#define TIMING_COUNTER

#ifndef TIMER
#define NOTIMER
#endif


volatile int8_t btn = -1;
volatile int8_t timer_val = 0;
volatile int paused = 0;
static FATFS fatfs;
static FIL file_in;
static int video_pos;

int change = 0;
int charged = 0;


XTime start1;
XTime end1;
XTime null_time;




FRESULT scan_files (char **videos_input, char **final_video){
	FRESULT res;
	DIR dir ;

	static FILINFO fno;
	int position = 0;

	char str[] = "3:/";



	res = f_opendir(&dir, "3:/"); /* Open the directory */
	if ( res == FR_OK) {
		for (;;) {
			res = f_readdir (&dir, &fno); /* Read a directory item */
			if ( res != FR_OK || fno.fname[0] == 0) break; /* Break on error or end of dir */
			if (!( fno.fattrib & AM_DIR)) { /* It is a directory */
				char *dot;
				dot = strchr(fno.fname,'.');
				size_t length = strlen(dot);
				if(dot != NULL && length == 4){
					if(dot[1] =='M' && dot[2] =='P' && dot[3] =='G'){
						for(int i = 0; i <=1; i++){
							if(strcmp(fno.fname, videos_input[i]) == 0){
								final_video[position] = (char *)malloc(strlen(fno.fname)+1 +strlen(str));
								sprintf (final_video[position],"%s/%s", "3:", fno.fname);
								final_video[position+1]= NULL;
								position++;
							}
						}
					}
				}
			}
		}
		f_closedir (&dir) ;
	}
	return res ;
}



void GpioHandler(void*CallBackRef, u32 Bank, u32 Status)
{
	btn = read_pin();
}

void TimerHandler(void*){
#ifdef TIMER
if (paused == 0 || charged == 1){
	vdma_out();
	if (charged == 1){
		charged = -1;
	}
}
#endif

}

void load_new_video(char **final_videos) {
	if (f_open(&file_in, final_videos[video_pos%2], FA_READ) != FR_OK ) error_and_exit("cannot open input file");
}

void close_video_file() {


    if (f_close(&file_in) != FR_OK) {error_and_exit("error closing video file");
    }
}



int main (int argc, const char * argv[])
{
	Xil_SetTlbAttributes(SHARED_ADDRESS,DEVICE_MEMORY);

//	printf(spinlock_start);

//	int i =0;
//	while(*spinlock_start != 1){
//		i++;
//	}
//
	printf("START");

	char *videos_input[] = { "V1_300.MPG", "V1_1730.MPG"};
	char *final_videos[2];
	rgb_pixel_t* rgbblock;
	final_videos[0]= NULL;

	f_mount(&fatfs, "3:/", 1);
 	scan_files(videos_input, final_videos);

 	video_pos = 0;

 	timer_gpio_init(TimerHandler, GpioHandler);
 	timer_start(TIMER_1S/TIMER_FPS);

 	dma_init();

 	load_new_video(final_videos);



	file_data_struct *file_data = FILE_DATA_ADDRESS;

 	mjpeg423_decode_init(&file_in, file_data);
	vdma_init(1280, 720, 2);


	#ifdef NOTIMER
	rgbblock = buff_next();
	#endif


 	while (1){
 		// DESCOMMENT NEXT LINE TO DESACTIVATE TIMER
		#ifdef TIMER

 		if(((rgbblock = buff_next()) != NULL || charged == -1) && file_data->frame_index < file_data->num_frames && (paused != 1 || change == 1)){
 			if(charged == -1){
 				buff_empty();
 				rgbblock = buff_next();
 				charged = 0;
 			}
			read_frame(&file_in, file_data, rgbblock);
			file_data->frame_index += 1;
			#ifdef TIMING_COUNTER
				XTime_GetTime(&start1);
				XTime_GetTime(&end1);
				null_time = end1 - start1;
				XTime_GetTime(&start1);
			#endif
			buff_reg();
			#ifdef TIMING_COUNTER
				XTime_GetTime(&end1);
				DEBUG_PRINT_ARG("BUFF REG TIME IS: %llu\n", end1 - start1 - null_time)
			#endif
			//DESCOMMENT NEXT LINE TO ACTIVATE TIMER
			if (change == 1){
				DEBUG_PRINT_ARG("\nFrame Charged#%u\n",file_data->frame_index)
				charged = 1;
			}
			change = 0;
 		}



		#endif
 		// COMMENT NEXT LINE TO ACTIVATE TIMER
		#ifdef NOTIMER
		if(file_data->frame_index < file_data->num_frames && (paused != 1 || change == 1)){

			read_frame(&file_in, file_data, rgbblock);
			file_data->frame_index += 1;
			#ifdef TIMING_COUNTER
				XTime_GetTime(&start1);
				XTime_GetTime(&end1);
				null_time = end1 - start1;
				XTime_GetTime(&start1);
			#endif
			buff_reg();
			#ifdef TIMING_COUNTER
				XTime_GetTime(&end1);
				DEBUG_PRINT_ARG("BUFF REG TIME IS: %llu\n", end1 - start1 - null_time)
			#endif
			// COMMENT NEXT LINE TO ACTIVATE TIMER
			vdma_out();
			// COMMENT NEXT LINE TO ACTIVATE TIMER

			rgbblock = buff_next();
			// DESCOMMENT NEXT LINE TO ACTIVATE TIMER
			change = 0;
		}
		#endif
		#ifdef TIMING_COUNTER
		if (file_data->frame_index == file_data->num_frames){
			print_timing_stats();
			while(1){}
		}
		#endif
		if (btn != -1) {
			switch (btn) {
			case 0:

				vdma_close();

				close_video_file();
				video_pos++;
				load_new_video(final_videos);
				mjpeg423_decode_init(&file_in, file_data);
				vdma_init(1280, 720, 2);
				paused = 0;
				break;
			case 1:
				paused = !paused;
				DEBUG_PRINT_ARG("pause %u\n", paused)
				break;

			case 2:

				if(file_data->frame_index+120 > file_data->num_frames){
					paused = 1;
				}else{
					check_Iframe(&file_in, file_data, 120);
					// DESCOMMENT NEXT LINE TO ACTIVATE TIMER
					#ifdef TIMER
					buff_empty();

					#endif
					change = 1;
				}
				break;
			case 3:
				DEBUG_PRINT_ARG("back %u\n", btn)
				if(file_data->frame_index < 120){
					file_data->frame_index = 0;
					if(f_lseek(&file_in,5 * sizeof(uint32_t)) != FR_OK) error_and_exit("cannot seek into file");
				}else{

					check_Iframe(&file_in, file_data, -120);
					// DESCOMMENT NEXT LINE TO ACTIVATE TIMER

				}
				#ifdef TIMER
				buff_empty();
				#endif
				change = 1;
				break;

			}
		}
		btn =-1;
		// DESCOMMENT NEXT LINE TO ACTIVATE TIMER



	}
	return 0;
}

