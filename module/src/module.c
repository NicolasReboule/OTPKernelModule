#include "../include/module.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SysMy|");
MODULE_DESCRIPTION("OTP Module Manager");

static int __init otp_init_module(void)
{
        int ret;

        pr_info("Loading OTP Module Manager\n");

        ret = init_debugfs();
        if (ret < 0) {
                return ret;
        }

        ret = create_devices();
        return ret;
}

static void __exit otp_exit_module(void)
{
        pr_info("Unloading OTP Module Manager\n");

        destroy_debugfs();
        destroy_devices();
        
        pr_info("OTP Module Manager unloaded\n");
}

module_init(otp_init_module);
module_exit(otp_exit_module);