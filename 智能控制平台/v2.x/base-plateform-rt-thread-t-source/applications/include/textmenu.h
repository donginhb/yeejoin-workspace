#ifndef TEXTMENU_H__
#define TEXTMENU_H__

typedef int (*do_textmenu_entry)(int);


/*
 * 系统菜单以树进行组织, 菜单树的根节点vertical_no, horizontal_no都为0
 * 菜单树的第1层节点的vertical_no都为1
 * 菜单树的第2层节点的vertical_no都为2
 *
 * 每层节点的horizontal_no都从0开始, 从左到右依次编号
 */
struct textmenu_entry_st {
	int vertical_no;	/* 系统一级菜单编号为1 */
	int horizontal_no;	/*  */
	struct list_head parent_child;
	struct list_head sibling;

	do_textmenu_entry pfun;
};


struct sys_textmenu_info {
	struct textmenu_entry_st *level_1_head;
	struct textmenu_entry_st *active_menu_entry;
};




#endif

