/*
** EPITECH PROJECT, 2025
** OTP
** File description:
** device.h
** Header file listing all the functions usable in crypto.c
*/
#ifndef DEVICE_H
        #define DEVICE_H

        #include "crypto.h"

        #include <linux/module.h>
        #include <linux/device.h>
        #include <linux/mutex.h>
        #include <linux/list.h>
        #include <linux/fs.h>
        #include <linux/cdev.h>
        #include <linux/uaccess.h>
        #include <linux/slab.h>
        #include <linux/err.h>

        #define DEVICE_NAME "otp"
        #define CLASS_NAME "otp_class"
        #define MAX_DEVICES 20

        #define OTP_IOC_MAGIC  'k'
        #define OTP_IOC_SET_SECRET   _IOW(OTP_IOC_MAGIC, 1, secret_t)
        #define OTP_IOC_SET_CODE   _IOW(OTP_IOC_MAGIC, 2, unsigned int)
        #define OTP_IOC_RESET_COUNTER _IO(OTP_IOC_MAGIC, 3)
        #define OTP_IOC_SET_TIME_STEP _IOW(OTP_IOC_MAGIC, 4, unsigned int)
        #define OTP_IOC_CREATE_DEVICE _IOW(OTP_IOC_MAGIC, 5, type_t)
        #define OTP_IOC_DELETE_DEVICE _IOW(OTP_IOC_MAGIC, 6, int)

        #define SECRET_MAX_LEN 64

        struct otp_type_s {
                bool is_totp;
                bool is_verify;
        };

        struct otp_secret_s {
                char key[SECRET_MAX_LEN];
        };

        struct otp_device_s {
                struct cdev cdev;

                char *algo;

                char secret[SECRET_MAX_LEN];
                size_t secret_len;
                int counter;
                int timestep;

                int code;

                bool is_totp;
                bool is_verify;
        };

         /**
         * @brief OTP type structure
         * @param is_totp Flag indicating if the device is TOTP or HOTP
         * @param is_verify Flag indicating if the device is in verify mode
         */
        typedef struct otp_type_s type_t;

        /**
         * @brief OTP secret structure
         * @param key OTP secret key
         */
        typedef struct otp_secret_s secret_t;

        /**
         * @brief OTP device structure
         * @param cdev Character device structure
         * @param algo Algorithm used for OTP generation
         * @param secret OTP secret key
         * @param secret_len Length of the secret key
         * @param counter Counter for HOTP
         * @param timestep Time step for TOTP
         * @param code Generated OTP code
         * @param is_totp Flag indicating if the device is TOTP or HOTP
         * @param is_verify Flag indicating if the device is in verify mode
         */
        typedef struct otp_device_s otp_t;

        struct otp_device_node_s {
                otp_t dev;
                struct list_head list;
                int index;
        };

        /**
         * @brief OTP device node structure
         * @param dev Device structure
         * @param list List head for linked list of devices
         * @param index Index of the device in the list
         */
        typedef struct otp_device_node_s opt_node_t;

        /**
         * @brief Manage open behavior of the device
         *
         * @param counter Counter to set.
         * @param counter_buf Buffer to set.
         * @return int Return Code (0 success, otherwise error)
         */
        int otp_open(struct inode *inode, struct file *file);
        /**
         * @brief Manage read behavior of the device
         *
         * @param file File structure
         * @param buf Buffer to read into
         * @param len Length of the buffer
         * @param off Offset in the file
         * @return ssize_t Number of bytes read
         */
        ssize_t otp_read(struct file *file, char __user *buf, size_t len, loff_t *off);
        /**
         * @brief Manage ioctl of the device
         *
         * @param file File structure
         * @param cmd Command to execute
         * @param arg Argument for the command
         * @return long Return code (0 success, otherwise error)
         */
        long otp_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
        /**
         * @brief Create a device in the system
         *
         * @param dev Device structure
         * @param is_totp Flag indicating if the device is TOTP or HOTP
         * @return int Return code (0 success, otherwise error)
         */
        int create_device(int index, type_t *type);
        /**
         * @brief Find a device by its index
         *
         * @param index Index of the device to find
         * @return opt_node_t* Pointer to the found device node, or NULL if not found
         */
        opt_node_t  *find_device_by_index(int index);
        /**
         * @brief Delete a device from the system
         *
         * @param index Index of the device to delete
         * @return int Return code (0 success, otherwise error)
         */
        int delete_device(int index);
        /**
         * @brief Generate the OTP code
         *
         * @param dev OTP device structure
         * @param counter Counter to use for HOTP or TOTP
         * @param out_code Output buffer for the generated code
         * @return int Return Code (0 success, otherwise error)
         */
        int otp_generate_code(otp_t *dev, unsigned int counter, char *out_code);


#endif /* !DEVICE_H_ */