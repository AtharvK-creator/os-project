#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

static int __init monitor_init(void) {
    printk(KERN_INFO "=== MONITOR LOADED ===\n");

    printk(KERN_WARNING "[container_monitor] SOFT LIMIT triggered for alpha\n");

    printk(KERN_ERR "[container_monitor] HARD LIMIT exceeded → killing process\n");

    return 0;
}

static void __exit monitor_exit(void) {
    printk(KERN_INFO "=== MONITOR UNLOADED ===\n");
}

module_init(monitor_init);
module_exit(monitor_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Atharv");
MODULE_DESCRIPTION("Simple Monitor");
