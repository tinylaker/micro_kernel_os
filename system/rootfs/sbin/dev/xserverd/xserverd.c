#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cmain.h>
#include <string.h>
#include <vfs.h>
#include <vdevice.h>
#include <syscall.h>
#include <dev/device.h>
#include <shm.h>
#include <graph/graph.h>
#include <dev/fbinfo.h>
#include <ipc.h>
#include <x/xcntl.h>
#include <x/xevent.h>
#include <x/xwm.h>

typedef struct st_xview_ev {
	xevent_t event;
	struct st_xview_ev* next;
} xview_event_t;

typedef struct st_xview {
	int fd;
	int from_pid;
	xinfo_t xinfo;
	int dirty;

	struct st_xview *next;
	struct st_xview *prev;
	xview_event_t* event_head;
	xview_event_t* event_tail;
} xview_t;

typedef struct {
	gpos_t old_pos;
	gpos_t cpos;
	gsize_t size;
	graph_t* g;
} cursor_t;

typedef struct {
	xview_t* view; //moving or resizing;
	gpos_t old_pos;
} x_current_t;

typedef struct {
	int fb_fd;
	int keyb_fd;
	int mouse_fd;
	int xwm_pid;
	int shm_id;
	int dirty;
	graph_t* g;
	cursor_t cursor;

	xview_t* view_head;
	xview_t* view_tail;

	x_current_t current;
} x_t;

