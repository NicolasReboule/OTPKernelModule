#include "../include/device.h"

const struct file_operations hotp_fops = {
    .owner = THIS_MODULE,
	.read = hotp_reader,
	.write = hotp_writer,
};

const struct file_operations totp_fops = {
    .owner = THIS_MODULE,
	.read = totp_reader,
	.write = totp_writer,
};

const struct file_operations validator_fops = {
    .owner = THIS_MODULE,
	.read = validator_reader,
	.write = validator_writer,
};

char HOTP_algo[16] = HMAC_SHA1;
char HOTP_secret[64] = "hotp_secret";

char TOTP_algo[16] = HMAC_SHA1;
char TOTP_secret[64] = "totp_secret";
int TOTP_timestep = 30;

static char validate_str[32];

const struct file_operations device_fops[DEVICE_COUNT] = {hotp_fops, totp_fops, validator_fops};
const char *device_names[DEVICE_COUNT] = {"hotp", "totp", "validator"};
struct cdev cdevs[DEVICE_COUNT];
struct device *devices[DEVICE_COUNT];
dev_t dev_num;
int major = 64;
struct class *otp_class;

static char *device_devnode(const struct device *dev, umode_t *mode)
{
    if (mode)
        *mode = PERMISSIONS;
    return NULL;
}

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
    otp_class->devnode = device_devnode;

    for (i = 0; i < DEVICE_COUNT; i++) {
            cdev_init(&cdevs[i], &device_fops[i]);
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

ssize_t hotp_reader(struct file *f, char *buf, size_t len, loff_t *offset)
{
    size_t size;
    char str[32];
    int code;
    char counter_buf[COUNTER_MAX_SIZE];

    if (*offset > 0)
        return 0;

    set_counter_buffer(counter, counter_buf);
    counter++;
    code = hotp_algo(HOTP_algo, HOTP_secret, counter_buf);
    size = snprintf(str, 32, "%d\n", code);
    add_otp_code(code, 0);
	return simple_read_from_buffer(buf, len, offset, str, size);
}

ssize_t hotp_writer(struct file *f, const char *buf, size_t len, loff_t *offset)
{
    char input[128];
    char algo[16] = {0};
    char secret[64] = {0};
    char *token;
    bool is_SHA1 = false;
    bool is_SHA256 = false;

    char old_algo[16];
    char old_secret[64];

    strcpy(old_algo, HOTP_algo);
    strcpy(old_secret, HOTP_secret);

    if (len >= sizeof(input)) {
        pr_err("Input too long\n");
        return -EINVAL;
    }

    if (copy_from_user(input, buf, len)) {
        pr_err("Failed to copy input from user\n");
        return -EFAULT;
    }

    input[len] = '\0';
    input[strcspn(input, "\n")] = '\0';

    // Parse the input
    if (strncmp(input, "show", 4) == 0) {
        pr_info("HOTP: algo=(%s), secret=(%s)\n", HOTP_algo, HOTP_secret);
        return len;
    } else if (strncmp(input, "update", 6) == 0) {
        token = strstr(input, "-a ");
        if (token) {
            sscanf(token + 3, "%15s", algo);
        }

        token = strstr(input, "-s ");
        if (token) {
            sscanf(token + 3, "%63s", secret);
        }

        if (strlen(algo) == 0 || strlen(secret) == 0) {
            pr_info(HOTP_HELP_MESSAGE);
            return -EINVAL;
        }

        if (strcmp(algo, "SHA1") == 0) {
            is_SHA1 = true;
            strcpy(HOTP_algo, HMAC_SHA1);
        } else if (strcmp(algo, "SHA256") == 0) {
            is_SHA256 = true;
            strcpy(HOTP_algo, HMAC_SHA256);
        }

        if (is_SHA1 == false && is_SHA256 == false) {
            pr_info("Invalid algorithm. Use SHA1 or SHA256.\n");
            return -EINVAL;
        }

        if (strlen(secret) > 64) {
            pr_info("Secret too long. Max length is 64 characters.\n");
            return -EINVAL;
        }
        strncpy(HOTP_secret, secret, sizeof(HOTP_secret) - 1);
        pr_info("Updated HOTP: algo=(%s => %s), secret=(%s => %s)\n", old_algo, HOTP_algo, old_secret, HOTP_secret);
    } else {
        pr_info(HOTP_HELP_MESSAGE);
        return -EINVAL;
    }

	return len;
}

ssize_t totp_reader(struct file *f, char *buf, size_t len, loff_t *offset)
{
    size_t size;
    char str[32];
    int code;
    char counter_buf[COUNTER_MAX_SIZE];
    unsigned long current_time;
    unsigned long step;

    if (*offset > 0)
        return 0;
    current_time = jiffies_to_msecs(jiffies) / 1000;
    step = current_time / TOTP_timestep;
    set_counter_buffer(step, counter_buf);
    code = hotp_algo(TOTP_algo, TOTP_secret, counter_buf);
    size = snprintf(str, 32, "%d\n", code);
	return simple_read_from_buffer(buf, len, offset, str, size);
}

ssize_t totp_writer(struct file *f, const char *buf, size_t len, loff_t *offset)
{
    char input[128];
    char algo[16] = {0};
    char secret[64] = {0};
    char timestep[3] = {0};
    char *token;
    bool is_SHA1 = false;
    bool is_SHA256 = false;

    char old_algo[16];
    char old_secret[64];
    int old_timestep = TOTP_timestep;

    strcpy(old_algo, HOTP_algo);
    strcpy(old_secret, HOTP_secret);

    if (len >= sizeof(input)) {
        pr_err("Input too long\n");
        return -EINVAL;
    }

    if (copy_from_user(input, buf, len)) {
        pr_err("Failed to copy input from user\n");
        return -EFAULT;
    }

    input[len] = '\0';
    input[strcspn(input, "\n")] = '\0';

    // Parse the input
    if (strncmp(input, "show", 4) == 0) {
        pr_info("TOTP: algo=(%s), secret=(%s)\n", HOTP_algo, HOTP_secret);
    } else if (strncmp(input, "update", 6) == 0) {
        token = strstr(input, "-a ");
        if (token) {
            sscanf(token + 3, "%15s", algo);
        }

        token = strstr(input, "-s ");
        if (token) {
            sscanf(token + 3, "%63s", secret);
        }

        token = strstr(input, "-t ");
        if (token) {
            sscanf(token + 3, "%3s", timestep);
        }

        if (strlen(algo) == 0 || strlen(secret) == 0) {
            pr_info(TOTP_HELP_MESSAGE);
            return -EINVAL;
        }

        if (strcmp(algo, "SHA1") == 0) {
            is_SHA1 = true;
            strcpy(HOTP_algo, HMAC_SHA1);
        } else if (strcmp(algo, "SHA256") == 0) {
            is_SHA256 = true;
            strcpy(HOTP_algo, HMAC_SHA256);
        }

        if (is_SHA1 == false && is_SHA256 == false) {
            pr_err("Invalid algorithm. Use SHA1 or SHA256.\n");
            return -EINVAL;
        }

        if (strlen(secret) > 64) {
            pr_err("Secret too long. Max length is 64 characters.\n");
            return -EINVAL;
        }
        strncpy(HOTP_secret, secret, sizeof(HOTP_secret) - 1);

        if (strlen(timestep) > 0) {
            int timestep_value = 0;
            if (kstrtoint(timestep, 10, &timestep_value) != 0 || timestep_value <= 0) {
                pr_err("Invalid timestep value. Must be a positive integer.\n");
                return -EINVAL;
            }
            TOTP_timestep = timestep_value;
        }
        pr_info("Updated TOTP:\nalgo=(%s => %s)\nsecret=(%s => %s)\ntimestep=(%d => %d)\n", old_algo, HOTP_algo, old_secret, HOTP_secret, old_timestep, TOTP_timestep);
    } else {
        pr_info(TOTP_HELP_MESSAGE);
        return -EINVAL;
    }

	return len;
}

static bool validate_hotp(int code) {
    code_node *node = find_otp_code(code, 0);
    if (!node)
        return false;
    delete_otp_code(code, 0);
    return true;
}

static bool validate_totp(int code) {
    int validate;
    char counter_buf[COUNTER_MAX_SIZE];
    unsigned long current_time;
    unsigned long step;

    current_time = jiffies_to_msecs(jiffies) / 1000;
    step = current_time / TOTP_timestep;
    set_counter_buffer(step, counter_buf);
    validate = hotp_algo(TOTP_algo, TOTP_secret, counter_buf);
    return validate == code;
}

ssize_t validator_reader(struct file *f, char *buf, size_t len, loff_t *offset)
{
	return simple_read_from_buffer(buf, len, offset, validate_str, strlen(validate_str));
}

ssize_t validator_writer(struct file *f, const char *buf, size_t len, loff_t *offset)
{
    char input[32];
    int code;

    if (len > 32) {
        pr_err("Failed to validate otp: input is longer than max size\n");
        return -EINVAL;
    }

    if (copy_from_user(input, buf, len)) {
        pr_err("Failed to copy input from user\n");
        return -EFAULT;
    }

    input[len] = '\0';
    input[strcspn(input, "\n")] = '\0';

    if (strcmp(input, "help") == 0) {
        pr_info("Usage: <otp> - Check if otp is valid\n");
        return len;
    }

    if (kstrtoint(input, 10, &code) < 0) {
        strcpy(validate_str, "0\n");
        pr_err("Failed to convert arg to int\n");
        return -EINVAL;
    }

    if (!validate_hotp(code) && !validate_totp(code)) {
        strcpy(validate_str, "0\n");
    } else {
        strcpy(validate_str, "1\n");
    }
	return len;
}
