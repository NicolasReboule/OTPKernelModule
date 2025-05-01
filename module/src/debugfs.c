#include "../include/debugfs.h"

struct dentry *d_opt_manager;
struct dentry *f_opt_list;

static char data[PAGE_SIZE];
static int data_size = 0;

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