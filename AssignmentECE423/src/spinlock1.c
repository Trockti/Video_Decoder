#include <xil_types.h>
#include <xstatus.h>

void spin_lock(volatile UINTPTR *lockaddr)
{
	u32 LockTempVar;


	    __asm__ __volatile__(
		    "1:    ldrex    %0, [%1]     \n"
	        "      teq		%0, %3       \n"
	        "      strexeq  %0, %2, [%1] \n"
	        "      teqeq	%0, #0       \n"
	        "      bne      1b           \n"
	        "      dmb                   \n"
			: "=&r" (LockTempVar)
			: "r" (lockaddr), "r"(0), "r"(1)
			: "cc");

	    return XST_SUCCESS;
}

void spin_unlock(volatile UINTPTR *lockaddr)
{
    __asm__ __volatile__(
        "dmb			         \n"
        "str 	%1, [%0]         \n"
        :
        : "r" (lockaddr), "r" (1)
        : "cc");

    return XST_SUCCESS;
}
