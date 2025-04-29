#include "../include/module.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SysMy|");
MODULE_DESCRIPTION("OTP Module Manager");

int major;
struct class *otp_class;
LIST_HEAD(device_list);
DEFINE_MUTEX(otp_device_lock);

EXPORT_SYMBOL(otp_class);
EXPORT_SYMBOL(otp_device_lock);
EXPORT_SYMBOL(major);
EXPORT_SYMBOL(device_list);

static int __init otp_init_module(void)
{
        dev_t dev;
        int ret;

        pr_info("Loading OTP Module Manager\n");

        // Allocate character device numbers
        ret = alloc_chrdev_region(&dev, 0, MAX_OTP_DEVICES, DEVICE_OTP_NAME);
        if (ret < 0) {
                pr_err("Failed to allocate character device numbers\n");
                return ret;
        }
        major = MAJOR(dev);

        // Create device class
        otp_class = class_create(CLASS_OTP_NAME);
        if (IS_ERR(otp_class)) {
                unregister_chrdev_region(MKDEV(major, 0), MAX_OTP_DEVICES);
                pr_err("Failed to create device class\n");
                return PTR_ERR(otp_class);
        }

        pr_info("OTP Module Manager loaded successfully\n");
        return 0;
}

static void __exit otp_exit_module(void)
{
        otp_list *node, *tmp;

        pr_info("Unloading OTP Module Manager\n");

        mutex_lock(&otp_device_lock);
        list_for_each_entry_safe(node, tmp, &device_list, list) {
                list_del(&node->list);
                device_destroy(otp_class, MKDEV(major, node->index));
                cdev_del(&node->dev.cdev);
                kfree(node);
        }
        mutex_unlock(&otp_device_lock);

        class_destroy(otp_class);
        unregister_chrdev_region(MKDEV(major, 0), MAX_OTP_DEVICES);

        pr_info("OTP Module Manager unloaded\n");
}

module_init(otp_init_module);
module_exit(otp_exit_module);