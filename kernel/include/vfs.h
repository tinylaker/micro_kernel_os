#ifndef VFS_TREE_H
#define VFS_TREE_H

#include <fsinfo.h>

typedef struct vfs_node {
	struct vfs_node* father; 
	struct vfs_node* first_kid; /*first child*/
	struct vfs_node* last_kid; /*last child*/
	struct vfs_node* next; /*next brother*/
	struct vfs_node* prev; /*prev brother*/

	uint32_t kids_num;

	fsinfo_t fsinfo;
} vfs_node_t;

vfs_node_t* vfs_new_node(void);

vfs_node_t* vfs_simple_add(vfs_node_t* father, const char* name);

vfs_node_t* vfs_simple_get(vfs_node_t* father, const char* name);

vfs_node_t* vfs_get(vfs_node_t* father, const char* name);

int32_t vfs_mount(vfs_node_t* org, vfs_node_t* node, uint32_t access);

void vfs_umount(vfs_node_t* node);

void vfs_del(vfs_node_t* node);

void vfs_init(void);

vfs_node_t* vfs_root(void);

#endif