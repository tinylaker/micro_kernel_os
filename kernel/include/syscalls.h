#ifndef SYSCALLS_H
#define SYSCALLS_H

enum {
	SYS_NONE = 0,
	SYS_CHAR_DEV_READ,
	SYS_CHAR_DEV_WRITE,

	SYS_INITRD,

	SYS_MALLOC,
	SYS_FREE,

	SYS_GET_PID,
	SYS_FORK,
	SYS_YIELD,
	SYS_WAIT_PID,
	SYS_SLEEP_ON,
	SYS_WAKEUP,
	SYS_EXIT,

	SYS_EXEC_ELF,

	SYS_SEND_MSG,
	SYS_GET_MSG,

	SYS_VFS_NEW_NODE,
	SYS_VFS_GET_MOUNT,
	SYS_VFS_MOUNT,
	SYS_VFS_UMOUNT,
	SYS_VFS_GET,
	SYS_VFS_FKID,
	SYS_VFS_NEXT,
	SYS_VFS_FATHER,
	SYS_VFS_SET,
	SYS_VFS_ADD,
	SYS_VFS_DEL,
	SYS_VFS_OPEN,

	SYS_VFS_PROC_CLOSE,
	SYS_VFS_PROC_SEEK,
	SYS_VFS_PROC_TELL,
	SYS_VFS_PROC_GET_BY_FD,
	SYS_VFS_PROC_DUP2,

	SYS_PROC_GET_CMD,
	SYS_PROC_SET_CWD,
	SYS_PROC_GET_CWD
};

#endif
