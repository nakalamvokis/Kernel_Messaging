#include <syscall.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include "mailbox.h"

#define __NR_cs3013_syscall1 349
#define __NR_cs3013_syscall2 350
#define __NR_cs3013_syscall3 351


/* function to send a message to another running process (using syscalls)
 * param dest -> process ID of recipient
 * param *msg -> uninterpreted array of bytes (message)
 * param len -> length of message (must not exceed MAX_MSG_SIZE)
 * param block -> specifies whether or not the message will be blocked
 */
long SendMsg(pid_t dest, void *msg, int len, bool block)
{
	return syscall(__NR_cs3013_syscall1, dest, msg, len, block);
}



/* function to recieve a message from another running process (using syscalls)
 * param *sender -> process ID of the sender
 * param *msg -> uninterpreted array of bytes (message)
 * param len -> length of message
 * param block -> specifies whether or not the message will be blocked
 */
long RcvMsg(pid_t *sender, void *msg, int *len, bool block)
{
	return syscall(__NR_cs3013_syscall2, sender, msg, len, block);
}




/* function to manage a process' mailbox (using syscalls)
 * param stop -> specifies whether to stop this mailbox from receiving any more messages
 * param *count -> the pointer of the vairable to hold the number of queued messages in the mailbox
 */
long ManageMailbox(bool stop, int *count)
{
	return syscall(__NR_cs3013_syscall3, stop, count);
}
