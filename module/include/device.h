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

        #define MAX_OTP_DEVICES 64
        #define DEVICE_OTP_NAME "otp"
        #define CLASS_OTP_NAME "otp_class"

        extern int major; // Major number for the devices
        extern struct class *otp_class; // Device class for OTP devices
        extern const struct file_operations otp_fops; // File operations for OTP devices

        /**
         * @brief Create an OTP device.
         *
         * @param index The index of the device (used for minor number and device name).
         * @param algo The algorithm to use (e.g., "HMAC_SHA1").
         * @param secret The secret key for the OTP device.
         * @param timestep The timestep for TOTP devices (ignored for HOTP).
         * @param is_totp Whether the device is a TOTP (true) or HOTP (false).
         * @return Pointer to the created OTP device, or NULL on failure.
         */
        otp *create_otp_device(unsigned int index, const char *algo, const char *secret, int timestep, bool is_totp);

        /**
         * @brief Delete an OTP device.
         *
         * @param dev Pointer to the OTP device to delete.
         */
        void delete_otp_device(otp *dev);

#endif /* DEVICE_H */