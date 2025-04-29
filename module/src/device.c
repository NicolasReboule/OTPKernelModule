#include "../include/device.h"

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

    char *device_type = is_totp ? "totp" : "hotp";

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