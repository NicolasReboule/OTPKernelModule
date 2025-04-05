/*
** EPITECH PROJECT, 2025
** OTP
** File description:
** otp.h
** Header file for OTP module
*/
#ifndef OTP_H
        #define OTP_H

        #include <linux/module.h>
        #include <linux/printk.h>

        #include "device.h"

        /**
         * @brief Initialize the module with required components.
         * Components:
         * - Sysfs (interface)
         * - Procfs (user friendly)
         * - Devices (interaction)
         *
         * @return int Return Code (0 success, -1 error)
         */
        static int __init otp_init_module(void);

        /**
         * @brief Unload the module and used components.
         *
         */
        static void __exit otp_exit_module(void);

#endif /* !OTP_H_*/