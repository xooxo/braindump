#include <stdio.h>

int main(void){

	__asm__("call -10(%rip)");
	return 0;



}
