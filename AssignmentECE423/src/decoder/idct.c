//
//  idct.c
//  mjpeg423app
//
//  Created by Rodolfo Pellizzoni on 12/28/13.
//  Copyright (c) 2013 __MyCompanyName__. All rights reserved.
//

#include <stdio.h>
#include "mjpeg423_decoder.h"
#include "../common/dct_math.h"
#include "../common/util.h"
#include <xaxidma.h>
#include "../spinlockAdd.h"

XAxiDma *InstancePtr;
XAxiDma AxiDma;


#ifndef NULL_DCT

/*
* This implementation is based on an algorithm described in
*   C. Loeffler, A. Ligtenberg and G. Moschytz, "Practical Fast 1-D DCT
*   Algorithms with 11 Multiplications", Proc. Int'l. Conf. on Acoustics,
*   Speech, and Signal Processing 1989 (ICASSP '89), pp. 988-991.
* The primary algorithm described there uses 11 multiplies and 29 adds.
* We use their alternate method with 12 multiplies and 32 adds.
* The advantage is that no data path contains more than one multiplication.
*/

/* normalize the result between 0 and 255 */
/* this is required to handle precision errors that might cause the decoded result to fall out of range */
#define NORMALIZE(x) (temp = (x), ( (temp < 0) ? 0 : ( (temp > 255) ? 255 : temp  ) ) )

void idct(dct_block_t DCAC[], color_block_t blockout[], int size, uint32_t frame_type, int counter){
	//Perform a transfer
	file_data_struct *file_data = FILE_DATA_ADDRESS;




	XAxiDma_SimpleTransfer(&AxiDma, (INTPTR)DCAC, size*128 , XAXIDMA_DMA_TO_DEVICE); //Program read channel
	XAxiDma_SimpleTransfer(&AxiDma, (INTPTR)blockout, size*64, XAXIDMA_DEVICE_TO_DMA); //Program write channel

	if(counter ==0){
		lossless_decode(file_data->hCb_size*file_data->wCb_size, file_data->Cbbitstream, file_data->CbDCAC, Cquant, frame_type);

	}
	if(counter ==1){
		lossless_decode(file_data->hCb_size*file_data->wCb_size, file_data->Crbitstream, file_data->CrDCAC, Cquant, frame_type);

	}

	//Poll until transfer is done, then reset the DMA in preparation for next transfer
	while (!( XAxiDma_ReadReg(InstancePtr->RxBdRing[0].ChanBase, XAXIDMA_SR_OFFSET) & XAXIDMA_ERR_INTERNAL_MASK)) {}
	XAxiDma_Reset(&AxiDma);
	while (!XAxiDma_ResetIsDone(&AxiDma)){}
}

void dma_init(){
	// Initialize the DMA
	InstancePtr = &AxiDma;
	XAxiDma_CfgInitialize(&AxiDma, XAxiDma_LookupConfig(XPAR_AXIDMA_0_DEVICE_ID));
	XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK, XAXIDMA_DMA_TO_DEVICE);

}


#else /* Null implementation */

void idct(pdct_block_t DCAC, pcolor_block_t block)
{
    for(int row = 0 ; row < 8; row ++)
        for(int column = 0; column < 8; column++)
            block[row][column] = DCAC[row][column];
}

#endif

