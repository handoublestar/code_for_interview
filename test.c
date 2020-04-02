#include <stdio.h>
#include "socketserver.h"
int main(void)
{
	printf("start:\n");
	Server_init("0.0.0.0", 1234);
	return 0;
}
