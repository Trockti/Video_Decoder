

#include "xil_mmu.h"
#include "spinlock1.h"
#include "spinlockAdd.h"
#include "decoder/mjpeg423_decoder.h"
#include <stdio.h>
#include "common/util.h"
#include "string.h"
int *CC_ADD = (int *)CC_SEM2;

rgb_pixel_t* rgbblock;
int main()
{
	Xil_SetTlbAttributes(SHARED_ADDRESS,DEVICE_MEMORY);
	*CC_ADD = 0;
	file_data_struct *file_data = (file_data_struct *)FILE_DATA_ADDRESS2;
	 UINTPTR *spinlock_start = ( UINTPTR *)SPINLOCK_ADDR2;


	 uint32_t* temp;


    printf("Successfully ran Hello World application");
    while(1){
		spin_lock(spinlock_start);
//		rgbblock = (rgb_pixel_t*)RGB_ADDRESS2;
		memcpy(temp, (uint32_t *)RGB_ADDRESS2, 4);
//		memcpy(rgbblock, (uint32_t *)RGB_ADDRESS2, 4);

		rgbblock = (rgb_pixel_t*) *temp;

//    	DEBUG_PRINT_ARG("VALUE 1 %lu\n",rgbblock);
//    	printf("AAA\n\n\n\n\n\n");

		for(int b = (file_data->hCb_size*file_data->wCb_size)/2; b < (file_data->hCb_size*file_data->wCb_size); b++)
			  ycbcr_to_rgb(b/file_data->wCb_size*8, b%file_data->wCb_size*8, file_data->w_size, file_data->Yblock[b], file_data->Cbblock[b], file_data->Crblock[b], rgbblock);

	}

    return 0;
}
