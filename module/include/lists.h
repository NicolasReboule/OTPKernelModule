/*
** EPITECH PROJECT, 2025
** OTP
** File description:
** lists.h
** Header file for Lists
*/
#ifndef LISTS_H
        #define LISTS_H

        #include "device.h"

        #include <linux/list.h>
        #include <linux/slab.h>
        #include <linux/mutex.h>

        struct otp_device_node_s {
                otp *otp;
                struct list_head list;
        };

        struct otp_code_node_s {
                unsigned int code;
                unsigned int device_index;
                struct list_head list;
        };

        typedef struct otp_device_node_s otp_node;

        typedef struct otp_code_node_s code_node;

        /**
         * @brief Initialize the OTP device and code lists.
         *
         * @return 0 on success, or a negative error code on failure.
         */
        int initialize_otp_node_system(void);

        int add_otp_device(otp *dev);
        otp *find_otp_device_by_index(unsigned int index);
        int delete_otp_device_by_index(unsigned int index);

        int add_otp_code(unsigned int code, unsigned int device_index);
        code_node *find_otp_code(unsigned int code, unsigned int device_index);
        int delete_otp_code(unsigned int code, unsigned int device_index);
        void delete_all_otp_codes(void);

#endif /* LISTS_H */

