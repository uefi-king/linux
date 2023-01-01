#include <linux/efi.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/printk.h>

static ssize_t uptime_show(struct kobject *kobj,
				     struct kobj_attribute *attr, char *buf)
{
	efi_status_t status;
	unsigned long seconds;

	status = efi.get_uptime_seconds(&seconds);
	if (status != EFI_SUCCESS) {
		return efi_status_to_err(status);
	}

	return sprintf(buf, "%lu\n", seconds);
}

static struct kobj_attribute uptime_attribute = __ATTR_RO(uptime);

static void __exit uptime_sysfs_exit(void)
{
	printk(KERN_INFO "EFI uptime sysfs deinit\n");
}

static int __init uptime_sysfs_init(void)
{
	int error;

	printk(KERN_INFO "EFI uptime sysfs init\n");

	error = sysfs_create_file(efi_kobj, &uptime_attribute.attr);
	if (error) {
		return error;
	}

	return 0;
}

MODULE_LICENSE("GPL");
module_init(uptime_sysfs_init);
module_exit(uptime_sysfs_exit);
