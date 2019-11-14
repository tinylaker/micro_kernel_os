#include <ipc.h>
#include <stddef.h>
#include <svc_call.h>

int ipc_send_raw(int pid, void* data, uint32_t size) {
	return svc_call3(SYS_SEND_MSG, (int32_t)pid, (int32_t)data, (int32_t)size);
}

void* ipc_get_raw(int* from_pid,  uint32_t* size, uint8_t block) {
	return (void*)svc_call3(SYS_GET_MSG, (int32_t)from_pid, (int32_t)size, (int32_t)block);
}

void* ipc_get_from_raw(int from_pid,  uint32_t* size, uint8_t block) {
	return (void*)svc_call3(SYS_GET_MSG_FROM, (int32_t)from_pid, (int32_t)size, (int32_t)block);
}

int ipc_send(int to_pid, const proto_t* pkg) {
	if(to_pid < 0 || pkg == NULL)
		return -1;
	return ipc_send_raw(to_pid, pkg->data, pkg->size);
}

int ipc_get(int* from_pid,  proto_t* pkg, uint8_t block) {
	uint32_t size;
	void *data = ipc_get_raw(from_pid, &size, block);
	if(data == NULL)
		return -1;

	if(pkg != NULL) {
		proto_clear(pkg);
		proto_init(pkg, data, size);
		pkg->read_only = 0;
	}
	return 0;	
}

int ipc_get_from(int from_pid,  proto_t* pkg, uint8_t block) {
	uint32_t size;
	void *data = ipc_get_from_raw(from_pid, &size, block);
	if(data == NULL)
		return -1;

	if(pkg != NULL) {
		proto_clear(pkg);
		proto_init(pkg, data, size);
		pkg->read_only = 0;
	}
	return 0;	
}

int ipc_recv(int* from_pid,  proto_t* pkg) {
	if(pkg == NULL)
		return -1;

	while(1) {
		int res = ipc_get(from_pid, pkg, 1);
		if(res == 0)
			return 0;
	}
	return -1;
}

int ipc_recv_from(int from_pid,  proto_t* pkg) {
	if(pkg == NULL)
		return -1;

	while(1) {
		int res = ipc_get_from(from_pid, pkg, 1);
		if(res == 0)
			return 0;
	}
	return -1;
}

int ipc_call(int to_pid,  const proto_t* ipkg, proto_t* opkg) {
	if(to_pid < 0 || ipkg == NULL)
		return -1;

	if(ipc_send(to_pid, ipkg) != 0)
		return -1;

	if(opkg == NULL)
		return 0;

	//if(ipc_recv_from(to_pid, opkg) == 0)
	if(ipc_recv(NULL, opkg) == 0)
		return 0;
	return -1;
}
