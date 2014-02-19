/**
* Nicholas Kalamvokis and Etienne Scraire
*
* Adapted from CS-502 Project #3, Fall 2006
*	originally submitted by Cliff Lindsay
* Modified for CS-3013, C-term 2012
*
*/

#ifndef __MAILBOX__
#define __MAILBOX__

#include <stdbool.h>
#include <linux/types.h>

#define NO_BLOCK 0
#define BLOCK   1
#define MAX_MSG_SIZE 128

#define NUM_MAILBOXES 150
#define MAILBOX_SIZE 20

// hash table of mailboxes -> pid is the key
struct mailbox mailbox_table[NUM_MAILBOXES];

// mailbox structure to be used for each process receiving messages
struct mailbox
{
	pid_t process_pid;
	char messages[MAILBOX_SIZE][MAX_MSG_SIZE];
};


// struct to be passed as parameter for send message syscall
struct send_info
{
	pid_t dest;
	void *msg;
	int len;
	bool block;
};


// struct to be passed as parameter for recieve message syscall
struct rcv_info
{
	pid_t *sender;
	void *msg;
	int *len;
	bool block;
};


// struct to be passed as parameter for manage syscall
struct manage_info
{
	bool stop;
	int *count;
};


/**
 * Functions for msgs
 *
 * */
long SendMsg(pid_t dest, void *msg, int len, bool block);
long RcvMsg(pid_t *sender, void *msg, int *len, bool block);

/**
 * functions for maintaining mailboxes
 *
 * */
long ManageMailbox(bool stop, int *count);

/**
 * error codes pertaining to mailboxes
 *
 * */
#define MAILBOX_FULL		1001
#define MAILBOX_EMPTY		1002
#define MAILBOX_STOPPED		1003
#define MAILBOX_INVALID		1004
#define MSG_LENGTH_ERROR	1005
#define MSG_ARG_ERROR		1006
#define MAILBOX_ERROR		1007

#endif

