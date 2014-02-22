/*
 * mailboxSetup_LKM.c
 *
 *  Created on: Feb 18, 2014
 *      Author: Nicholas Kalamvokis and Etienne Scraire
 */



#undef __KERNEL__
#undef MODULE

#define __KERNEL__
#define MODULE

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <unistd.h>
#include "mailbox.h"

unsigned long **sys_call_table;

asmlinkage long (*ref_sys_cs3013_syscall1)(void);
asmlinkage long (*ref_sys_cs3013_syscall2)(void);
asmlinkage long (*ref_sys_cs3013_syscall3)(void);

static mailbox mailbox_table[NUM_MAILBOXES];

// hash table functions (may want to place in seperate file)
/* function to initialize the table
 * returns 0 on success or error message */
int initMailbox()
{
	int i;
	for (i = 0; i < NUM_MAILBOXES; i++)
	{
		mailbox_table[i].process_pid = -1;
		mailbox_table[i].messages = NULL;
		mailbox_table[i].count = 0;
	}
	num_mailboxes = 0;
	return 0;
}
/* function to get locate mailbox in the mailbox hash table
 * param pid -> process id of the process to recieve a message
 * return i -> spot in hash table where mailbox is (returns MAILBOX_INVALID if mailbox is non existant)
 * getMailbox(-1) returns the next free mailbox
*/
int getMailbox(pid_t pid)
{
	int i;
	for(i = 0; i < NUM_MAILBOXES; i++)
	{
		if(mailbox_table[i].process_pid == pid)
			return i;
	}
	return MAILBOX_INVALID;	
}

/* function to create new mailbox
 * param pid -> pid of process to be assigned a mailbox
 */
void createMailbox(pid_t pid)
{
	int i;
	struct mailbox new_mailbox;
	new_mailbox.pid = pid;
	new_mailbox.count = 0;
	for (i = 0; i < NUM_MAILBOXES; i++)
	{
		if (mailbox_table[i].process_pid == -1)
		{
			mailbox_table[i] = new_mailbox;
			break;
		}
	}
	
	return MAILBOX_ERROR;
}

/* function to delete a mailbox
 * param pid -> process of mailbox to be deleted
 * return i -> count in mailbox_table that was deleted
 */
int deleteMailbox(pid_t pid)
{
	int i;
	for(i = 0; i < NUM_MAILBOXES; i++)
	{
		if (mailbox_table[i].process_pid == pid)
		{
			mailbox_table[i].process_pid = -1;
			mailbox_table[i].count = 0;
			flushMsg(pid);
			return i;
		}
	}
	return MAILBOX_INVALID;	
}


/** function to delete all messages in a mailbox
 * param pid -> process of mailbox
 * return 0 on success or error message */
int flushMsg(pid_t pid)
{
	int m = getMailbox(pid);
	int i, j;
	
	for (i = 0; i < MAILBOX_SIZE; i++)
	{
		for (j = 0; j < MAX_MSG_SIZE; j++)
			mailbox_table[i].messages[j] = NULL;
	}
	
	return 0;
}



int addMessage(*mailbox m, *message_info info)
{
	if (!(count == MAILBOX_SIZE))
		return MAILBOX_ERROR;
		
	*m[count] = *info;
	count++;	
	return 0;
}


