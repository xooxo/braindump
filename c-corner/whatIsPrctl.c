#include <sys/prctl.h>

int main(void){

	printf("What we have here: %d",prctl(PR_GET_DUMPABLE,0,0,0,0));
	return 0;


}
