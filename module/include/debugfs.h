#ifndef DEBUGFS_H
        #define DEBUGFS_H

        #include "lists.h"

        #include <linux/debugfs.h>

        #define DIR_NAME "otp_manager" // Name of debugfs' directory
        #define FILE_NAME "otp_list" // Name of debugfs' file
        #define PERMISSIONS 0666 // rw-rw-rw-

        extern struct dentry *d_opt_manager;
        extern struct dentry *f_opt_list;

        extern const struct file_operations debugfs_fops; // File operators for debugfs

        //typedef struct otp_code_node_s code_node;

        /**
         * @brief Initialize debugfs' directory and file
         *
         * @return int Return 0 on success, -1 otherwise
         */
        int init_debugfs(void);

        /**
         * @brief Destroy debugfs' directory and file
         */
        void destroy_debugfs(void);

        /**
         * @brief Reader for debugfs, shows otp list
         *
         * @param file File structure
         * @param buf Buffer to write from
         * @param len Length of the buffer
         * @param off Offset in the file
         *
         * @return ssize_t Number of bytes written, otherwise the error code
         */
        ssize_t debugfs_reader(struct file *f, char *buf, size_t len, loff_t *offset);

        /**
         * @brief Writer for debugfs, manages commands given using input
         * 
         * @param file File structure
         * @param buf Buffer to write from
         * @param len Length of the buffer
         * @param off Offset in the file
         *
         * @return ssize_t Number of bytes read, otherwise the error code
         */
        ssize_t debugfs_writer(struct file *f, const char *buf, size_t len, loff_t *offset);

#endif /* !DEBUGFS_H */