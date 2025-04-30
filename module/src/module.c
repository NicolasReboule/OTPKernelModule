#include "../include/module.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SysMy|");
MODULE_DESCRIPTION("OTP Module Manager");

static int __init otp_init_module(void)
{
        if (initialize_otp_node_system() != 0) {
                pr_err("Failed to initialize OTP system\n");
                return -1;
        }

        const int code = hotp_algo(HMAC_SHA1, "secret", "1");

        if (code < 0) {
                pr_err("Failed to generate OTP code\n");
                return -1;
        }

        pr_info("Generated OTP code: %d\n", code);

        add_otp_code(code, 0);

        code_node *check_code = find_otp_code(code, 0);
        if (check_code) {
                pr_info("Found OTP code %u for device %u\n", check_code->code, check_code->device_index);
        } else {
        pr_err("Code not found\n");
        }

        pr_info("Loading OTP Module Manager\n");


        pr_info("OTP Module Manager loaded successfully\n");
        return 0;
}

static void __exit otp_exit_module(void)
{
        // otp_node *node, *tmp;

        pr_info("Unloading OTP Module Manager\n");

        delete_all_otp_codes();

        // mutex_lock(&otp_otp_device_lock);
        // list_for_each_entry_safe(node, tmp, &device_list, list) {
        //         list_del(&node->list);
        //         device_destroy(otp_class, MKDEV(major, node->index));
        //         cdev_del(&node->dev.cdev);
        //         kfree(node);
        // }
        // mutex_unlock(&otp_otp_device_lock);

        // class_destroy(otp_class);
        // unregister_chrdev_region(MKDEV(major, 0), MAX_OTP_DEVICES);

        pr_info("OTP Module Manager unloaded\n");
}

module_init(otp_init_module);
module_exit(otp_exit_module);