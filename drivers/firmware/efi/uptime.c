#include "linux/types.h"
#include <linux/efi.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/printk.h>

static ssize_t uptime_show(struct kobject *kobj,
				     struct kobj_attribute *attr, char *buf)
{
	efi_status_t status;
	unsigned long ticks;

	status = efi.get_uptime(&ticks);
	if (status != EFI_SUCCESS) {
		return efi_status_to_err(status);
	}

	return sprintf(buf, "%lu\n", ticks);
}

static struct kobj_attribute uptime_attribute = __ATTR_RO(uptime);

static ssize_t time_show(struct kobject *kobj,
				     struct kobj_attribute *attr, char *buf)
{
	efi_status_t status;
	efi_time_t time;

	status = efi.get_time(&time, NULL);
	if (status != EFI_SUCCESS) {
		return efi_status_to_err(status);
	}

	return sprintf(buf, "%04u-%02u-%02u %02u:%02u:%02u\n",
		       time.year, time.month, time.day,
		       time.hour, time.minute, time.second);
}

static struct kobj_attribute time_attribute = __ATTR_RO(time);

static void __exit uptime_sysfs_exit(void)
{
	printk(KERN_INFO "EFI uptime sysfs exit\n");
}

static int __init uptime_sysfs_init(void)
{
	int error;

	printk(KERN_INFO "EFI uptime sysfs init\n");

	error = sysfs_create_file(efi_kobj, &uptime_attribute.attr);
	if (error) {
		return error;
	}

	error = sysfs_create_file(efi_kobj, &time_attribute.attr);
	if (error) {
		sysfs_remove_file(efi_kobj, &uptime_attribute.attr);
		return error;
	}

	return 0;
}

MODULE_LICENSE("GPL");
module_init(uptime_sysfs_init);
module_exit(uptime_sysfs_exit);