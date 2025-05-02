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

        #define DEFAULT_LIST_SIZE 10

        extern struct list_head otp_code_list;
        extern struct mutex otp_code_lock;

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

        /**
         * @brief Add a device to the OTP device lists.
         *
         * @param otp Device to add.
         *
         * @return 0 on success, or a negative error code on failure.
         */
        int add_otp_device(otp *dev);

        /**
         * @brief Retrive a device from the OTP device lists.
         *
         * @return otp The device found.
         */
        otp *find_otp_device_by_index(unsigned int index);

        /**
         * @brief Delete a device from the OTP device lists.
         *
         * @param index Index of the device to delete.
         *
         * @return 0 on success, or a negative error code on failure.
         */
        int delete_otp_device_by_index(unsigned int index);

        /**
         * @brief Initialize list of OTP codes.
         *
         * @return 0 on success, or -1 on failure.
         */
        int init_otp_list(void);

        /**
         * @brief Print list of OTP codes.
         */
        void print_list(void);

        /**
         * @brief Adds an OTP code to the list of OTP codes.
         *
         * @param code OTP code to add
         * @param device_index Index of the device
         *
         * @return 0 on success, or a negative error code on failure.
         */
        int add_otp_code(unsigned int code, unsigned int device_index);
        
        /**
         * @brief Finds an OTP code from the list of OTP codes.
         *
         * @param code OTP code to find
         * @param device_index Index of the device
         *
         * @return 0 on success, or a negative error code on failure.
         */
        code_node *find_otp_code(unsigned int code, unsigned int device_index);
        
        /**
         * @brief Deletes an OTP code to the list of OTP codes.
         *
         * @param code OTP code to delete
         * @param device_index Index of the device
         *
         * @return 0 on success, or a negative error code on failure.
         */
        int delete_otp_code(unsigned int code, unsigned int device_index);
        
        /**
         * @brief Deletes an OTP code to the list of OTP codes using its index.
         *
         * @param index Index of the OTP code to delete
         * @param device_index Index of the device
         *
         * @return 0 on success, or a negative error code on failure.
         */
        int delete_otp_code_by_index(unsigned int index, unsigned int device_index);
        
        /**
         * @brief Delete list of OTP codes.
         */
        void delete_all_otp_codes(void);

#endif /* LISTS_H */

