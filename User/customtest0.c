#include <stdio.h>
#include "mailbox.h"

int main()
{
	int status, count, len = 0;
	pid_t pid;
	pid_t sender = 0;
	pid = getpid();
	char mesg[] = "Test Message";
	void* msg[128];
	
	status = SendMsg(pid, mesg, 13, false);
	if (status)
		printf("Send ERROR: %d\n", status);
	printf("Sent message!\n");
	
	status = ManageMailbox(false, &count);
	if (status)
		printf("Manage ERROR: %d\n", status);
		
	printf("There is %d message in the inbox!\n", count);
	
	status = RcvMsg(&sender, msg, &len, false);
	if (status)
		printf("Receive ERROR: %d\n", status);
	
	printf("Received message %s from sender %d of length %d!\n", (char*) msg, sender, len);
	return 0;
	
}
