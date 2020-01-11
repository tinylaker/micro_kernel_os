#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cmain.h>
#include <string.h>
#include <vfs.h>
#include <gpio.h>
#include <vdevice.h>
#include <dev/device.h>
#include "../lib/gpio_arch.h"

static int mount(fsinfo_t* mnt_point, mount_info_t* mnt_info, void* p) {
	(void)p;
	fsinfo_t info;
	memset(&info, 0, sizeof(fsinfo_t));
	strcpy(info.name, mnt_point->name);
	info.type = FS_TYPE_DEV;
	vfs_new_node(&info);

	if(vfs_mount(mnt_point, &info, mnt_info) != 0) {
		vfs_del(&info);
		return -1;
	}
	memcpy(mnt_point, &info, sizeof(fsinfo_t));
	return 0;
}

static int gpio_mount(fsinfo_t* info, mount_info_t* mnt_info, void* p) {
	mount(info, mnt_info, p);
	return 0;
}

static int gpio_fcntl(int fd, int from_pid, fsinfo_t* info, int cmd, proto_t* in, proto_t* out, void* p) {
	(void)fd;
	(void)from_pid;
	(void)info;
	(void)p;

	int32_t gpio_num = proto_read_int(in);
	if(cmd == 0) { //0: config
		int32_t v = proto_read_int(in);
		if(v == GPIO_MODE_INPUT)
			gpio_arch_config(gpio_num, GPIO_INPUT);
		else if(v == GPIO_MODE_OUTPUT)
			gpio_arch_config(gpio_num, GPIO_OUTPUT);
		else
			gpio_arch_config(gpio_num, v);
	}
	else if(cmd == 1) { //1: pull
		int32_t v = proto_read_int(in);
		if(v == GPIO_PULL_DOWN)
			gpio_arch_pull(gpio_num, 1);
		else if(v == GPIO_PULL_UP)
			gpio_arch_pull(gpio_num, 2);
		else
			gpio_arch_pull(gpio_num, v);
	}
	else if(cmd == 2) { //2: write
		int32_t v = proto_read_int(in);
		gpio_arch_write(gpio_num, v);
	}
	else if(cmd == 3) { //3: read
		int32_t v = gpio_arch_read(gpio_num);
		proto_add_int(out, v);
	}
	return 0;
}

static int gpio_umount(fsinfo_t* info, void* p) {
	(void)p;
	vfs_umount(info);
	return 0;
}

int main(int argc, char** argv) {
	gpio_arch_init();

	fsinfo_t mnt_point;
	const char* mnt_name = argc > 1 ? argv[1]: "/dev/gpio";
	vfs_create(mnt_name, &mnt_point, FS_TYPE_DEV);

	vdevice_t dev;
	memset(&dev, 0, sizeof(vdevice_t));
	strcpy(dev.name, "gpio");
	dev.mount = gpio_mount;
	dev.fcntl = gpio_fcntl;
	dev.umount = gpio_umount;

	mount_info_t mnt_info;
	strcpy(mnt_info.dev_name, dev.name);
	mnt_info.dev_index = 0;
	mnt_info.access = 0;

	device_run(&dev, &mnt_point, &mnt_info, NULL, 1);
	return 0;
}
