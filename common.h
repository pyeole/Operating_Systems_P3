#ifndef COMMON
#define COMMON

typedef struct{
	int id;
	int left_id;
	int right_id;
	char ip[1024];
	char left_ip[1024];
	char right_ip[1024];
	int left_port;
	int right_port;
	int listen;
}player;

extern player *player_list;

#endif
