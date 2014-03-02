#include <stdio.h>
#include "mailbox.h"

int main()
{
	pid_t parent;
	parent = getpid();
	int status, count;
	int childPID = fork();

	if(childPID == 0) 
	{
		sleep(1);
		char mesg[] = "I am your child!";
		status = SendMsg(childPID, mesg, 17, false);
		if (status)
			printf("SendMsg failed with error status: %d\n", status);
		else
			printf("SendMsg was successful!\n");
	}
	else
	{
		pid_t child;
		void *msg[128];
		int len;
		sleep(2);
		status = RcvMsg(&child, msg, &len, false);
		if (status)
			printf("RcvMsg failed with error status: %d\n", status);
		else
		{
			printf("Message received.\n");
			printf("Message: %s\n", (char *) msg);
		}
	}
	
	return 0;
	
}