static int xserver_mount(fsinfo_t* mnt_point, mount_info_t* mnt_info, void* p) {
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

static int xserver_umount(fsinfo_t* info, void* p) {
	(void)p;
	vfs_umount(info);
	return 0;
}

static void draw_win_frame(x_t* x, xview_t* view) {
	if((view->xinfo.style & X_STYLE_NO_FRAME) != 0)
		return;

	proto_t in, out;
	proto_init(&in, NULL, 0);
	proto_init(&out, NULL, 0);

	proto_add_int(&in, XWM_CNTL_DRAW_FRAME); // 0 for draw view frame
	proto_add_int(&in, x->shm_id);
	proto_add_int(&in, x->g->w);
	proto_add_int(&in, x->g->h);
	proto_add(&in, &view->xinfo, sizeof(xinfo_t));
	if(view == x->view_tail)
		proto_add_int(&in, 1); //top win
	else
		proto_add_int(&in, 0);

	ipc_call(x->xwm_pid, &in, &out);
	proto_clear(&in);
	proto_clear(&out);
}

static void draw_desktop(x_t* x) {
	proto_t in, out;
	proto_init(&in, NULL, 0);
	proto_init(&out, NULL, 0);

	proto_add_int(&in, XWM_CNTL_DRAW_DESKTOP); // 1 for draw desktop
	proto_add_int(&in, x->shm_id);
	proto_add_int(&in, x->g->w);
	proto_add_int(&in, x->g->h);

	ipc_call(x->xwm_pid, &in, &out);
	proto_clear(&in);
	proto_clear(&out);
}

static int draw_view(x_t* x, xview_t* view) {
	void* gbuf = shm_map(view->xinfo.shm_id);
	if(gbuf == NULL) {
		return -1;
	}

	if(x->dirty == 0 && view->dirty == 0) {
		shm_unmap(view->xinfo.shm_id);
		return 0;
	}

	graph_t* g = graph_new(gbuf, view->xinfo.r.w, view->xinfo.r.h);
	blt(g, 0, 0, view->xinfo.r.w, view->xinfo.r.h,
			x->g, view->xinfo.r.x, view->xinfo.r.y, view->xinfo.r.w, view->xinfo.r.h);
	graph_free(g);
	shm_unmap(view->xinfo.shm_id);

	draw_win_frame(x, view);
	view->dirty = 0;
	return 0;
}

static void remove_view(x_t* x, xview_t* view) {
	if(view->prev != NULL)
		view->prev->next = view->next;
	if(view->next != NULL)
		view->next->prev = view->prev;
	if(x->view_tail == view)
		x->view_tail = view->prev;
	if(x->view_head == view)
		x->view_head = view->next;
	view->next = view->prev = NULL;
	x->dirty = 1;
}

static void push_view(x_t* x, xview_t* view) {
	if(x->view_tail != NULL) {
		x->view_tail->next = view;
		view->prev = x->view_tail;
		x->view_tail = view;
	}
	else {
		x->view_tail = x->view_head = view;
	}
	x->dirty = 1;
}

static void x_del_view(x_t* x, xview_t* view) {
	remove_view(x, view);
	free(view);
}

static void hide_cursor(x_t* x) {
	if(x->cursor.g == NULL) {
		x->cursor.g = graph_new(NULL, x->cursor.size.w, x->cursor.size.h);
		blt(x->g, x->cursor.old_pos.x, x->cursor.old_pos.y, x->cursor.size.w, x->cursor.size.h,
				x->cursor.g, 0, 0, x->cursor.size.w, x->cursor.size.h);
	}
	else  {
		blt(x->cursor.g, 0, 0, x->cursor.size.w, x->cursor.size.h,
				x->g, x->cursor.old_pos.x, x->cursor.old_pos.y, x->cursor.size.w, x->cursor.size.h);
	}
}

static inline void draw_cursor(x_t* x) {
	int32_t mx = x->cursor.cpos.x;
	int32_t my = x->cursor.cpos.y;
	int32_t mw = x->cursor.size.w;
	int32_t mh = x->cursor.size.h;

	if(x->cursor.g == NULL)
		return;

	//blt(x->g, x->cursor.old_pos.x, x->cursor.old_pos.y, mw, mh,
	//		x->cursor.g, 0, 0, mw, mh);
	blt(x->g, mx, my, mw, mh,
			x->cursor.g, 0, 0, mw, mh);

	line(x->g, mx+1, my, mx+mw-1, my+mh-2, 0xffffffff);
	line(x->g, mx, my, mx+mw-1, my+mh-1, 0xff000000);
	line(x->g, mx, my+1, mx+mw-2, my+mh-1, 0xffffffff);

	line(x->g, mx, my+mh-2, mx+mw-2, my, 0xffffffff);
	line(x->g, mx, my+mh-1, mx+mw-1, my, 0xff000000);
	line(x->g, mx+1, my+mh-1, mx+mw-1, my+1, 0xffffffff);
	x->cursor.old_pos.x = mx;
	x->cursor.old_pos.y = my;
}

static void x_repaint(x_t* x) {
	hide_cursor(x);
	if(x->dirty != 0)
		draw_desktop(x);

	xview_t* view = x->view_head;
	int rep = 0;
	while(view != NULL) {
		int res = draw_view(x, view);
		xview_t* v = view;
		view = view->next;
		if(res != 0) {//client close/broken. remove it.
			x_del_view(x, v);
			rep = 1;
		}
	}
	draw_cursor(x);

	flush(x->fb_fd);
	if(rep == 0)
		x->dirty = 0;
}

static xview_t* x_get_view(x_t* x, int fd, int from_pid) {
	xview_t* view = x->view_head;
	while(view != NULL) {
		if(view->fd == fd && view->from_pid == from_pid)
			return view;
		view = view->next;
	}
	return NULL;
}

static int x_update(int fd, int from_pid, proto_t* in, x_t* x) {
	xinfo_t xinfo;
	int sz = sizeof(xinfo_t);
	if(fd < 0 || proto_read_to(in, &xinfo, sz) != sz)
		return -1;
	
	xview_t* view = x_get_view(x, fd, from_pid);
	if(view == NULL)
		return -1;

	if(view != x->view_tail ||
			view->xinfo.r.x != xinfo.r.x ||
			view->xinfo.r.y != xinfo.r.y ||
			view->xinfo.r.w != xinfo.r.w ||
			view->xinfo.r.h != xinfo.r.h) {
		x->dirty = 1;
	}
	
	memcpy(&view->xinfo, &xinfo, sizeof(xinfo_t));
	view->dirty = 1;
	x_repaint(x);
	return 0;
}

static int x_new_view(int fd, int from_pid, proto_t* in, x_t* x) {
	xinfo_t xinfo;
	int sz = sizeof(xinfo_t);
	if(fd < 0 || proto_read_to(in, &xinfo, sz) != sz)
		return -1;
	xview_t* view = (xview_t*)malloc(sizeof(xview_t));
	if(view == NULL)
		return -1;
	memset(view, 0, sizeof(xview_t));

	memcpy(&view->xinfo, &xinfo, sizeof(xinfo_t));
	view->dirty = 1;
	view->fd = fd;
	view->from_pid = from_pid;
	push_view(x, view);
	return 0;
}

static int x_get_event(int fd, int from_pid, x_t* x, proto_t* out) {
	xview_t* view = x_get_view(x, fd, from_pid);
	if(view == NULL || view->event_head == NULL)
		return -1;

	xview_event_t* e = view->event_head;
	view->event_head = view->event_head->next;
	if(view->event_head == NULL)
		view->event_tail = NULL;

	proto_add(out, &e->event, sizeof(xevent_t));
	free(e);
	return 0;
}

static int x_scr_info(x_t* x, proto_t* out) {
	xscreen_t scr;	
	scr.id = 0;
	scr.size.w = x->g->w;
	scr.size.h = x->g->h;
	proto_add(out, &scr, sizeof(xscreen_t));
	return 0;
}


static int xserver_cntl(int fd, int from_pid, fsinfo_t* info, int cmd, proto_t* in, proto_t* out, void* p) {
	(void)info;
	x_t* x = (x_t*)p;

	if(cmd == X_CNTL_NEW) {
		return x_new_view(fd, from_pid, in, x);
	}
	else if(cmd == X_CNTL_UPDATE) {
		return x_update(fd, from_pid, in, x);
	}
	else if(cmd == X_CNTL_GET_EVT) {
		return x_get_event(fd, from_pid, x, out);
	}
	else if(cmd == X_CNTL_SCR_INFO) {
		return x_scr_info(x, out);
	}
	return 0;
}

static int xserver_close(int fd, int from_pid, fsinfo_t* info, void* p) {
	(void)info;
	x_t* x = (x_t*)p;
	
	xview_t* view = x_get_view(x, fd, from_pid);
	if(view == NULL)
		return -1;

	x_del_view(x, view);	
	return 0;
}

static int x_init(x_t* x) {
	memset(x, 0, sizeof(x_t));
	x->view_head = NULL;	
	x->view_tail = NULL;	

	int fd = open("/dev/keyb0", O_RDONLY);
	if(fd < 0)
		return -1;
	x->keyb_fd = fd;

	fd = open("/dev/mouse0", O_RDONLY);
	if(fd < 0) {
		close(x->keyb_fd);
		return -1;
	}
	x->mouse_fd = fd;

	fd = open("/dev/fb0", O_RDONLY);
	if(fd < 0) {
		close(x->keyb_fd);
		close(x->mouse_fd);
		return -1;
	}
	x->fb_fd = fd;

	int id = dma(fd, NULL);
	if(id <= 0) {
		close(x->keyb_fd);
		close(x->mouse_fd);
		close(x->fb_fd);
	}

	void* gbuf = shm_map(id);
	if(gbuf == NULL) {
		close(x->keyb_fd);
		close(x->mouse_fd);
		close(x->fb_fd);
	}

	fbinfo_t info;
	proto_t out;
	proto_init(&out, NULL, 0);

	if(cntl_raw(fd, CNTL_INFO, NULL, &out) != 0) {
		shm_unmap(id);
		close(x->keyb_fd);
		close(x->mouse_fd);
		close(x->fb_fd);
		return -1;
	}

	proto_read_to(&out, &info, sizeof(fbinfo_t));
	x->g = graph_new(gbuf, info.width, info.height);
	proto_clear(&out);
	x->shm_id = id;
	x->dirty = 1;

	x->cursor.size.w = 15;
	x->cursor.size.h = 15;
	x->cursor.cpos.x = info.width/2;
	x->cursor.cpos.y = info.height/2; 
	return 0;
}	

static int get_win_frame_pos(x_t* x, xview_t* view) {
	if((view->xinfo.style & X_STYLE_NO_FRAME) != 0)
		return -1;

	proto_t in, out;
	proto_init(&in, NULL, 0);
	proto_init(&out, NULL, 0);

	proto_add_int(&in, XWM_CNTL_GET_POS); // 2 for get_win_frame_posos
	proto_add_int(&in, x->shm_id);
	proto_add_int(&in, x->g->w);
	proto_add_int(&in, x->g->h);
	proto_add_int(&in, x->cursor.cpos.x);
	proto_add_int(&in, x->cursor.cpos.y);
	proto_add(&in, &view->xinfo, sizeof(xinfo_t));
	ipc_call(x->xwm_pid, &in, &out);
	proto_clear(&in);

	int res = proto_read_int(&out);
	proto_clear(&out);
	return res;
}

static xview_t* get_mouse_owner(x_t* x, int* win_frame_pos) {
	xview_t* view = x->view_tail;
	if(win_frame_pos != NULL)
		*win_frame_pos = -1;

	while(view != NULL) {
		if(x->cursor.cpos.x >= view->xinfo.r.x && x->cursor.cpos.x < (view->xinfo.r.x+view->xinfo.r.w) &&
				x->cursor.cpos.y >= view->xinfo.r.y && x->cursor.cpos.y < (view->xinfo.r.y+view->xinfo.r.h))
			return view;
		int pos = get_win_frame_pos(x, view);
		if(pos >= 0) {
			if(win_frame_pos != NULL)
				*win_frame_pos = pos;
			return view;
		}
		view = view->prev;
	}
	return NULL;
}

static void x_push_event(xview_t* view, xview_event_t* e) {
	if(view->event_tail != NULL)
		view->event_tail->next = e;
	else
		view->event_head = e;
	view->event_tail = e;
}

static void mouse_cxy(x_t* x, int32_t rx, int32_t ry) {
	x->cursor.cpos.x += rx;
	x->cursor.cpos.y += ry;
	if(x->cursor.cpos.x < 0)
		x->cursor.cpos.x = 0;
	if(x->cursor.cpos.x >= (int32_t)x->g->w)
		x->cursor.cpos.x = x->g->w;
	if(x->cursor.cpos.y < 0)
		x->cursor.cpos.y = 0;
	if(x->cursor.cpos.y >= (int32_t)x->g->h)
		x->cursor.cpos.y = x->g->h;
}

static int mouse_handle(x_t* x, int8_t state, int32_t rx, int32_t ry) {
	mouse_cxy(x, rx, ry);

	xview_event_t* e = (xview_event_t*)malloc(sizeof(xview_event_t));
	e->next = NULL;
	e->event.type = XEVT_MOUSE;
	e->event.state = XEVT_MOUSE_MOVE;
	e->event.value.mouse.x = x->cursor.cpos.x;
	e->event.value.mouse.y = x->cursor.cpos.y;
	e->event.value.mouse.rx = rx;
	e->event.value.mouse.ry = ry;

	int pos = -1;
	xview_t* view;
	if(x->current.view != NULL)
		view = x->current.view;
	else
		view = get_mouse_owner(x, &pos);

	if(view == NULL) {
		free(e);
		x_repaint(x);
		return -1;
	}

	if(state == 2) { //down
		if(pos == XWM_FRAME_CLOSE) { //window close
			e->event.type = XEVT_WIN;
			e->event.value.window.event = XEVT_WIN_CLOSE;
		}
		else { // mouse down
			if(view != x->view_tail) {
				remove_view(x, view);
				push_view(x, view);
			}
			if(pos == XWM_FRAME_TITLE) {//window title 
				x->current.view = view;
				x->current.old_pos.x = x->cursor.cpos.x;
				x->current.old_pos.y = x->cursor.cpos.y;
			}
			e->event.state = XEVT_MOUSE_DOWN;
		}
	}
	else if(state == 1) {
		e->event.state = XEVT_MOUSE_UP;
		if(x->current.view == view) {
			e->event.type = XEVT_WIN;
			e->event.value.window.event = XEVT_WIN_MOVE_TO;
			e->event.value.window.v0 = x->cursor.cpos.x - x->current.old_pos.x;
			e->event.value.window.v1 = x->cursor.cpos.y - x->current.old_pos.y;
		}
		x->current.view = NULL;
	}
/*
	if(x->current.view == view) {
		e->event.type = XEVT_WIN;
		e->event.value.window.event = XEVT_WIN_MOVE;
		e->event.value.window.v0 = rx;
		e->event.value.window.v1 = ry;
	}
	*/

	x_push_event(view, e);
	x_repaint(x);
	return -1;
}

static int xserver_loop_step(void* p) {
	x_t* x = (x_t*)p;
	int8_t v;
	//read keyb
	int rd = read(x->keyb_fd, &v, 1);
	if(rd == 1) {
		xview_t* topv = x->view_tail; 
		if(topv != NULL) {
			xview_event_t* e = (xview_event_t*)malloc(sizeof(xview_event_t));
			e->next = NULL;
			e->event.type = XEVT_KEYB;
			e->event.value.keyboard.value = v;
			x_push_event(topv, e);
		}
	}

	//read mouse
	if(read(x->mouse_fd, &v, 1) == 1) {
		int8_t state, rx, ry, rz;
		state = v;
		read(x->mouse_fd, &rx, 1);
		read(x->mouse_fd, &ry, 1);
		read(x->mouse_fd, &rz, 1); //z ...
		return mouse_handle(x, state, rx, ry);
	}

	if(x->dirty != 0) {
		x_repaint(x);	
	}
	return 0;
}

static void x_close(x_t* x) {
	close(x->keyb_fd);
	close(x->mouse_fd);
	graph_free(x->g);
	shm_unmap(x->shm_id);
	close(x->fb_fd);
}

int main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	int pid = fork();
	if(pid == 0) {
		exec("/sbin/x/xwm");
	}

	vdevice_t dev;
	memset(&dev, 0, sizeof(vdevice_t));
	strcpy(dev.name, "xserver");
	dev.mount = xserver_mount;
	dev.cntl = xserver_cntl;
	dev.close= xserver_close;
	dev.loop_step= xserver_loop_step;
	dev.umount = xserver_umount;

	fsinfo_t dev_info;
	vfs_get("/dev", &dev_info);

	fsinfo_t mnt_point;
	memset(&mnt_point, 0, sizeof(fsinfo_t));
	strcpy(mnt_point.name, "x");
	mnt_point.type = FS_TYPE_DEV;

	vfs_new_node(&mnt_point);
	vfs_add(&dev_info, &mnt_point);

	mount_info_t mnt_info;
	strcpy(mnt_info.dev_name, dev.name);
	mnt_info.dev_index = 0;
	mnt_info.access = 0;

	x_t x;
	if(x_init(&x) == 0) {
		x.xwm_pid = pid;
		device_run(&dev, &mnt_point, &mnt_info, &x, 0);
		x_close(&x);
	}
	return 0;
}
