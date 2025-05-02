/*
** EPITECH PROJECT, 2025
** OTP
** File description:
** device.h
** Header file for Device management
*/
#ifndef DEVICE_H
        #define DEVICE_H

        #include <linux/cdev.h>
        #include <linux/device.h>
        #include <linux/fs.h>
        #include <linux/slab.h>
        #include <linux/module.h>

        #include "otp.h"
        #include "debugfs.h"

        #define MAX_OTP_DEVICES 64
        #define DEVICE_OTP_NAME "otp"
        #define CLASS_OTP_NAME "otp_class"
        #define DEVICE_COUNT 3

        #define HOTP_HELP_MESSAGE "Usage:\nshow\t\t\tShow the current parameters\nupdate -a <algo> -s <secret>\tUpdate the parameters of the HOTP\n\t\t\talgo: SHA1 or SHA256\n\t\t\tsecret: secret key (max 64 characters)\n"
        #define TOTP_HELP_MESSAGE "Usage:\nshow\t\t\tShow the current parameters\nupdate -a <algo> -s <secret> -t\tUpdate the parameters of the TOTP\n\t\t\talgo: SHA1 or SHA256\n\t\t\tsecret: secret key (max 64 characters)\n\t\t\ttimestep: timestep in seconds (default 30)\n"

        extern const char *device_names[DEVICE_COUNT]; // Array of device names
        extern struct cdev cdevs[DEVICE_COUNT]; // Array of cdev (represents the device)
        extern struct device *devices[DEVICE_COUNT]; // Array of devices
        extern dev_t dev_num; // Where the device_number will be stored
        extern int major; // Major number for the devices
        extern struct class *otp_class; // Device class for OTP devices

        /**
         * @brief Initialize all devices
         *
         * @return int Return 0 on success, -1 otherwise
         */
        int create_devices(void);

        /**
         * @brief Set devices permissions
         *
         * @param dev Device to attribute permissions
         * @param mode Permissions to attribute
         *         
         * @return char * Always NULL
         */
        char *device_devnode(const struct device *dev, umode_t *mode);

        /**
         * @brief Destroy all devices
         */
        void destroy_devices(void);

        /**
         * @brief Create an OTP device.
         *
         * @param index The index of the device (used for minor number and device name).
         * @param otp_data Pointer to the OTP creation data structure.
         * @return Pointer to the created OTP device, or NULL on failure.
         */
        otp *create_otp_device(unsigned int index, struct otp_ioctl_create_s *otp_data);

        /**
         * @brief Delete an OTP device.
         *
         * @param dev Pointer to the OTP device to delete.
         */
        void delete_otp_device(otp *dev);

        /**
         * @brief Reader for hotp device, returns a hotp
         *
         * @param file File structure
         * @param buf Buffer to write from
         * @param len Length of the buffer
         * @param off Offset in the file
         *
         * @return ssize_t Number of bytes written, otherwise the error code
         */
        ssize_t hotp_reader(struct file *f, char *buf, size_t len, loff_t *offset);
        
        /**
         * @brief Writer for hotp device, manages commands given using input
         * 
         * @param file File structure
         * @param buf Buffer to write from
         * @param len Length of the buffer
         * @param off Offset in the file
         *
         * @return ssize_t Number of bytes read, otherwise the error code
         */
        ssize_t hotp_writer(struct file *f, const char *buf, size_t len, loff_t *offset);
        
        /**
         * @brief Reader for totp device, returns a totp
         *
         * @param file File structure
         * @param buf Buffer to write from
         * @param len Length of the buffer
         * @param off Offset in the file
         *
         * @return ssize_t Number of bytes written, otherwise the error code
         */
        ssize_t totp_reader(struct file *f, char *buf, size_t len, loff_t *offset);
        
        /**
         * @brief Writer for totp device, manages commands given using input
         * 
         * @param file File structure
         * @param buf Buffer to write from
         * @param len Length of the buffer
         * @param off Offset in the file
         *
         * @return ssize_t Number of bytes read, otherwise the error code
         */
        ssize_t totp_writer(struct file *f, const char *buf, size_t len, loff_t *offset);
        
        /**
         * @brief Reader for otp validator, returns 1 if otp is valid, 0 otherwise
         *
         * @param file File structure
         * @param buf Buffer to write from
         * @param len Length of the buffer
         * @param off Offset in the file
         *
         * @return ssize_t Number of bytes written, otherwise the error code
         */
        ssize_t validator_reader(struct file *f, char *buf, size_t len, loff_t *offset);
        
        /**
         * @brief Writer for otp validator, checks if otp is valid
         * 
         * @param file File structure
         * @param buf Buffer to write from
         * @param len Length of the buffer
         * @param off Offset in the file
         *
         * @return ssize_t Number of bytes read, otherwise the error code
         */
        ssize_t validator_writer(struct file *f, const char *buf, size_t len, loff_t *offset);

#endif /* DEVICE_H */