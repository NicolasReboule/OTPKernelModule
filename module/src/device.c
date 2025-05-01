#include "../include/device.h"

const char *device_names[DEVICE_COUNT] = {"hotp", "totp", "validator"};
struct cdev cdevs[DEVICE_COUNT];
struct device *devices[DEVICE_COUNT];
dev_t dev_num;
int major = 64;
struct class *otp_class;

int create_devices(void)
{
    int ret, i;

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
    return -1;
}

void destroy_devices(void)
{
    for (int i = 0; i < DEVICE_COUNT; i++) {
        device_destroy(otp_class, dev_num + i);
        cdev_del(&cdevs[i]);
    }
    class_destroy(otp_class);
    unregister_chrdev_region(dev_num, DEVICE_COUNT);
}

otp *create_otp_device(unsigned int index, struct otp_ioctl_create_s *otp_data)
{
    otp *new_device;
    dev_t devt;
    struct device *device;

    new_device = create_otp(otp_data, false);
    // Initialize the OTP device structure

    // Assign a unique minor number for the device
    devt = MKDEV(major, index);

    // Initialize the character device
    cdev_init(&new_device->cdev, &otp_fops);
    new_device->cdev.owner = THIS_MODULE;

    if (cdev_add(&new_device->cdev, devt, 1) < 0) {
        pr_err("Failed to add cdev for OTP device\n");
        kfree(new_device);
        return NULL;
    }

    char *device_type = otp_data->is_totp ? "totp" : "hotp";

    // Create the device node in /dev
    device = device_create(otp_class, NULL, devt, NULL, "%s%d", device_type, index);
    if (IS_ERR(device)) {
        pr_err("Failed to create device node for OTP device\n");
        cdev_del(&new_device->cdev);
        kfree(new_device);
        return NULL;
    }

    pr_info("Created OTP device /dev/otp%d\n", index);
    return new_device;
}

void delete_otp_device(otp *dev)
{
    if (!dev) {
        pr_err("Invalid OTP device to delete\n");
        return;
    }

    // Remove the device node from /dev
    device_destroy(otp_class, MKDEV(major, dev->index));

    // Delete the character device
    cdev_del(&dev->cdev);

    // Free the allocated memory
    kfree(dev);

    pr_info("Deleted OTP device /dev/otp%d\n", dev->index);
}