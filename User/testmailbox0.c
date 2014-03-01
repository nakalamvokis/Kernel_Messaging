#include "mailbox.h"
#include <stdio.h>

int main() {
	int count;
	ManageMailbox(false, &count);
	printf("Count is %d!\n", count);
	return 0;
}
