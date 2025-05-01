#include "../include/debugfs.h"

struct dentry *d_opt_manager;
struct dentry *f_opt_list;

int init_debugfs(void)
{
        d_opt_manager = debugfs_create_dir(DIR_NAME, NULL);
        if (!d_opt_manager || IS_ERR(d_opt_manager)) {
                pr_err("Failed to create debugfs directory\n");
                return -1;
        }
        f_opt_list = debugfs_create_file(FILE_NAME, PERMISSIONS, d_opt_manager, NULL, &debugfs_fops);
        if (!f_opt_list || IS_ERR(f_opt_list)) {
                pr_err("Failed to create debugfs file\n");
                debugfs_remove_recursive(d_opt_manager);
                return -1;   
        }
        return 0;
}

void destroy_debugfs(void)
{
        debugfs_remove_recursive(d_opt_manager);
}

static ssize_t debugfs_reader(struct file *f, char *buf, size_t len, loff_t *offset)
{
        code_node *node;
        size_t size;
        ssize_t ret;
        char *str = kmalloc(PAGE_SIZE, GFP_KERNEL);

        if (*offset > 0)
                return 0;

        if (!str) {
                pr_err("Failed to allocate buffer to read list\n");
                return -ENOMEM;
        }

        mutex_lock(&otp_code_lock);
        list_for_each_entry(node, &otp_code_list, list) {
                size += scnprintf(str, PAGE_SIZE - size, "%s%d\n", str, node->code);
        }
        mutex_unlock(&otp_code_lock);
        ret = simple_read_from_buffer(buf, len, offset, str, size);
        kfree(str);
	return ret;
}

static ssize_t debugfs_writer(struct file *f, const char *buf, size_t len, loff_t *offset)
{
        char cmd_buf[32];
        char cmd[16], args[16];
        int arg;

        if (len > 32) {
                pr_err("Invalid input: input size > %d\n", 32);
                return -EINVAL;
        }
        if (copy_from_user(cmd_buf, buf, len)) {
                pr_err("Could not copy input from user\n");
                return -EFAULT;
        }
        cmd_buf[len] = '\0';
        cmd[0] = args[0] = '\0';

        sscanf(cmd_buf, "%15s %15s", cmd, args);

        if (strcmp(cmd, "add") == 0) {
                if (kstrtoint(args, 10, &arg) < 0) {
                        pr_err("Failed to convert arg to int\n");
                        return -EINVAL;
                }
                add_otp_code(arg, 0);
        } else if (strcmp(cmd, "delete") == 0) {
                if (kstrtoint(args, 10, &arg) < 0) {
                        pr_err("Failed to convert arg to int\n");
                        return -EINVAL;
                }
                if (delete_otp_code_by_index(arg, 0) < 0)
                        return -ENOENT;
        } else {
                pr_info("Usage:\n");
                pr_info("\tadd <int> - Adds the number to list as an otp\n");
                pr_info("\tdelete <int> - Removes otp from list at corresponding index\n");
                pr_info("\tdefault - Shows this message\n");
        }
	return len;
}

const struct file_operations debugfs_fops = {
	.read = debugfs_reader,
	.write = debugfs_writer,
};