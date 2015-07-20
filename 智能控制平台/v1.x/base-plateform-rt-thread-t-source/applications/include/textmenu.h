#ifndef TEXTMENU_H__
#define TEXTMENU_H__

typedef int (*do_textmenu_entry)(int);


/*
 * ϵͳ�˵�����������֯, �˵����ĸ��ڵ�vertical_no, horizontal_no��Ϊ0
 * �˵����ĵ�1��ڵ��vertical_no��Ϊ1
 * �˵����ĵ�2��ڵ��vertical_no��Ϊ2
 *
 * ÿ��ڵ��horizontal_no����0��ʼ, ���������α��
 */
struct textmenu_entry_st {
	int vertical_no;	/* ϵͳһ���˵����Ϊ1 */
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

