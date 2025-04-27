/*
** EPITECH PROJECT, 2025
** OTP
** File description:
** otp.h
** Header file for OTP module
*/
#ifndef LISTS_H
        #define LISTS_H

        #include "otp.h"

        #include <linux/list.h>
        #include <linux/slab.h>
        #include <linux/mutex.h>

        struct otp_device_node_s {
                otp *otp;
                struct list_head list;
        }

        struct otp_code_node_s {
                char *code;
                struct list_head list;
        }

        typedef struct otp_device_node_s otp_list;

        int save_otp_code(otp *dev, char *buf, size_t len);
        int validate_otp_code(otp *dev, char *buf, size_t len);

        int add_otp_device(otp *dev);
        otp *find_otp_device_by_index(unsigned int index);
        int delete_otp_device_by_index(unsigned int index);

#endif /* LISTS_H */

