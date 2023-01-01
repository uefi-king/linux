#include <linux/efi.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/printk.h>

static struct bin_attribute *flash;

static ssize_t flash_read(struct file *filp, struct kobject *kobj,
			  struct bin_attribute *bin_attr,
			  char *buf, loff_t pos, size_t count)
{
	efi_status_t status;

	status = efi.read_flash(pos, (u64 *)&count, buf);
	if (status != EFI_SUCCESS) {
		return efi_status_to_err(status);
	}

	return count;
}

static ssize_t flash_write(struct file *filp, struct kobject *kobj,
			   struct bin_attribute *bin_attr,
			   char *buf, loff_t pos, size_t count)
{
	efi_status_t status;

	status = efi.write_flash(pos, (u64 *)&count, buf);
	if (status != EFI_SUCCESS) {
		return efi_status_to_err(status);
	}

	return count;
}

static void flash_sysfs_exit(void)
{
	printk(KERN_INFO "Flash sysfs deinit\n");
	sysfs_remove_bin_file(efi_kobj, flash);
}

static int flash_sysfs_init(void)
{
	int error;
	u64 flash_size;
	efi_status_t status;

	printk(KERN_INFO "Flash sysfs init\n");

	status = efi.get_flash_size(&flash_size);
	if (status != EFI_SUCCESS) {
		return efi_status_to_err(status);
	}
	printk(KERN_INFO "Flash size: %llu\n", flash_size);

	flash = kzalloc(sizeof *flash, GFP_KERNEL);
	if (flash == NULL) {
		return -ENOMEM;
	}

	flash->attr.name = "flash";
	flash->attr.mode = 0600;
	flash->size = flash_size;
	flash->read = flash_read;
	flash->write = flash_write;

	sysfs_bin_attr_init(flash);

	error = sysfs_create_bin_file(efi_kobj, flash);
	if (error) {
		kfree(flash);
		return error;
	}

	return 0;
}

MODULE_LICENSE("GPL");
module_init(flash_sysfs_init);
module_exit(flash_sysfs_exit);
