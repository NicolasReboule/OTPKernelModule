// File: src/device.c
#include "../include/device.h"

extern int major;
extern struct class *otp_class;
extern struct mutex otp_device_lock;
extern struct list_head device_list;

/**
 * @brief Manage open behavior of the device
 *
 * @param counter Counter to set.
 * @param counter_buf Buffer to set.
 */
int otp_open(struct inode *inode, struct file *file)
{
        file->private_data = container_of(inode->i_cdev, otp_t, cdev);
        return 0;
}

/**
 * @brief Manage read behavior of the device
 *
 * @param file File structure
 * @param buf Buffer to read into
 * @param len Length of the buffer
 * @param off Offset in the file
 * @return ssize_t Number of bytes read
 */
ssize_t otp_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
        otp_t *dev = file->private_data;
        char code[12];
        unsigned int counter;
        int ret;

        if (!dev) {
                pr_err("Device not initialized properly\n");
                return -EFAULT;
        }

        // Determine the counter value
        if (dev->is_totp) {
                counter = ktime_get_real_seconds() / dev->timestep;
        } else {
                counter = dev->counter++;
        }

        set_counter_buffer(counter, dev->counter_buf);

        // Generate the OTP code
        ret = otp_generate_code(dev, counter, code);
        if (ret < 0) {
                pr_err("Failed to generate OTP code\n");
                return ret;
        }

        // Ensure we only read once
        if (*off > 0)
                return 0;

        // Copy the generated code to the user buffer
        if (copy_to_user(buf, code, strlen(code))) {
                pr_err("Failed to copy OTP code to user buffer\n");
                return -EFAULT;
        }

        *off += strlen(code);
        return strlen(code);
}

/**
 * @brief Manage ioctl of the device
 *
 * @param file File structure
 * @param cmd Command to execute
 * @param arg Argument for the command
 * @return long Return code (0 success, otherwise error)
 */
long otp_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        otp_t *dev = file->private_data;

        switch (cmd) {
                case OTP_IOC_SET_SECRET: {
                        secret_t user_secret;
                        if (copy_from_user(&user_secret, (void __user *)arg, sizeof(user_secret)))
                        return -EFAULT;
                        strncpy(dev->secret, user_secret.key, SECRET_MAX_LEN);
                        dev->secret_len = strnlen(user_secret.key, SECRET_MAX_LEN);
                        break;
                }
                case OTP_IOC_SET_CODE:
                        if (arg < 6 || arg > 10)
                        return -EINVAL;
                        dev->code = arg;
                        break;
                case OTP_IOC_RESET_COUNTER:
                        dev->counter = 0;
                        break;
                case OTP_IOC_SET_TIME_STEP:
                        dev->timestep = arg;
                        break;
                case OTP_IOC_CREATE_DEVICE:
                        type_t type;
                        if (copy_from_user(&type, (void __user *)arg, sizeof(type)))
                                return -EFAULT;
                        if (create_device(dev->counter, &type) < 0)
                                return -EINVAL;
                        break;
                case OTP_IOC_DELETE_DEVICE:
                        if (delete_device(arg) < 0)
                            return -EINVAL;
                        break;
                default:
                        return -EINVAL;
        }

        return 0;
}

static const struct file_operations otp_fops = {
        .owner = THIS_MODULE,
        .open = otp_open,
        .read = otp_read,
        .unlocked_ioctl = otp_ioctl,
};

int create_device(int index, type_t *type)
{
        opt_node_t *node;
        dev_t devt = MKDEV(major, index);
        struct device *device;

        // Allocate memory for the device node
        node = kzalloc(sizeof(*node), GFP_KERNEL);
        if (!node)
                return -ENOMEM;

        // Initialize the device structure
        node->dev.is_totp = type->is_totp;
        node->dev.is_verify = type->is_verify;
        node->dev.code = 6;
        node->dev.timestep = 30;
        node->index = index;

        // Initialize the character device
        cdev_init(&node->dev.cdev, &otp_fops);
        node->dev.cdev.owner = THIS_MODULE;

        // Add the character device
        if (cdev_add(&node->dev.cdev, devt, 1) < 0) {
                kfree(node);
                return -EINVAL;
        }

        // Create the device in /dev
        device = device_create(otp_class, NULL, devt, NULL,
                                type->is_totp ? "totp%d" : (type->is_verify ? "verify%d" : "hotp%d"), index);
        if (IS_ERR(device)) {
                cdev_del(&node->dev.cdev);
                kfree(node);
                return PTR_ERR(device);
        }

        // Add the device node to the linked list
        mutex_lock(&otp_device_lock);
        list_add_tail(&node->list, &device_list);
        mutex_unlock(&otp_device_lock);

        return 0;
}

int delete_device(int index)
{
        opt_node_t  *node, *tmp;

        mutex_lock(&otp_device_lock);
        list_for_each_entry_safe(node, tmp, &device_list, list) {
                if (node->index == index) {
                list_del(&node->list); // Remove from the linked list
                device_destroy(otp_class, MKDEV(major, index));
                cdev_del(&node->dev.cdev);
                kfree(node);
                mutex_unlock(&otp_device_lock);
                return 0;
                }
        }
        mutex_unlock(&otp_device_lock);

        return -ENOENT;
}

opt_node_t  *find_device_by_index(int index)
{
        opt_node_t *node;

        mutex_lock(&otp_device_lock);
        list_for_each_entry(node, &device_list, list) {
                if (node->index == index) {
                mutex_unlock(&otp_device_lock);
                return node;
                }
        }
        mutex_unlock(&otp_device_lock);

        return NULL;
}

/**
 * @brief Generate the OTP code
 *
 * @param dev OTP device structure
 * @param counter Counter to use for HOTP or TOTP
 * @param out_code Output buffer for the generated code
 * @return int Return Code (0 success, otherwise error)
 */
int otp_generate_code(otp_t *dev, unsigned int counter, char *out_code)
{
        unsigned char counter_buf[8];

        set_counter_buffer(counter, counter_buf);

        int code = hotp_algo(HMAC_SHA1, dev->secret, counter_buf);
        snprintf(out_code, dev->code + 2, "%0*d\n", dev->code, code);

        return 0;
}