#include "../include/lists.h"

static DEFINE_MUTEX(otp_device_lock);
static DEFINE_MUTEX(otp_code_lock);

static LIST_HEAD(otp_device_list);
static LIST_HEAD(otp_code_list);

/**
 * @brief Initialize the OTP device and code lists.
 *
 * This function initializes the linked lists for OTP devices and OTP codes,
 * and ensures the mutex locks are ready for use.
 *
 * @return 0 on success, or a negative error code on failure.
 */
int initialize_otp_node_system(void)
{
    int ret = 0;
    dev_t dev;

    ret = alloc_chrdev_region(&dev, 0, MAX_OTP_DEVICES, DEVICE_OTP_NAME);
    if (ret < 0) {
            pr_err("Failed to allocate character device numbers\n");
            return ret;
    }
    major = MAJOR(dev);

    otp_class = class_create(CLASS_OTP_NAME);
    if (IS_ERR(otp_class)) {
            unregister_chrdev_region(MKDEV(major, 0), MAX_OTP_DEVICES);
            pr_err("Failed to create device class\n");
            return PTR_ERR(otp_class);
    }

    INIT_LIST_HEAD(&otp_device_list);
    INIT_LIST_HEAD(&otp_code_list);

    mutex_init(&otp_device_lock);
    mutex_init(&otp_code_lock);

    pr_info("OTP system initialized successfully\n");
    return ret;
}

int add_otp_device(otp *dev)
{
    otp_node *node;
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

    node->otp = dev;

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
    mutex_lock(&otp_device_lock);
    list_add_tail(&node->list, &otp_device_list);
    mutex_unlock(&otp_device_lock);

    pr_info("Added OTP device to the list and created /dev/otp%d\n", dev->index);
    return 0;
}

otp *find_otp_device_by_index(unsigned int index)
{
        otp_node *node;
        otp *otp_device;

        mutex_lock(&otp_device_lock);
        list_for_each_entry(node, &otp_device_list, list) {
                if (node->otp->index == index) {
                        otp_device = node->otp;
                        break;
                }
        }
        mutex_unlock(&otp_device_lock);

        return otp_device;
}

int delete_otp_device_by_index(unsigned int index)
{
        otp_node *node, *tmp;

        mutex_lock(&otp_device_lock);
        list_for_each_entry_safe(node, tmp, &otp_device_list, list) {
                if (node->otp->index == index) {
                        list_del(&node->list);
                        kfree(node);
                        pr_info("Deleted OTP device from the list\n");
                        mutex_unlock(&otp_device_lock);
                        return 0;
                }
        }
        mutex_unlock(&otp_device_lock);

        pr_err("OTP device with index %u not found\n", index);
        return -ENOENT;
}

int add_otp_code(unsigned int code, unsigned int device_index)
{
    code_node *new_code;

    // Allocate memory for the new code
    new_code = kmalloc(sizeof(*new_code), GFP_KERNEL);
    if (!new_code) {
        pr_err("Failed to allocate memory for OTP code\n");
        return -ENOMEM;
    }

    // Initialize the code structure
    new_code->code = code;
    new_code->device_index = device_index;

    // Add the code to the list
    mutex_lock(&otp_code_lock);
    list_add_tail(&new_code->list, &otp_code_list);
    mutex_unlock(&otp_code_lock);

    pr_info("Added OTP code %u for device %u\n", code, device_index);
    return 0;
}

code_node *find_otp_code(unsigned int code, unsigned int device_index)
{
    code_node *entry;

    mutex_lock(&otp_code_lock);
    list_for_each_entry(entry, &otp_code_list, list) {
        if (entry->code == code && entry->device_index == device_index) {
            mutex_unlock(&otp_code_lock);
            return entry;
        }
    }
    mutex_unlock(&otp_code_lock);

    pr_err("OTP code %u for device %u not found\n", code, device_index);
    return NULL;
}

int delete_otp_code(unsigned int code, unsigned int device_index)
{
    code_node *entry, *tmp;

    mutex_lock(&otp_code_lock);
    list_for_each_entry_safe(entry, tmp, &otp_code_list, list) {
        if (entry->code == code && entry->device_index == device_index) {
            list_del(&entry->list);
            kfree(entry);
            pr_info("Deleted OTP code %u for device %u\n", code, device_index);
            mutex_unlock(&otp_code_lock);
            return 0;
        }
    }
    mutex_unlock(&otp_code_lock);

    pr_err("Failed to delete OTP code %u for device %u: not found\n", code, device_index);
    return -ENOENT;
}

void delete_all_otp_codes(void)
{
    code_node *entry, *tmp;

    mutex_lock(&otp_code_lock);
    list_for_each_entry_safe(entry, tmp, &otp_code_list, list) {
        list_del(&entry->list);
        kfree(entry);
    }
    mutex_unlock(&otp_code_lock);

    pr_info("Deleted all OTP codes\n");
}