#ifndef DEBUGFS_H
        #define DEBUGFS_H

        #include "lists.h"

        #include <linux/debugfs.h>

        #define DIR_NAME "otp_manager"
        #define FILE_NAME "otp_list"
        #define PERMISSIONS 0666 // rw-rw-rw-

        extern struct dentry *d_opt_manager;
        extern struct dentry *f_opt_list;

        extern const struct file_operations debugfs_fops;

        //typedef struct otp_code_node_s code_node;

        int init_debugfs(void);
        void destroy_debugfs(void);

#endif /* !DEBUGFS_H */