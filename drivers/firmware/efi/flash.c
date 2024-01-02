#include <linux/efi.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/printk.h>

static struct bin_attribute *flash;
static struct bin_attribute *flash_encrypted;

static ssize_t flash_read(struct file *f, struct kobject *kobj,
			  struct bin_attribute *bin_attr,
			  char *buf, loff_t pos, size_t count)
{
	efi_status_t status;

	status = efi.read_flash(0, pos, (u64 *)&count, buf);
	if (status != EFI_SUCCESS) {
		return efi_status_to_err(status);
	}

	return count;
}

static ssize_t flash_write(struct file *f, struct kobject *kobj,
			   struct bin_attribute *bin_attr,
			   char *buf, loff_t pos, size_t count)
{
	efi_status_t status;

	status = efi.write_flash(0, pos, (u64 *)&count, buf);
	if (status != EFI_SUCCESS) {
		return efi_status_to_err(status);
	}

	return count;
}

static ssize_t flash_read_encrypted(struct file *f, struct kobject *kobj,
			  struct bin_attribute *bin_attr,
			  char *buf, loff_t pos, size_t count)
{
	efi_status_t status;

	status = efi.read_flash(1, pos, (u64 *)&count, buf);
	if (status != EFI_SUCCESS) {
		return efi_status_to_err(status);
	}

	return count;
}

static ssize_t flash_write_encrypted(struct file *f, struct kobject *kobj,
			   struct bin_attribute *bin_attr,
			   char *buf, loff_t pos, size_t count)
{
	efi_status_t status;
	status = efi.write_flash(1, pos, (u64 *)&count, buf);
	if (status != EFI_SUCCESS) {
		return efi_status_to_err(status);
	}

	return count;
}

static void flash_sysfs_exit(void)
{
	sysfs_remove_bin_file(efi_kobj, flash);
}

static int flash_sysfs_init(void)
{
	int error;
	u64 flash_size;
	u64 flash_size_encrypted;
	efi_status_t status;

	printk(KERN_INFO "Flash sysfs init\n");

	status = efi.get_flash_size(0, &flash_size);
	if (status != EFI_SUCCESS) {
		printk(KERN_ERR "get flash size fail, status: %lu\n", status);
		return efi_status_to_err(status);
	}
	printk(KERN_INFO "Flash size: %llu\n", flash_size);

	status = efi.get_flash_size(1, &flash_size_encrypted);
	if (status != EFI_SUCCESS) {
		printk(KERN_ERR "get encrypted flash size fail, status: %lu\n", status);
		return efi_status_to_err(status);
	}
	printk(KERN_INFO "Encrypted flash size: %llu\n", flash_size_encrypted);

	flash = kzalloc(sizeof *flash, GFP_KERNEL);
	if (flash == NULL) {
		printk(KERN_ERR "alloc flash fail\n");
		return -ENOMEM;
	}

	flash_encrypted = kzalloc(sizeof *flash_encrypted, GFP_KERNEL);
	if (flash_encrypted == NULL) {
		printk(KERN_ERR "alloc flash_encrypted fail\n");
		return -ENOMEM;
	}

	flash->attr.name = "flash";
	flash->attr.mode = 0600;
	flash->size = flash_size;
	flash->read = flash_read;
	flash->write = flash_write;

	flash_encrypted->attr.name = "flash_encrypted";
	flash_encrypted->attr.mode = 0600;
	flash_encrypted->size = flash_size_encrypted;
	flash_encrypted->read = flash_read_encrypted;
	flash_encrypted->write = flash_write_encrypted;

	sysfs_bin_attr_init(flash);
	sysfs_bin_attr_init(flash_encrypted);

	error = sysfs_create_bin_file(efi_kobj, flash);
	if (error) {
		printk(KERN_ERR "create bin file fail\n");
		kfree(flash);
		return error;
	}

	error = sysfs_create_bin_file(efi_kobj, flash_encrypted);
	if (error) {
		printk(KERN_ERR "create bin file fail\n");
		kfree(flash_encrypted);
		return error;
	}

	return 0;
}

module_init(flash_sysfs_init);
module_exit(flash_sysfs_exit);