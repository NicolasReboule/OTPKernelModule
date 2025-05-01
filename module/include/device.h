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

        extern const char *device_names[DEVICE_COUNT];
        extern struct cdev cdevs[DEVICE_COUNT];
        extern struct device *devices[DEVICE_COUNT];
        extern dev_t dev_num;
        extern int major; // Major number for the devices
        extern struct class *otp_class; // Device class for OTP devices

        int create_devices(void);

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

        ssize_t hotp_reader(struct file *f, char *buf, size_t len, loff_t *offset);
        ssize_t hotp_writer(struct file *f, const char *buf, size_t len, loff_t *offset);
        ssize_t totp_reader(struct file *f, char *buf, size_t len, loff_t *offset);
        ssize_t totp_writer(struct file *f, const char *buf, size_t len, loff_t *offset);
        ssize_t validator_reader(struct file *f, char *buf, size_t len, loff_t *offset);
        ssize_t validator_writer(struct file *f, const char *buf, size_t len, loff_t *offset);

#endif /* DEVICE_H */