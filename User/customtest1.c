#include <stdio.h>
#include "mailbox.h"

int main()
{
	pid_t parent;
	parent = getpid();
	int status;
	int childPID = fork();

	if(childPID == 0) 
	{
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
		int len, count;
		ManageMailbox(false, &count);
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
