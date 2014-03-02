#include <syscall.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include "mailbox.h"

#define __NR_cs3013_syscall1 349
#define __NR_cs3013_syscall2 350
#define __NR_cs3013_syscall3 351

// struct to be passed as parameter for send and recieve message syscall
typedef struct message_info
{
	pid_t *sender;
	pid_t dest;
	void *msg;
	int len;
	int* lenPtr;
	bool block;
} message_info;

// struct to be passed as parameter for manage syscall
typedef struct manage_info
{
	bool stop;
	int *count;
} manage_info;



/* function to send a message to another running process (using syscalls)
 * param dest -> process ID of recipient
 * param *msg -> uninterpreted array of bytes (message)
 * param len -> length of message (must not exceed MAX_MSG_SIZE)
 * param block -> specifies whether or not the message will be blocked
 */
long SendMsg(pid_t dest, void *msg, int len, bool block)
{
	message_info info;
	info.dest = dest;
	info.msg = msg;
	info.len = len;
	info.block = block;
	printf("Sending message %s, to dest %d!\n", (char *) info.msg, info.dest);
	 
	return syscall(__NR_cs3013_syscall1, &info);
}



/* function to recieve a message from another running process (using syscalls)
 * param *sender -> process ID of the sender
 * param *msg -> uninterpreted array of bytes (message)
 * param len -> length of message
 * param block -> specifies whether or not the message will be blocked
 */
long RcvMsg(pid_t *sender, void *msg, int *len, bool block)
{
	message_info info;
	
	
	info.block = block;
	
	int status = syscall(__NR_cs3013_syscall2, &info);
	
	sender = info.sender;
	msg = info.msg;
	len = info.lenPtr;
	
	return status;
}




/* function to manage a process' mailbox (using syscalls)
 * param stop -> specifies whether to stop this mailbox from receiving any more messages
 * param *count -> the pointer of the vairable to hold the number of queued messages in the mailbox
 */
long ManageMailbox(bool stop, int *count)
{
	manage_info info;
	info.stop = stop;
	int status = syscall(__NR_cs3013_syscall3, &info);
	count = info.count;
	return status;
}
