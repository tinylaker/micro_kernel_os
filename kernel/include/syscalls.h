#ifndef SYSCALLS_H
#define SYSCALLS_H

enum {
	SYS_NONE = 0,
	SYS_DEV_CHAR_READ,
	SYS_DEV_CHAR_READ_NBLOCK,
	SYS_DEV_CHAR_WRITE,
	SYS_DEV_CHAR_WRITE_NBLOCK,
	SYS_DEV_BLOCK_READ,
	SYS_DEV_BLOCK_WRITE,
	SYS_DEV_BLOCK_READ_DONE,
	SYS_DEV_BLOCK_WRITE_DONE,
	SYS_DEV_OP,

	SYS_FRAMEBUFFER,

	SYS_MALLOC,
	SYS_FREE,

	SYS_GET_PID,
	SYS_FORK,
	SYS_YIELD,
	SYS_WAIT_PID,
	SYS_SLEEP_ON,
	SYS_SLEEP,
	SYS_WAKEUP,
	SYS_EXIT,
	SYS_KILL,
	SYS_DETACH,

	SYS_EXEC_ELF,

	SYS_GET_KEVENT,

	SYS_SEND_MSG,
	SYS_GET_MSG,
	SYS_GET_MSG_NBLOCK,

	SYS_VFS_NEW_NODE,
	SYS_VFS_GET_MOUNT,
	SYS_VFS_GET_MOUNT_BY_ID,
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
	SYS_VFS_PROC_DUP,
	SYS_VFS_PROC_DUP2,

	SYS_PIPE_OPEN,
	SYS_PIPE_READ,
	SYS_PIPE_READ_NBLOCK,
	SYS_PIPE_WRITE,
	SYS_PIPE_WRITE_NBLOCK,

	SYS_PROC_GET_CMD,
	SYS_PROC_SET_CWD,
	SYS_PROC_GET_CWD,

	SYS_PROC_SET_ENV,
	SYS_PROC_GET_ENV,
	SYS_PROC_GET_ENV_NAME,
	SYS_PROC_GET_ENV_VALUE,

	SYS_PROC_SHM_ALLOC,
	SYS_PROC_SHM_MAP,
	SYS_PROC_SHM_UNMAP,
	SYS_PROC_SHM_REF,

	SYS_GET_SYSINFO,
	SYS_GET_PROCS,

	SYS_SET_GLOBAL,
	SYS_GET_GLOBAL,

	SYS_THREAD
};

#endif
