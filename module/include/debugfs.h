#ifndef DEBUGFS_H
        #define DEBUGFS_H

        #include <linux/debugfs.h>

        #define DIR_NAME "otp_manager"
        #define FILE_NAME "otp_list"
        #define PERMISSIONS 0644

        extern struct dentry *d_opt_manager;
        extern struct dentry *f_opt_list;

        extern const struct file_operations debugfs_fops;
#endif /* !DEBUGFS_H */