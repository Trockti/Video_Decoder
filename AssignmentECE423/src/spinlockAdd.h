#include "decoder/mjpeg423_decoder.h"


//#define SHARED_ADDRESS 0x1fe16030
#define SHARED_ADDRESS 0x1fd00000
#define SPINLOCK_SIZE 0x1000
#define SPINLOCK_ADDR1  (SHARED_ADDRESS)
#define RGB_ADDRESS (SPINLOCK_ADDR1 + SPINLOCK_SIZE)
#define FILE_DATA_ADDRESS (RGB_ADDRESS + 2*sizeof(uint32_t))
#define CC_SEM (FILE_DATA_ADDRESS + sizeof(int))






