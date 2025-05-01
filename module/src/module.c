#include "../include/module.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SysMy|");
MODULE_DESCRIPTION("OTP Module Manager");

static int __init otp_init_module(void)
{
        int ret, i;

        pr_info("Loading OTP Module Manager\n");

        d_opt_manager = debugfs_create_dir(DIR_NAME, NULL);
        if (!d_opt_manager || IS_ERR(d_opt_manager)) {
                pr_err("Failed to create debugfs directory\n");
                return -1;
        }
        f_opt_list = debugfs_create_file(FILE_NAME, PERMISSIONS, d_opt_manager, NULL, &debugfs_fops);
        if (!f_opt_list || IS_ERR(f_opt_list)) {
                pr_err("Failed to create debugfs file\n");
                debugfs_remove_recursive(d_opt_manager);
                return -1;   
        }

        ret = alloc_chrdev_region(&dev_num, 0, DEVICE_COUNT, DEVICE_OTP_NAME);
        if (ret < 0) {
                pr_err("Failed to allocate char device region\n");
                return ret;
        }

        otp_class = class_create(CLASS_OTP_NAME);
        if (IS_ERR(otp_class)) {
                unregister_chrdev_region(dev_num, DEVICE_COUNT);
                return PTR_ERR(otp_class);
        }

        for (i = 0; i < DEVICE_COUNT; i++) {
                cdev_init(&cdevs[i], &otp_fops);
                cdevs[i].owner = THIS_MODULE;
                ret = cdev_add(&cdevs[i], dev_num + i, 1);
                if (ret) {
                        pr_err("Failed to add cdev %d\n", i);
                        goto error;
                }
                devices[i] = device_create(otp_class, NULL, dev_num + i, NULL, "%s-%s", "otp", device_names[i]);
                if (IS_ERR(devices[i])) {
                        pr_err("Failed to create device %s\n", device_names[i]);
                        ret = PTR_ERR(devices[i]);
                        goto error;
                }
        }
        pr_info("OTP Module Manager loaded successfully\n");
        return 0;

error:
        while (--i >= 0) {
                device_destroy(otp_class, dev_num + i);
                cdev_del(&cdevs[i]);
        }
        class_destroy(otp_class);
        unregister_chrdev_region(dev_num, DEVICE_COUNT);
        return ret;
}

static void __exit otp_exit_module(void)
{
        pr_info("Unloading OTP Module Manager\n");

        debugfs_remove_recursive(d_opt_manager);

        for (int i = 0; i < DEVICE_COUNT; i++) {
                device_destroy(otp_class, dev_num + i);
                cdev_del(&cdevs[i]);
        }
        class_destroy(otp_class);
        unregister_chrdev_region(dev_num, DEVICE_COUNT);
        pr_info("OTP Module Manager unloaded\n");
}

module_init(otp_init_module);
module_exit(otp_exit_module);