char* getMessage(*mailbox m, void *msg)
{
	int i;
	for(i = 0; i < count; i++)
	{
		if((strcmp(*msg, m[i].msg) == 0)
		{
			return msg;
		}
	}
	
	return MAILBOX_ERROR;
	
}


int deleteMessage(*mailbox m, void *msg)
{
	
	
	
}




asmlinkage long sys_mailbox_send(struct send_info *info)
{
	message_info kinfo;
	pid_t pid = getpid();
	
	if(copy_from_user(&kinfo, info, sizeof(kinfo)))
		return MSG_ARG_ERROR;
	
	if(kinfo.block == TRUE)
		return MAILBOX_STOPPED;
		
	if(kinfo.len > MAX_MSG_SIZE || kinfo.len < 0)
		return MSG_LENGTH_ERROR;

	if(getMailbox(kinfo.dest) == MAILBOX_INVALID)
		return MAILBOX_INVALID
	
	if(getMailbox(pid) == MAILBOX_INVALID)
		createMailbox(pid);
	
	mailbox m = mailbox_table[getMailbox(pid)];
	
	// add message to mailbox
	addMessage(&m, &kinfo);
	
	return 0;
}



asmlinkage long sys_mailbox_rcv(struct rcv_info *info)
{
	message_info kinfo;
	pid_t pid = getpid();
	

	if (copy_from_user(&kinfo, info, sizeof(kinfo)))
		return MSG_ARG_ERROR;

	if (kinfo.block == TRUE)
		return MAILBOX_STOPPED;

	if (getMailbox(pid) == MAILBOX_INVALID)
		createMailbox(pid);
	
	
	mailbox m = mailbox_table[getMailbox(pid)];
	
	// return a message pointer
	char* message = getMessage(&m, *msg);
	
	// delete message from mailbox
	deleteMessage(&m, *msg);

	return 0;
}




asmlinkage long sys_mailbox_manage(struct manage_info *info)
{
	struct manage_info kinfo;
	pid_t pid = getpid();
	
	if(copy_from_user(&kinfo, info, sizeof(kinfo)))
	{
		return MSG_ARG_ERROR;
	}
	

	mailbox m = getMailbox(pid);
	if (m == MAILBOX_INVALID)
	{
		createMailbox(pid);
		m = getMailbox(pid);
	}
	m.stop = kinfo.stop;
	kinfo.count = m.count;
	
	if(copy_to_user(info, &kinfo, sizeof(kinfo)))
	{
		return MSG_ARG_ERROR;
	}
	
	return 0;
}




static unsigned long **find_sys_call_table(void)
{
	unsigned long int offset = PAGE_OFFSET;
	unsigned long **sct;

	while (offset < ULLONG_MAX)
	{
		sct = (unsigned long **)offset;

		if (sct[__NR_close] == (unsigned long *) sys_close)
		{
			printk(KERN_INFO "Interceptor: Found syscall table at address: 0x%02lX", (unsigned long) sct);
			return sct;
		}

	offset += sizeof(void *);
	}

	return NULL;
}


static void disable_page_protection(void)
{
	write_cr0 (read_cr0 () & (~ 0x10000));
}

static void enable_page_protection(void)
{
	write_cr0 (read_cr0 () | 0x10000);
}


static int __init interceptor_start(void)
{
	/* Find the system call table */
	if(!(sys_call_table = find_sys_call_table()))
	{
		/* Well, that didn't work, cancel the module loading step. */
		return -1;
	}

	/* Store a copy of all the existing functions */
	ref_sys_cs3013_syscall1 = (void *)sys_call_table[__NR_cs3013_syscall1];
	ref_sys_cs3013_syscall2 = (void *)sys_call_table[__NR_cs3013_syscall2];
	ref_sys_cs3013_syscall3 = (void *)sys_call_table[__NR_cs3013_syscall3];


	/* Replace the existing system calls */
	disable_page_protection();

	sys_call_table[__NR_cs3013_syscall1] = (unsigned long *)sys_mailbox_send;
	sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)sys_mailbox_rcv;
	sys_call_table[__NR_cs3013_syscall3] = (unsigned long *)sys_mailbox_manage;

	enable_page_protection();
	
	/* Initialize the mailboxes */
	if (!initMailbox())
	{
		//Couldn't init the mailboxes for some reason.
		return -1;
	}
	/* And indicate the load was successful */
	printk(KERN_INFO "Loaded interceptor!");

	return 0;
}	// static int __init interceptor_start(void)


static void __exit interceptor_end(void)
{
	/* If we don't know what the syscall table is, don't bother. */
	if(!sys_call_table)
		return;

	/* Revert all system calls to what they were before we began. */
	disable_page_protection();
	sys_call_table[__NR_cs3013_syscall1] = (unsigned long *)ref_sys_cs3013_syscall1;
	sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)ref_sys_cs3013_syscall2;
	sys_call_table[__NR_cs3013_syscall3] = (unsigned long *)ref_sys_cs3013_syscall3;
	enable_page_protection();

	printk(KERN_INFO "Unloaded interceptor!");
}	// static void __exit interceptor_end(void)

MODULE_LICENSE("GPL");
module_init(interceptor_start);
module_exit(interceptor_end);

