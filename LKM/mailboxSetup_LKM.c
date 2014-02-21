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
#include "mailbox.h"

unsigned long **sys_call_table;

asmlinkage long (*ref_sys_cs3013_syscall1)(void);
asmlinkage long (*ref_sys_cs3013_syscall2)(void);
asmlinkage long (*ref_sys_cs3013_syscall3)(void);


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
	}
	num_mailboxes = 0;
	return 0;
}
/* function to get locate mailbox in the mailbox hash table
 * param pid -> process id of the process to recieve a message
 * return i -> spot in hash table where mailbox is (returns MAILBOX_INVALID if mailbox is non existant)
*/
int getMailbox(pid_t pid)
{
	int i;
	for(i = 0; i < NUM_MAILBOXES; i++)
	{
		if(mailbox_table[i].pid == pid)
		{
			return i;
		}
	}
	return MAILBOX_INVALID;	
}

/* function to create new mailbox
 * param pid -> pid of process to be assigned a mailbox
 */
void createMailbox(pid_t pid)
{
	struct mailbox new_mailbox;
	new_mailbox.pid = pid;
	new_mailbox.place = 0;
	mailbox_table[num_mailboxes] = new_mailbox;
	num_mailboxes++;
}

/* function to delete a mailbox
 * param pid -> process of mailbox to be deleted
 * return i -> place in mailbox_table that was deleted
 */
int deleteMailbox(pid_t pid)
{
	int i;
	for(i = 0; i < NUM_MAILBOXES; i++)
	{
		if(mailbox_table[i].pid == pid)
		{
			int j;
			for (j = i; j < num_mailboxes - 1; j++)
			{
				mailbox_table[j] = mailbox_table[j+1];
			}
			num_mailboxes--;
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
		{
			mailbox_table[i][j] = NULL;
		}
	}
	
	return 0;
}




asmlinkage long sys_mailbox_send(struct send_info *info)
{
	struct send_info kinfo;

	if(copy_from_user(&kinfo, info, sizeof(kinfo)))
	{
		return MSG_ARG_ERROR;
	}
		
	if(kinfo.len > MAX_MSG_SIZE || kinfo.len < 0)
	{
		return MSG_LENGTH_ERROR;
	}
		
	if(kinfo.block == TRUE)
	{
		return MAILBOX_STOPPED;
	}
	
	return 0;
}



asmlinkage long sys_mailbox_rcv(struct rcv_info *info)
{
	struct rcv_info kinfo;
	
	if((num_mailboxes == 0) || (getMailbox(current.pid) == MAILBOX_INVALID))
	{
		createMailbox(current.pid);
	}
	

	if(copy_from_user(&kinfo, info, sizeof(kinfo)))
	{
		return MSG_ARG_ERROR;
	}
		
	if(kinfo.block == TRUE)
	{
		return MAILBOX_STOPPED;
	}
	
	return 0;
}




asmlinkage long sys_mailbox_manage(struct manage_info *info)
{
	struct manage_info kinfo;

	if(copy_from_user(&kinfo, info, sizeof(kinfo)))
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

