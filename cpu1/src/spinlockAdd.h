#include "decoder/mjpeg423_decoder.h"


//#define SHARED_ADDRESS 0x1fe16030
#define SHARED_ADDRESS 0x1fd00000
#define SPINLOCK_SIZE 0x1000
#define SPINLOCK_ADDR2  (SHARED_ADDRESS)
#define RGB_ADDRESS2 (SPINLOCK_ADDR2 + SPINLOCK_SIZE)
#define FILE_DATA_ADDRESS2 (RGB_ADDRESS2 + 2*sizeof(uint32_t))
#define CC_SEM2 (FILE_DATA_ADDRESS2)






