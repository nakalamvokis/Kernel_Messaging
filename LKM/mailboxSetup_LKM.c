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
#include <linux/slab.h>
#include <linux/unistd.h>
#include <linux/mailbox.h>
#include <linux/types.h>
#include <linux/pid.h>
#include <linux/spinlock.h>


#define MAILBOX_SIZE 20

// struct to be passed as parameter for send and recieve message syscall
typedef struct message_info
{
	pid_t *sender;
	pid_t dest;
	void *msg;
	int len;
	bool block;
} message_info;


// mailbox structure to be used for each process receiving messages
typedef struct mailbox
{
	pid_t pid;
	spinlock_t mlock;
	int count;
	bool stop;
	message_info messages[MAILBOX_SIZE];
} mailbox;


// struct to be passed as parameter for manage syscall
typedef struct manage_info
{
	bool stop;
	int *count;
} manage_info;


typedef struct list_node
{
	mailbox* box;
	pid_t pid;
	struct list_node* next_node;
} list_node;


typedef struct hash_table
{
	list_node *head;
} hash_table;

unsigned long **sys_call_table;

asmlinkage long (*ref_sys_cs3013_syscall1)(void);
asmlinkage long (*ref_sys_cs3013_syscall2)(void);
asmlinkage long (*ref_sys_cs3013_syscall3)(void);

static struct kmem_cache* kcache;
static hash_table h;
 
int createMailbox(pid_t pid);
mailbox* getMailbox(pid_t pid);
int deleteMailbox(pid_t pid);

int addMessage(mailbox* m, message_info* info);
message_info* getMessage(mailbox* m);
int deleteMessage(mailbox* m);



// HASH TABLE FUNCTIONS

/* function to create new mailbox
 * param pid -> pid of process to be assigned a mailbox
 */
int createMailbox(pid_t pid)
{
	list_node* node_ptr;
	mailbox* new_mailbox = kmem_cache_alloc(kcache, GFP_KERNEL);
	new_mailbox->pid = pid;
	new_mailbox->count = 0;

	node_ptr = h.head;
	while(node_ptr != NULL)
	{
		node_ptr = node_ptr->next_node;
	}

	node_ptr->box = new_mailbox;

	return 0;
}



/* function to get mailbox in the mailbox hash table
 * param pid -> process id of the process to recieve a message
 * return a pointer to a mailbox
 */
mailbox* getMailbox(pid_t pid)
{
	list_node* node_ptr;
	node_ptr = h.head;

	while(node_ptr->pid != pid)
	{
		if (node_ptr == NULL)
		{
			return NULL;
		}
		node_ptr = node_ptr->next_node;
	}

	return node_ptr->box;
}


/* function to delete a mailbox
 * param pid -> process of mailbox to be deleted
 */
int deleteMailbox(pid_t pid)
{
	mailbox* m = getMailbox(pid);
	list_node* node_ptr;
	node_ptr = h.head;

	if (m == NULL)
	{
		return MAILBOX_INVALID;
	}

	kmem_cache_free(kcache,m);

	while(node_ptr != NULL)
	{
		if (node_ptr->next_node->pid == pid)
		{
			node_ptr->next_node = node_ptr->next_node->next_node;
			return 0;
		}
		node_ptr = node_ptr->next_node;
	}

	return MAILBOX_INVALID;
}


// MAILBOX FUNCTIONS


/* function to add a mailbox to the hash table
 * param m -> mailbox to receive message
 * param info -> message to add to mailbox
 */
int addMessage(mailbox* m, message_info* info)
{
	if(m->count == MAILBOX_SIZE)
	{
		return MAILBOX_FULL;
	}

	m->messages[m->count] = *info;
	m->count++;
	return 0;
}


/* function to get a message from a mailbox
 * param m -> mailbox to recieve message
 * return a pointer to a message_info struct
 */
message_info* getMessage(mailbox* m)
{
	if(m->count == 0)
	{
		return NULL;
	}

	return &(m->messages[0]);
}

/* function to delete a message
 * param m -> mailbox to be edited
 */
int deleteMessage(mailbox* m)
{
	int i;
	for(i = 0; i < m->count; i++)
	{
		m->messages[i] = m->messages[i+1];
	}
	m->count--;
	return 0;
}


asmlinkage long sys_mailbox_send(struct message_info *info)
{
	message_info kinfo;
	pid_t pid = kinfo.dest;
	*(kinfo.sender) = getpid();
	
	if(copy_from_user(&kinfo, info, sizeof(kinfo)))
		return MSG_ARG_ERROR;
	
	if(kinfo.block == true)
		return MAILBOX_STOPPED;
		
	if(kinfo.len > MAX_MSG_SIZE || kinfo.len < 0)
		return MSG_LENGTH_ERROR;

	if(getMailbox(pid) == NULL)
		return MAILBOX_INVALID;
	
	if(getMailbox(*(kinfo.sender)) == NULL)
		createMailbox(*(kinfo.sender));
	
	mailbox* m = getMailbox(pid);
	
	// add message to mailbox
	addMessage(m, &kinfo);
	
	return 0;
}



asmlinkage long sys_mailbox_rcv(struct message_info *info)
{
	message_info kinfo;
	pid_t pid = getpid();
	

	if (copy_from_user(&kinfo, info, sizeof(kinfo)))
		return MSG_ARG_ERROR;

	if (kinfo.block == true)
		return MAILBOX_STOPPED;

	if (getMailbox(pid) == NULL)
		createMailbox(pid);
	
	
	mailbox* m = getMailbox(pid);
	
	// get a message_info
	kinfo = *(getMessage(m));

	if(copy_to_user(&info, &kinfo, sizeof(kinfo)))
	{
		return MAILBOX_ERROR;
	}
	
	// delete sent message from mailbox
	deleteMessage(m);

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
	

	mailbox* m = getMailbox(pid);
	if (m == NULL)
	{
		createMailbox(pid);
		m = getMailbox(pid);
	}
	m->stop = kinfo.stop;
	*(kinfo.count) = m->count;
	
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
	
	kcache = kmem_cache_create("Mailboxes", sizeof(mailbox), 0, 0, NULL);
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

	kmem_cache_destroy(kcache)
	
	printk(KERN_INFO "Unloaded interceptor!");
}	// static void __exit interceptor_end(void)

MODULE_LICENSE("GPL");
module_init(interceptor_start);
module_exit(interceptor_end);

