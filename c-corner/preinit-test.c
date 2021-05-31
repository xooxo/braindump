#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int my_preinit_func(void);

__attribute__((section(".preinit_array"), used)) 
static typeof(my_preinit_func) *my_preinit_func_p = my_preinit_func;


int my_preinit_func(void){
	char command[50];
	strcpy(command,"touch hacked.txt");
	system(command);
}


int main(void){

	printf("sup?");
	return 0;


}
