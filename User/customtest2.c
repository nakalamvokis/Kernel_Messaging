/* Nick Kalamvokis & Etienne Scraire
 * CS3013 - Project 4
 */

#include "mailbox.h"
#include <stdio.h>

int main() {
  pid_t sender;
  sender = getpid();
  int childPID = fork();
  int status;
  
  if(childPID == 0){
    void *msg[128];
    int len;
	int count;
    ManageMailbox(false, &count);
    if(status= RcvMsg(&sender,msg,&len,false))
    {printf("ERROR: %d\n", status);}
	else 
	{
		printf("Message received.\n");
		printf("Sender: %d\nMessage: %s\nLen: %d\n", sender, (char *) msg, len);
	}
	if(status= RcvMsg(&sender,msg,&len,false))
	{printf("ERROR: %d\n", status);}
	else 
	{
		printf("Message received.\n");
		printf("Sender: %d\nMessage: %s\nLen: %d\n", sender, (char *) msg, len);
	}
	if(status= RcvMsg(&sender,msg,&len,false))
	{printf("ERROR: %d\n", status);}
	else 
	{
		printf("Message received.\n");
		printf("Sender: %d\nMessage: %s\nLen: %d\n", sender, (char *) msg, len);
	}
  }
  else{
    char mesg[] = "I am your father";
    char mesg2[] = "Join me in the dark side!";
    char mesg3[] = "Together we can rule the galaxy!";
    printf("Sending Messages to child.\n");
    if (status=SendMsg(childPID, mesg, 17, false)){
      printf("Send failed with error %d\n", status);
    }
    if (status=SendMsg(childPID, mesg2, 26, false)){
      printf("Send failed with error %d\n", status);
    }
    if (status=SendMsg(childPID, mesg3, 33, false)){
      printf("Send failed with error %d\n", status);
    }
  }
  return 0;
}
