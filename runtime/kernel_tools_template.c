#include <linux/printk.h>

/* this function can be hooked from a pass for example */
__used noinline void kernel_tools_hook_test(void) {
  printk(KERN_INFO "KERNEL TOOLS HOOK TEST");
}

/* this function will be called at boot time */
int kernel_tools_test(void) {
    printk(KERN_INFO "KERNEL TOOLS TEST");
    return 0;
}
__initcall(kernel_tools_test);
