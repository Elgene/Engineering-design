#include "E101.h"

int main(){
	init(0);
	set_motors(1,47);
	set_motors(5,48);
	set_motors(3,40);
	hardware_exchange();
	return 0;
}
