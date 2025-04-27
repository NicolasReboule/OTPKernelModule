#include "../include/otp.h"

otp *create_otp(struct otp_ioctl_create_s *create_otp_data, bool is_validate = false)
{
        char *algo = create_otp_data->algo;
        char *secret = create_otp_data->secret;
        int timestep = create_otp_data->timestep;
        bool is_totp = create_otp_data->is_totp;

        otp *my_otp = kmalloc(sizeof(otp), GFP_KERNEL);
        if (!my_otp) {
                pr_err("Failed to allocate memory for OTP structure\n");
                return NULL;
        }

        my_otp->cdev.owner = THIS_MODULE;
        my_otp->cdev.ops = &otp_fops;
        my_otp->cdev.dev = MKDEV(major, 0); // Replace with actual device number

        my_otp->is_totp = is_totp;
        strncpy(my_otp->algo, algo, sizeof(my_otp->algo) - 1);
        strncpy(my_otp->secret, secret, sizeof(my_otp->secret) - 1);
        my_otp->counter = 0;
        my_otp->timestep = timestep;
        my_otp->is_locked = false;

        return my_otp;
}

otp *update_otp(struct otp_ioctl_update_s *update_otp_data)
{
        if (update_otp_data == NULL) {
                pr_err("Invalid update data\n");
                return NULL;
        }

        char *algo = update_otp_data->algo;
        char *secret = update_otp_data->secret;
        int timestep = update_otp_data->timestep;


        if (update_otp_data->algo) {
                strncpy(my_otp->algo, update_otp_data->algo, sizeof(my_otp->algo) - 1);
        }
        if (update_otp_data->secret[0] != '\0') {
                strncpy(my_otp->secret, update_otp_data->secret, sizeof(my_otp->secret) - 1);
        }
        if (update_otp_data->timestep > 0) {
                my_otp->timestep = update_otp_data->timestep;
        }
        return my_otp;
}

int otp_open(struct inode *inode, struct file *file)
{
        otp *dev = container_of(inode->i_cdev, otp, cdev);

        if (!dev) {
                pr_err("Failed to get device structure\n");
                return -ENODEV;
        }

        file->private_data = dev;
        return 0;
}

int otp_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
        otp *my_otp = file->private_data;

        char code[OTP_MAX_LEN];
        int otp_code = 0;

        char counter_buff[COUNTER_MAX_SIZE] = NULL;

        int ret = 0;

        if (my_otp->is_totp) {
                // Generate TOTP code
                unsigned long current_time = jiffies_to_msecs(jiffies) / 1000;
                unsigned long time_step = current_time / my_otp->timestep;
                unsigned char counter_buf[8];
                set_counter_buffer(my_otp->counter, counter_buff);
                otp_code = hotp_algo(my_otp->algo, my_otp->secret, counter_buff);
                sprintf(code, "%d", otp_code);

        } else {
                // Generate HOTP code
                otp_code = hotp_algo(my_otp->algo, my_otp->secret, my_otp->counter);
                sprintf(code, "%d", otp_code);
        }
        my_otp->counter++;

        ret = strlen(code);

        if (!copy_to_user(buf, code, ret)) {
                pr_err("Failed to copy OTP code to user space\n");
                return -EFAULT;
        }
        return ret;
}

otp *_manage_create_command(char *algo, char *secret, char *timestep, char *is_totp)
{
        struct otp_ioctl_create_s create_otp_data;
        otp *my_otp = NULL;

        if (algo == NULL || strcmp(algo, HMAC_SHA1) != 0 || strcmp(algo, HMAC_SHA256) != 0) {
                pr_err("Invalid algorithm specified\n");
                return NULL;
        }
        strncpy(create_otp_data.algo, arg_1, sizeof(create_otp_data.algo) - 1);

        if (secret == NULL || strlen(secret) > SECRET_MAX_LEN) {
                pr_err("Invalid secret specified\n");
                return NULL;
        }
        strncpy(create_otp_data.secret, arg_2, sizeof(create_otp_data.secret) - 1);

        if (timestep == NULL || atoi(timestep) <= 0) {
                pr_err("Invalid timestep specified\n");
                return NULL;
        }
        create_otp_data.timestep = atoi(arg_3);

        if (is_totp == NULL || (strcmp(is_totp, "true") != 0)) {
                create_otp_data.is_totp = false;
        }
        create_otp_data.is_totp = (strcmp(arg_4, "true") == 0);

        return create_otp(&create_otp_data, false);;
}

