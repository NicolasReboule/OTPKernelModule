#include "../include/debugfs.h"

struct dentry *d_opt_manager;
struct dentry *f_opt_list;

static char data[PAGE_SIZE];
static int data_size = 0;

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
	if (*offset >= data_size)
		return 0;
	copy_to_user(buf, data, data_size);
	*offset = data_size;
	return data_size;
}

static ssize_t debugfs_writer(struct file *f, const char *buf, size_t len, loff_t *offset)
{
	copy_from_user(data, buf, len);
	data_size = len;
	return len;
}

const struct file_operations debugfs_fops = {
	.read = debugfs_reader,
	.write = debugfs_writer,
};