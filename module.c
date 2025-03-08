#include <linux/module.h>
#include <linux/printk.h>

#include "crypto.c"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SysMy|");
MODULE_DESCRIPTION("OTP Module Manager");

/**
 * @brief Initialize the module with required components.
 * Components:
 * - Sysfs (interface)
 * - Procfs (user friendly)
 * - Devices (interaction)
 *
 * @return int Return Code (0 success, -1 error)
 */
static int __init otp_init_module(void)
{
        pr_info("Loading OTP Module Manager");
        // hotp_algo("mykey", "password");
        return 0;
}

/**
 * @brief Unload the module and used components.
 *
 */
static void __exit otp_exit_module(void)
{
        pr_info("Unload OTP Module Manager");
}

module_init(otp_init_module);
module_exit(otp_exit_module);