otp *_manage_update_command(char *algo, char *secret, char *timestep)
{
        struct otp_ioctl_update_s update_otp_data;
        otp *my_otp = NULL;

        if (algo != NULL) {
                strncpy(update_otp_data.algo, algo, sizeof(update_otp_data.algo) - 1);
        }
        if (secret != NULL) {
                strncpy(update_otp_data.secret, secret, sizeof(update_otp_data.secret) - 1);
        }
        if (timestep != NULL) {
                update_otp_data.timestep = atoi(timestep);
        }

        return update_otp(&update_otp_data);
}

void _delete_otp_device(otp *my_otp)
{
        if (!my_otp) {
                pr_err("Invalid OTP device to delete\n");
                return;
        }

        device_destroy(otp_class, MKDEV(major, my_otp->index));
        cdev_del(&my_otp->cdev);
        kfree(my_otp);

        pr_info("OTP device deleted successfully\n");
}

int otp_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
        otp *my_otp = file->private_data;
        char input[OTP_INPUT_MAX_LEN];

        if (len > OTP_MAX_LEN) {
                pr_err("Input length exceeds maximum allowed length\n");
                return -EINVAL;
        }

        if (copy_from_user(input, buf, len)) {
                pr_err("Failed to copy data from user space\n");
                return -EFAULT;
        }

        char input_cpy[OTP_INPUT_MAX_LEN];
        strcpy(input_cpy, input);

        // CREATE or UPDATE or DELETE

        char *command = strtok(input, ",");
        char *arg_1 = strtok(NULL, ",");
        char *arg_2 = strtok(NULL, ",");
        char *arg_3 = strtok(NULL, ",");
        char *arg_4 = strtok(NULL, ",");

        if (strcmp(command, "UPDATE") == 0 && arg_1 && arg_2 && arg_3) {
                // UPDATE,<algorithm>,<secret>,<timestep>
                strncpy(my_otp->algo, arg_1, sizeof(my_otp->algo) - 1);
                strncpy(my_otp->secret, arg_2, sizeof(my_otp->secret) - 1);
                my_otp->timestep = atoi(arg_3);
        } else if (strcmp(command, "VALIDATE") == 0 && arg_1) {
                // VALIDATE,<code>

        }

        if (strcmp(command, "DELETE") == 0) {
                // DELETE,confirme
                if (strcmp(arg_1, "confirme") == 0) {
                        // Reset the device or delete it from the list
                        device_destroy(otp_class, MKDEV(major, my_otp->index));
                } else {
                        pr_err("Invalid confirmation for deletion\n");
                        return -EINVAL;
                }
        }

        pr_err("Invalid command: %s\n", input);
        return -EINVAL;

        return len;
}

int otp_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        otp *otp = file->private_data;

        switch (cmd) {
                case OTP_IOCTL_CREATE:
                        if (copy_from_user(&otp_ioctl_create, (void __user *)arg, sizeof(otp_ioctl_create))) {
                                pr_err("Failed to copy data from user space\n");
                                return -EFAULT;
                        }
                        otp = _manage_create_command(otp_ioctl_create.algo, otp_ioctl_create.secret, otp_ioctl_create.timestep, otp_ioctl_create.is_totp);
                        break;
                case OTP_IOCTL_UPDATE:
                        if (copy_from_user(&otp_ioctl_update, (void __user *)arg, sizeof(otp_ioctl_update))) {
                                pr_err("Failed to copy data from user space\n");
                                return -EFAULT;
                        }
                        otp = _manage_update_command(otp_ioctl_update.algo, otp_ioctl_update.secret, otp_ioctl_update.timestep);
                        break;
                case OTP_IOCTL_DELETE:
                        _delete_otp_device(otp);
                        break;
                default:
                        pr_err("Invalid command\n");
                        return -EINVAL;
        }

        return 0;
}
