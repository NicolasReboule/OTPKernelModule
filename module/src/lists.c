#include "../include/lists.h"

static DEFINE_MUTEX(device_lock);
static DEFINE_MUTEX(code_lock);

static LIST_HEAD(otp_device_list);
static LIST_HEAD(otp_code_list);



int add_otp_device(otp *dev)
{
    otp_list *node;
    dev_t devt;
    struct device *device;

    if (!dev) {
        pr_err("Invalid OTP device\n");
        return -EINVAL;
    }

        node = kmalloc(sizeof(*node), GFP_KERNEL);
    if (!node) {
        pr_err("Failed to allocate memory for list node\n");
        return -ENOMEM;
    }

    node->otp_device = dev;

    // Assign a unique minor number for the device
    devt = MKDEV(major, dev->index);

    // Initialize the character device
    cdev_init(&dev->cdev, &otp_fops);
    dev->cdev.owner = THIS_MODULE;

    if (cdev_add(&dev->cdev, devt, 1) < 0) {
        pr_err("Failed to add cdev for OTP device\n");
        kfree(node);
        return -EINVAL;
    }

    // Create the device node in /dev
    device = device_create(otp_class, NULL, devt, NULL, "otp%d", dev->index);
    if (IS_ERR(device)) {
        pr_err("Failed to create device node for OTP device\n");
        cdev_del(&dev->cdev);
        kfree(node);
        return PTR_ERR(device);
    }

    // Add the device to the linked list
    mutex_lock(&device_lock);
    list_add_tail(&node->list, &otp_device_list);
    mutex_unlock(&device_lock);

    pr_info("Added OTP device to the list and created /dev/otp%d\n", dev->index);
    return 0;
}

otp *find_otp_device_by_index(unsigned int index)
{
        otp_list *node;
        otp *otp_device = NULL;

        mutex_lock(&device_lock);
        list_for_each_entry(node, &otp_device_list, list) {
                if (node->otp_device->index == index) {
                        otp_device = node->otp_device;
                        break;
                }
        }
        mutex_unlock(&device_lock);

        return otp_device;
}

int delete_otp_device_by_index(unsigned int index)
{
        otp_list *node, *tmp;

        mutex_lock(&device_lock);
        list_for_each_entry_safe(node, tmp, &otp_device_list, list) {
                if (node->otp_device->index == index) {
                        list_del(&node->list);
                        kfree(node);
                        pr_info("Deleted OTP device from the list\n");
                        mutex_unlock(&device_lock);
                        return 0;
                }
        }
        mutex_unlock(&device_lock);

        pr_err("OTP device with index %u not found\n", index);
        return -ENOENT;
}

