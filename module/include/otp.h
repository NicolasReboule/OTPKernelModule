/*
** EPITECH PROJECT, 2025
** OTP
** File description:
** otp.h
** Header file for OTP module
*/
#ifndef OTP_H
        #define OTP_H

        #include "crypto.h"

        #include <linux/string.h>
        #include <linux/cdev.h>
        #include <linux/types.h>

        #define OTP_IOC_MAGIC 'K'

        #define OTP_INPUT_MAX_LEN 256
        #define COUNTER_MAX_SIZE 9

        #define IOCTL_CREATE_OTP _IOW(OTP_IOC_MAGIC, 1, struct otp_ioctl_create_s)
        #define IOCTL_UPDATE_OTP _IOW(OTP_IOC_MAGIC, 2, struct otp_ioctl_update_s)
        #define IOCTL_DELETE_OTP _IOW(OTP_IOC_MAGIC, 3, unsigned int)
        #define IOCTL_VALIDATE_OTP _IOW(OTP_IOC_MAGIC, 4, unsigned int)

        /**
         * @brief OTP IOCTL structure for create command
         * All fields are required.
         *
         * @param is_totp Flag indicating if the OTP is TOTP or HOTP
         * @param algo Algorithm to use (e.g. HMAC_SHA1 or HMAC_SHA256).
         * @param secret OTP secret key
         * @param timestep Time step for TOTP (only for TOTP)
         */
        struct otp_ioctl_create_s {
                bool is_totp;
                char algo[16];
                char secret[64];
                int timestep;
        };

        /**
         * @brief OTP IOCTL structure for update command
         * If any of the fields are not set, they will not be updated.
         *
         * @param algo Algorithm to use (e.g. HMAC_SHA1 or HMAC_SHA256).
         * @param secret OTP secret key
         * @param timestep Time step for TOTP (only for TOTP)
         */
        struct otp_ioctl_update_s {
                char algo[16];
                char secret[64];
                int timestep;
        };

        struct otp_s {
                unsigned int index;
                struct cdev cdev;

                char *algo;

                char secret[64];
                int counter;
                int timestep;

                bool is_totp;
                bool is_validate;
        };

        /**
         * @brief OTP structure
         * This structure is used to manage the OTP device.
         *
         * @param cdev Character device structure
         * @param algo Algorithm used for OTP generation
         * @param secret OTP secret key
         * @param counter Counter for HOTP
         * @param timestep Time step for TOTP
         * @param is_totp Flag indicating if the device is TOTP or HOTP
         * @param is_verify Flag indicating if the device is in verify mode
         * @param is_validate Flag indicating if the device is locked
         */
        typedef struct otp_s otp;

        /**
         * @brief Create an OTP structure
         * This function creates an OTP structure and initializes it with the given parameters.
         *
         * @param create_otp_data Pointer to the OTP creation data structure
         * @param is_validate Flag indicating if the OTP is in validate mode
         *
         * @return otp Pointer to the created OTP structure
         */
        otp *create_otp(struct otp_ioctl_create_s *create_otp_data, bool is_validate);

        /**
         * @brief Update an OTP structure
         * This function updates the OTP structure with the given parameters.
         *
         * @param update_otp_data Pointer to the OTP update data structure
         * @param is_validate Flag indicating if the OTP is in validate mode
         *
         * @return otp Pointer to the updated OTP structure
         */
        otp *update_otp(otp *otp, struct otp_ioctl_update_s *update_otp_data);


        /**
         * @brief Manage open behavior of the device
         *
         * @param counter Counter to set.
         * @param counter_buf Buffer to set.
         *
         * @return int Return code (0 success, otherwise error)
         */
        int otp_open(struct inode *inode, struct file *file);

        /**
         * @brief Manage read behavior of the device
         *
         * @param file File structure
         * @param buf Buffer to read into
         * @param len Length of the buffer
         * @param off Offset in the file
         *
         * @return ssize_t Number of bytes read
         */
        long otp_read(struct file *file, char __user *buf, size_t len, loff_t *off);

        /**
         * @brief Manage write behavior of the device
         *
         * @param file File structure
         * @param buf Buffer to write from
         * @param len Length of the buffer
         * @param off Offset in the file
         *
         * @return int Return code (0 success, otherwise error)
         */
        long otp_write(struct file *file, const char __user *buf, size_t len, loff_t *off);

        /**
         * @brief Manage ioctl of the device
         *
         * @param file File structure
         * @param cmd Command to execute
         * @param arg Argument for the command
         *
         * @return long Return code (0 success, otherwise error)
         */
        long otp_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

        /**
         * @brief Generate OTP code using the specified algorithm and key
         *
         * @param algo Algorithm to use (e.g. HMAC_SHA1 or HMAC_SHA256).
         * @param key Key to use.
         * @param counter Counter to use for HOTP.
         * @param out_code Buffer to store the generated OTP code.
         *
         * @return int Return code (0 success, otherwise error)
         */
        int otp_generate_code(const char *algo, const char *key, unsigned int counter, char *out_code);

        /**
         * @brief OTP structure to manage IOCTL behavior
         * .owner = THIS_MODULE,
         * .open = otp_open,
         * .read = otp_read,
         * .unlocked_ioctl ,
         */
        extern const struct file_operations otp_fops;


#endif /* !OTP_H_ */