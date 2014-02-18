




/* function to send a message to another running process (using syscalls)
 * param dest -> process ID of recipient
 * param *msg -> uninterpreted array of bytes (message)
 * param len -> length of message (must not exceed MAX_MSG_SIZE)
 * param block -> specifies whether or not the message will be blocked
 */
long SendMsg(pid_t dest, void *msg, int len, bool block)
{
	struct send_info info;
	
	info.dest = dest;
	info.msg = msg;
	info.len = len;
	info.block = block;
	
	return (cs3013_syscall1, &info);
}



/* function to recieve a message from another running process (using syscalls)
 * param *sender -> process ID of the sender
 * param *msg -> uninterpreted array of bytes (message)
 * param len -> length of message
 * param block -> specifies whether or not the message will be blocked
 */
long RcvMsg(pid_t *sender, void *msg, int *len, bool block)
{
	struct rcv_info info;
	
	info.sender = sender;
	info.msg = msg;
	info.len = len;
	info.block = block;
	
	return (cs3013_syscall2, &info);
}




/* function to manage a process' mailbox (using syscalls)
 * param stop -> specifies whether to stop this mailbox from receiving any more messages
 * param *count -> the pointer of the vairable to hold the number of queued messages in the mailbox
 */
long ManageMailbox(bool stop, int *count)
{
	struct manage_info info;
	
	info.stop = stop;
	info.count = count;
	
	return (cs3013_syscall3, &info);
}