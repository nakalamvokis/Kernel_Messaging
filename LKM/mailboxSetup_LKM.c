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
#include <asm/current.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/unistd.h>
#include <mailbox.h>
#include <linux/types.h>
#include <linux/pid.h>
#include <linux/spinlock.h>


#define MAILBOX_SIZE 20


// wait_queue_head_t waitqueue;

//blah

// struct to be passed as parameter for send and recieve message syscall
typedef struct message_info
{
	pid_t sender;
	pid_t dest;
	char* msg;
	int len;
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

/*asmlinkage void (*ref_sys_exit)(long code);
asmlinkage void (*ref_sys_exit_group)(int exit_code);*/

static struct kmem_cache* mcache;
static struct kmem_cache* lcache;
static struct kmem_cache* msgcache;

static hash_table h;
 
int createMailbox(pid_t pid);
mailbox* getMailbox(pid_t pid);
int deleteMailbox(pid_t pid);

int addMessage(mailbox* m, pid_t dest, pid_t sender, void* msg, int len);
message_info* getMessage(mailbox* m);
int deleteMessage(mailbox* m);



// HASH TABLE FUNCTIONS

/* function to create new mailbox
 * pa
ram pid -> pid of process to be assigned a mailbox
 */
int createMailbox(pid_t pid)
{
	list_node* node_ptr;
	list_node* new_node = kmem_cache_alloc(lcache, GFP_KERNEL);
	mailbox* new_mailbox = kmem_cache_alloc(mcache, GFP_KERNEL);
	
	if (h.head == NULL)
	{
		h.head = new_node;
	}
	
	else 
	{
		//printk("Node is not head!\n");
		node_ptr = h.head;
		while(node_ptr->next_node != NULL)
		{
			node_ptr = node_ptr->next_node;
		}
		node_ptr->next_node = new_node;
		//printk("Found node!\n");
	}
	
	spin_lock_init(&new_mailbox->mlock);
	if (current->mm == NULL)
	{
		new_node->box = NULL;
		new_node->pid = pid;
		new_node->next_node = NULL;
		kmem_cache_free(mcache, new_mailbox);
	}
	else
	{
		new_mailbox->pid = pid;
		new_mailbox->count = 0;
		new_mailbox->stop = false;
	
		new_node->box = new_mailbox;
		new_node->pid = pid;
		new_node->next_node = NULL;
	}
	//printk("Set node!\n");

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

	if (node_ptr == NULL)
	{
		return NULL;
	}
		
	
	while(node_ptr->pid != pid)
	{
	    node_ptr = node_ptr->next_node;
	    
	    if (node_ptr == NULL)
		{
			return NULL;
		}
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
	list_node* oldnode;
	
	node_ptr = h.head;
		
	if (m == NULL)
		return MAILBOX_INVALID;
	
	spin_lock(&m->mlock);
	if (node_ptr->next_node == NULL)
	{
		h.head == NULL;
		kmem_cache_free(lcache, node_ptr);
		spin_unlock(&m->mlock);
		kmem_cache_free(mcache, m);
		return 0;
	}
	
	while(node_ptr != NULL)
	{
		if (node_ptr->next_node->pid == pid)
		{
			oldnode = node_ptr->next_node;
			node_ptr->next_node = node_ptr->next_node->next_node;
			kmem_cache_free(lcache, oldnode);
			spin_unlock(&m->mlock);
			kmem_cache_free(mcache, m);
			return 0;
		}
		node_ptr = node_ptr->next_node;
	}
	spin_unlock(&m->mlock);
	return MAILBOX_INVALID;
}


// MAILBOX FUNCTIONS


/* function to add a mailbox to the hash table
 * param m -> mailbox to receive message
 * param info -> message to add to mailbox
 */
int addMessage(mailbox* m, pid_t dest, pid_t sender, void* msg, int len)
{	
	spin_lock(&m->mlock);
	
	if(m->count == (MAILBOX_SIZE - 1))
	{
		return MAILBOX_FULL;
	}
	m->messages[m->count].dest = dest;
	m->messages[m->count].sender = sender;
	m->messages[m->count].len = len;
	m->messages[m->count].msg = kmem_cache_alloc(msgcache, GFP_KERNEL);
	strncpy(m->messages[m->count].msg, (char *)msg, len);
	
	printk("Added a message: %s\n", (char *) m->messages[m->count].msg);
	
	m->count++;
	spin_unlock(&m->mlock);
	
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
	
	printk("Got a message: %s\n", (char *) m->messages[0].msg);

	return &(m->messages[0]);
}

/* function to delete a message
 * param m -> mailbox to be edited
 */
int deleteMessage(mailbox* m)
{
	int i;
	
	if (m->count == 0)
		return MAILBOX_EMPTY;
	
	spin_lock(&m->mlock);
	kmem_cache_free(msgcache, m->messages[m->count - 1].msg);
	if (m->count == 1)
	{
		m->messages[0] == NULL;
	}
	else
	{
		for(i = 0; i < m->count; i++)
		{
			//printk(".");
			m->messages[i] = m->messages[i+1];
		}
	}
	m->count--;
	spin_unlock(&m->mlock);
	
	return 0;
}


asmlinkage long sys_mailbox_send(pid_t dest, void *msg, int len, bool block)
{
	mailbox* m;
	pid_t pid;/*, send_dest;
	void *send_msg;
	int send_len;
	bool send_block;
	unsigned long status;*/
	
	pid = current->pid;

	/*
	if((status = copy_from_user(&send_dest, &dest, sizeof())))
	{
		printk("Made it past dest test %lu,  %d!\n", status, send_dest);
		return MSG_ARG_ERROR;
	}
		
	printk("Made it past dest test!\n");
		
	if(copy_from_user(send_msg, msg, sizeof(void *)))
		return MSG_ARG_ERROR;
		
	printk("Made it past msg test!\n");
		
	if(copy_from_user(&send_len, &len, sizeof(len)))
		return MSG_ARG_ERROR;
		
	printk("Made it past len test!\n");
	
	if(copy_from_user(&send_block, &block, sizeof(bool)))
		return MSG_ARG_ERROR;
		
	printk("Made it past block test!\n");*/
		
	if (getMailbox(pid) == NULL)
	{
		createMailbox(pid);
		printk("Created mailbox for process %d!\n", pid);
	}
	
	if(getMailbox(dest) == NULL)
	{
		/*return MAILBOX_INVALID;*/
		createMailbox(dest);
	}
	//printk(KERN_INFO "Mailbox valid!\n");

	if(len > MAX_MSG_SIZE || len < 0)
		return MSG_LENGTH_ERROR;
		
	//printk(KERN_INFO "Message of right size.");


	m = getMailbox(dest);
	
	spin_lock(&m->mlock);
	
	if((m->count == MAILBOX_SIZE -1)||(block == true))
	{
		m->stop = true;
	}
	
	/*if(m->stop == true)
	{
		DEFINE_WAIT(wait);
		add_wait_queue(m->waitqueue, &wait);
		prepare_to_wait(&(m->waitqueue), &wait, TASK_INTERRUPTIBLE);
		
		while(m->stop == true)
		{
			spin_unlock(&m->mlock);
			usleep(10);
			spin_lock(&m->mlock);
			if(m->count < MAILBOX_SIZE - 1)
				break;
		}
		finish_wait(&(m->waitqueue), &wait);
		spin_unlock(&m->mlock);	
	}*/

	//printk(KERN_INFO "Mailbox not stopped!\n");
	
	
	
	spin_unlock(&m->mlock);
	addMessage(m, dest, pid, msg, len);
	
	printk("Sent a message: %s\n", (char *) m->messages[0].msg);
	
	return 0;
}



asmlinkage long sys_mailbox_rcv(pid_t *sender, void *msg, int *len, bool block)
{
	pid_t pid;
	mailbox* m;
	/*
	pid_t rcv_sender;
	void *rcv_msg = (void *) "Uninitialized message";
	int rcv_len;
	bool rcv_block;*/
	message_info *rcv_message;
	char* mesg;
	/*
	if(copy_from_user(&rcv_sender, sender, sizeof(pid_t)))
		return MSG_ARG_ERROR;
		
	if(copy_from_user(rcv_msg, msg, sizeof(void *)))
		return MSG_ARG_ERROR;
		
	if(copy_from_user(&rcv_len, len, sizeof(int)))
		return MSG_ARG_ERROR;
	
	if(copy_from_user(&rcv_block, &block, sizeof(bool)))
		return MSG_ARG_ERROR;
	*/
	
	pid = current->pid;
		
	if (getMailbox(pid) == NULL)
	{
		createMailbox(pid);
		printk("Created mailbox for process %d!\n", pid);
	}

	//printk(KERN_INFO "Started receiving!\n");

	m = getMailbox(pid);
	
	if (block == true)
	{
		return MAILBOX_STOPPED;
	}
	//printk(KERN_INFO "Mailbox not stopped!\n");
	
	spin_lock(&m->mlock);
	
	if(m->count == 0)
		return MAILBOX_EMPTY;
	
	// get a message_info
	rcv_message = getMessage(m);
	
	printk("Received a message: %s\n", (char *) rcv_message->msg);
	
	spin_unlock(&m->mlock);
	
	if(copy_to_user(sender, &(rcv_message->sender), sizeof(pid_t)))
		return MSG_ARG_ERROR;
			
	if(copy_to_user(len, &(rcv_message->len), sizeof(int)))
		return MSG_ARG_ERROR;
		
	if(copy_to_user(msg, (char *) rcv_message->msg;, sizeof(char) * (*len)))
		return MSG_ARG_ERROR;
		
	deleteMessage(m);
	
	//spin_unlock(&(m->mlock));
	
	return 0;
}




asmlinkage long sys_mailbox_manage(bool stop, int *count)
{
	pid_t pid;
	mailbox* m;
	/*
	bool manage_stop;
	int manage_count;
	*/
	
	
	
	pid = current->pid;
	/*
	if(copy_from_user(&manage_stop, &stop, sizeof(bool)))
		return MSG_ARG_ERROR;
	
	if(copy_from_user(&manage_count, count, sizeof(int)))
		return MSG_ARG_ERROR;
		*/
		
	if (getMailbox(pid) == NULL)
	{
		createMailbox(pid);
		printk("Created mailbox for process %d!", pid);
	}

	m = getMailbox(pid);
	spin_lock(&m->mlock);
	printk("Got mailbox for pid %d!\n", pid);
	
	
	/* //try spinlock
	while(spin_trylock(&(m->mlock)) == 0)
	{
	// wait for lock of spinlock
	}
	
	*/
	
	m->stop = stop;
	//printk("Managed stop.\n");
	
	*count = m->count;
	
	// spin_unlock(&(m->mlock)); // unlock spinlock
	
	//printk("There are %d messages in the mailbox.", *count);
	
	/*if(copy_to_user(count, &manage_count, sizeof(int)))
	{
		return MSG_ARG_ERROR;
	}*/
	
	spin_unlock(&m->mlock);
	
	return 0;
}
/*
asmlinkage void sys_new_exit(long code)
{
	pid_t pid;
	pid = current->pid;
	
	deleteMailbox(pid);

	printk("Deleted mailbox for pid %d!\n", pid);
	ref_sys_exit(code);
}
asmlinkage void sys_new_exit_group(int exit_code)
{
	pid_t pid;
	pid = current->pid;
	
	deleteMailbox(pid);

	printk("Deleted mailbox for group pid %d!\n", pid);
	ref_sys_exit_group(exit_code);
} */


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
	
	/*ref_sys_exit = (void *)sys_call_table[__NR_exit];
	ref_sys_exit = (void *)sys_call_table[__NR_exit_group];*/

	/* Replace the existing system calls */
	disable_page_protection();

	sys_call_table[__NR_cs3013_syscall1] = (unsigned long *)sys_mailbox_send;
	sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)sys_mailbox_rcv;
	sys_call_table[__NR_cs3013_syscall3] = (unsigned long *)sys_mailbox_manage;

	/*sys_call_table[__NR_exit] = (unsigned long *)sys_new_exit;
	sys_call_table[__NR_exit_group] = (unsigned long *)sys_new_exit_group;*/
	enable_page_protection();
	
	mcache = kmem_cache_create("Mailboxes", sizeof(mailbox), 0, 0, NULL);
	lcache = kmem_cache_create("Hash Tables Entries", sizeof(list_node), 0, 0, NULL);
	msgcache = kmem_cache_create("Message", sizeof(char) * MAX_MSG_SIZE, 0, 0, NULL);
	
	/* And indicate the load was successful */
	printk(KERN_INFO "Loaded interceptor!\n");

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
	
	/*sys_call_table[__NR_exit] = (unsigned long *)ref_sys_exit;
	sys_call_table[__NR_exit_group] = (unsigned long *)ref_sys_exit_group;*/
	enable_page_protection();


	kmem_cache_destroy(mcache);
	kmem_cache_destroy(lcache);
	kmem_cache_destroy(msgcache);
	printk(KERN_INFO "Unloaded interceptor!\n");
}	// static void __exit interceptor_end(void)

MODULE_LICENSE("GPL");
module_init(interceptor_start);
module_exit(interceptor_end);

