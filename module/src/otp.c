#include "../include/otp.h"

extern int major;
extern struct class *otp_class;

otp *create_otp(struct otp_ioctl_create_s *create_otp_data, bool is_validate)
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
        my_otp->is_validate = false;

        return my_otp;
}

otp *update_otp(otp *otp, struct otp_ioctl_update_s *update_otp_data)
{
        if (otp == NULL) {
                pr_err("Invalid OTP structure\n");
                return NULL;
        }

        if (update_otp_data == NULL) {
                pr_err("Invalid update data\n");
                return NULL;
        }

        char *algo = update_otp_data->algo;
        char *secret = update_otp_data->secret;
        int timestep = update_otp_data->timestep;


        if (algo[0] != '\0') {
                strncpy(otp->algo, algo, sizeof(otp->algo) - 1);
        }
        if (secret[0] != '\0') {
                strncpy(otp->secret, secret, sizeof(otp->secret) - 1);
        }
        if (timestep > 0) {
                otp->timestep = timestep;
        }
        return otp;
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

long otp_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
        otp *my_otp = file->private_data;

        char code[OTP_INPUT_MAX_LEN];
        int otp_code = 0;

        char counter_buff[COUNTER_MAX_SIZE];

        long ret = 0;

        if (my_otp->is_totp) {
                // Generate TOTP code
                unsigned long current_time = jiffies_to_msecs(jiffies) / 1000;
                unsigned long time_step = current_time / my_otp->timestep;
                set_counter_buffer(time_step, counter_buff);
                otp_code = hotp_algo(my_otp->algo, my_otp->secret, counter_buff);
                sprintf(code, "%d", otp_code);
        } else {
                // Generate HOTP code
                set_counter_buffer(my_otp->counter, counter_buff);
                otp_code = hotp_algo(my_otp->algo, my_otp->secret, counter_buff);
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

// static otp *_manage_create_command(char *algo, char *secret, char *timestep, char *is_totp)
// {
//         struct otp_ioctl_create_s create_otp_data;
//         int ret;

//         if (algo == NULL || strcmp(algo, HMAC_SHA1) != 0 || strcmp(algo, HMAC_SHA256) != 0) {
//                 pr_err("Invalid algorithm specified\n");
//                 return NULL;
//         }
//         strncpy(create_otp_data.algo, algo, sizeof(create_otp_data.algo) - 1);

//         if (secret == NULL || strlen(secret) > SECRET_MAX_LEN) {
//                 pr_err("Invalid secret specified\n");
//                 return NULL;
//         }
//         strncpy(create_otp_data.secret, secret, sizeof(create_otp_data.secret) - 1);

//         int timestep_value = 0;
//         ret = kstrtoint(timestep, 10, &timestep_value);
//         if (ret != 0) {
//                 pr_err("Failed to convert timestep to integer\n");
//                 return NULL;
//         }
//         if (timestep == NULL || timestep_value <= 0) {
//                 pr_err("Invalid timestep specified\n");
//                 return NULL;
//         }
//         create_otp_data.timestep = timestep_value;

//         if (is_totp == NULL || (strcmp(is_totp, "true") != 0)) {
//                 create_otp_data.is_totp = false;
//         }
//         create_otp_data.is_totp = (strcmp(is_totp, "true") == 0);

//         return create_otp(&create_otp_data, false);;
// }

static otp *_manage_update_command(otp *otp, char *algo, char *secret, char *timestep)
{
        struct otp_ioctl_update_s update_otp_data;
        int ret;

        if (otp == NULL) {
                pr_err("Invalid OTP structure\n");
                return NULL;
        }
        if (algo != NULL) {
                strncpy(update_otp_data.algo, algo, sizeof(update_otp_data.algo) - 1);
        }
        if (secret != NULL) {
                strncpy(update_otp_data.secret, secret, sizeof(update_otp_data.secret) - 1);
        }
        if (timestep != NULL) {
                int timestep_value = 0;
                ret = kstrtoint(timestep, 10, &timestep_value);
                if (ret != 0 || timestep_value <= 0) {
                        pr_err("Invalid timestep specified\n");
                        return NULL;
                }
                update_otp_data.timestep = timestep_value;
        }

        return update_otp(otp, &update_otp_data);
}

static void _delete_otp_device(otp *my_otp)
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

long otp_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
        // otp *my_otp = file->private_data;
        char input[OTP_INPUT_MAX_LEN];

        if (len > OTP_INPUT_MAX_LEN) {
                pr_err("Input length exceeds maximum allowed length\n");
                return -EINVAL;
        }

        if (copy_from_user(input, buf, len)) {
                pr_err("Failed to copy data from user space\n");
                return -EFAULT;
        }

        char input_cpy[OTP_INPUT_MAX_LEN];
        char *input_ptr = input_cpy;
        strcpy(input_cpy, input);

        // CREATE or UPDATE or DELETE

        char *token;
        while ((token = strsep(&input_ptr, ",")) != NULL) {
                // Process token
        }

        // char *command = strtok(input, ",");
        // char *arg_1 = strtok(NULL, ",");
        // char *arg_2 = strtok(NULL, ",");
        // char *arg_3 = strtok(NULL, ",");
        // char *arg_4 = strtok(NULL, ",");

        // if (strcmp(command, "UPDATE") == 0 && arg_1 && arg_2 && arg_3) {
        //         // UPDATE,<algorithm>,<secret>,<timestep>
        //         strncpy(my_otp->algo, arg_1, sizeof(my_otp->algo) - 1);
        //         strncpy(my_otp->secret, arg_2, sizeof(my_otp->secret) - 1);
        //         my_otp->timestep = atoi(arg_3);
        // } else if (strcmp(command, "VALIDATE") == 0 && arg_1) {
        //         // VALIDATE,<code>

        // }

        // if (strcmp(command, "DELETE") == 0) {
        //         // DELETE,confirme
        //         if (strcmp(arg_1, "confirme") == 0) {
        //                 // Reset the device or delete it from the list
        //                 device_destroy(otp_class, MKDEV(major, my_otp->index));
        //         } else {
        //                 pr_err("Invalid confirmation for deletion\n");
        //                 return -EINVAL;
        //         }
        // }

        pr_err("Invalid command: %s\n", input);
        return -EINVAL;

        return len;
}

long otp_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        otp *otp = file->private_data;

        switch (cmd) {
                case IOCTL_CREATE_OTP:
                        struct otp_ioctl_create_s otp_ioctl_create;
                        if (copy_from_user(&otp_ioctl_create, (void __user *)arg, sizeof(otp_ioctl_create))) {
                                pr_err("Failed to copy data from user space\n");
                                return -EFAULT;
                        }
                        // otp = _manage_create_command(otp_ioctl_create.algo, otp_ioctl_create.secret, otp_ioctl_create.timestep, otp_ioctl_create.is_totp);
                        break;
                case IOCTL_UPDATE_OTP:
                        struct otp_ioctl_update_s otp_ioctl_update;
                        if (copy_from_user(&otp_ioctl_update, (void __user *)arg, sizeof(otp_ioctl_update))) {
                                pr_err("Failed to copy data from user space\n");
                                return -EFAULT;
                        }
                        char timestep_str[16];
                        snprintf(timestep_str, sizeof(timestep_str), "%d", otp_ioctl_update.timestep);
                        otp = _manage_update_command(otp, otp_ioctl_update.algo, otp_ioctl_update.secret, timestep_str);
                        break;
                case IOCTL_DELETE_OTP:
                        _delete_otp_device(otp);
                        break;
                case IOCTL_VALIDATE_OTP:
                        break;
                default:
                        pr_err("Invalid command\n");
                        return -EINVAL;
        }

        return 0;
}

const struct file_operations otp_fops = {
        .owner = THIS_MODULE,
        .open = otp_open,
        .read = otp_read,
        .write = otp_write,
        .unlocked_ioctl = otp_ioctl
};