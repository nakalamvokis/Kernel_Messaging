/* Nick Kalamvokis & Etienne Scraire
 * CS3013 - Project 4
 */
#include <stdio.h>
#include "mailbox.h"

int main()
{
	pid_t pid = fork();
	int status;
	
	if (pid == 0)
	{
		printf("Child managing\n");
		status = manage();
		sleep(1);
		printf("Childing exiting\n");
		exit(0);
	}
	else
	{
		sleep(5);
		printf("Parent managing\n");
		status = manage();
		printf("Parent exiting\n");
	}
	
	return 0;	
}

int manage()
{
	int status, count;
	status = ManageMailbox(false, &count);
	if (status)
		printf("Manage ERROR: %d\n", status);
	else
		printf("%d messages in the mailbox!", count);
		
	return status;
}
