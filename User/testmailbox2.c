/* Alfredo Porras
 * July 12th, 2011
 * CS 3013
 * Project 4 - test program 1
 * Tests if messages can be sent and received.
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
		printf("ERROR: %d\n", status);
	else {
    printf("Message received.\n");
    printf("Message: %s\n", (char *) msg);
    }
  }
  else{
    char mesg[] = "I am your father";
    printf("Sending Message to child.\n");
    if (status=SendMsg(childPID, mesg, 17, false)){
      printf("Send failed with error %d\n", status);
    }
  }
  return 0;
}
