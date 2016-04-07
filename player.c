#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<net/if.h>
#include<time.h>
#include<ifaddrs.h>
#include<pthread.h>
#include "common.h"

player self;
int master_port;
struct hostent *mhp;

struct sockaddr_in plyr,sock_to_m;
char buff2[1024];
char buff3[1024];
char buff4[1024];
struct sockaddr_in sock_in,sin2,sock_out;
int rcv_sock,send_sock;
int p,rc;
struct thread_args{
	int *sock;
	struct sockaddr_in sin;
};

void *func1(void *arg){
	struct thread_args *t;
	t=(struct thread_args *)(arg);
	//printf("%d\n",rcv_sock);
	//printf("Waiting in thread for accept\n");
	memset(&sock_in,0,sizeof(sock_in));
	int len=sizeof(sock_in);
	p=accept(rcv_sock, (struct sockaddr *) &sock_in, &len);
	//printf("%d\n",p);
	if (p < 0){
              perror("Failed to accept from neighbor..");
              exit(p);
        }
	//printf("Accepted from player\n");
	pthread_exit(NULL);
	//return 0;
}

void *func2(void *arg){
	struct thread_args *t;
	t=(struct thread_args *)(arg);
	int rc;
	//printf("%d\n",send_sock);
	sleep(1);
	rc = connect(send_sock,(struct sockaddr *) &sin2, sizeof (sin2));
	if (rc < 0){
              perror("Failed to connect to neighbor..");
              exit(rc);
        }
	//printf("Connected to player\n");
	pthread_exit(NULL);
	//return;
}

int main(int argc, char *argv[]){
	int s,rcv,temp;
	struct sockaddr_in sc;
	char str[1024];
	char host[64];
	char buff[1024];
	
	char *token;
	time_t t;
	/* read host and port number from command line */
	if (argc != 3){
       		fprintf(stderr, "Usage: %s <host-name> <port-number>\n", argv[0]);
        	exit(1);
    	}
	
	mhp=gethostbyname(argv[1]);
	if (mhp == NULL){
        	fprintf(stderr, "Host not found!\n");
        	exit(1);
    	}
	master_port = atoi(argv[2]);
	s=socket(AF_INET,SOCK_STREAM,0);
	if(s < 0){
		perror("Failed to create socket!\n");
		exit(1);
	}
	sc.sin_family = AF_INET;
	sc.sin_port = htons(master_port);
	memcpy(&sc.sin_addr, mhp->h_addr_list[0], mhp->h_length);
	connect(s, (struct sockaddr *)&sc, sizeof(sc));
	//send(s,sc,sizeof(sc),0);
	
	//generate random listening port
	int l_port;
	srand((unsigned) time(&t));	
	l_port = rand() % (65000 - 2000);
	memset(str, 0, 1024);
    	sprintf(str, "%d", l_port);
	//printf("Random port generated %d\n",l_port);

	//start receiving
	int r,hop,port;
	struct hostent *hp, *ihp;
	struct sockaddr_in sin1;
	char msg[1024];
	rcv_sock = socket(AF_INET, SOCK_STREAM, 0);
	//printf("%d\n",rcv_sock);
	if(rcv_sock < 0){
		perror("Failed to create socket!\n");
		exit(1);
	}
	memset(host, 0, 64);
	gethostname(host, sizeof(host));
	hp = gethostbyname(host);
	if (hp == NULL){
        	fprintf(stderr, "Host not found!\n");
        	exit(1);
    	}
	memset(&sin1, 0, sizeof(sin1));
	sin1.sin_family = AF_INET;
    	sin1.sin_port = htons(l_port);
	sin1.sin_addr.s_addr = htonl(INADDR_ANY);
    	//memcpy(&sin1.sin_addr, hp->h_addr_list[0], hp->h_length);
	
	while(1){
		if(bind(rcv_sock, (struct sockaddr *) &sin1, sizeof(sin1)) < 0){
			l_port = rand() % (65000 - 2000);			
			sin1.sin_port = htons(l_port);
		}
		else{
			break;
		}
	}

    	int len = send(s, str, strlen(str), 0);
    	if (len < 0){
        	perror("Error while sending");
    	}
	
	//close(s);
	memset(buff, 0, 1024);
	rcv = recv(s,buff,1024,0);

	token=strtok(buff,";");	

	if(!strcmp(token,"NEIGHBORS")){
		token = strtok(NULL, ";");
            	temp = atoi(token);
            	self.id = temp;

            	token = strtok(NULL, ";");
            	temp = atoi(token);
            	self.left_id = temp;

            	token = strtok(NULL, ";");
            	strcpy(self.left_ip, token);

            	token = strtok(NULL, ";");
            	self.left_port = atoi(token);
		//printf("Left port is %d\n",self.left_port);

            	token = strtok(NULL, ";");
            	temp = atoi(token);
            	self.right_id = temp;

            	token = strtok(NULL, ";");
            	strcpy(self.right_ip, token);

            	token = strtok(NULL, ";");
            	self.right_port = atoi(token);
		//printf("Right port is %d\n",self.right_port);

            	printf("Connected as player %d\n", self.id);
	}

	listen(rcv_sock,3);
	//printf("Return from listen %d\n",rc);
	//printf("Waiting to accept from player..\n");
	/*p=accept(rcv_sock, (struct sockaddr *) &sock_in, sizeof(sock_in));
	printf("Accepted from player\n");*/
	//int tc1,tc2;
	struct hostent *nhp;
	nhp=gethostbyname(self.right_ip);
	if (nhp == NULL){
        	fprintf(stderr, "Host not found!\n");
        	exit(1);
    	}

	//printf("%s\n",nhp->h_name);

	pthread_t thread1, thread2;
	struct thread_args *args1 = (struct thread_args *)malloc(sizeof (struct thread_args));
	struct thread_args *args2 = (struct thread_args *)malloc(sizeof (struct thread_args));
	args1->sock = &rcv_sock;
	args1->sin = sock_in;
	
	send_sock = socket(AF_INET, SOCK_STREAM, 0);
	//printf("%d\n",send_sock);
	if(send_sock < 0){
		perror("Failed to create socket!\n");
		exit(1);
	}
	memset(&sin2,0,sizeof(sin2));
	sin2.sin_family = AF_INET;
    	sin2.sin_port = htons(self.right_port);
    	memcpy(&sin2.sin_addr, nhp->h_addr_list[0], nhp->h_length);
	
	args2->sock = &send_sock;
	args2->sin = sock_out;
	//sleep(1);

	pthread_create(&thread1, NULL, func1, NULL);

	pthread_create(&thread2, NULL, func2, NULL);
      	
	pthread_join(thread2, NULL);
	//printf("Thread 2 done\n");
	pthread_join(thread1, NULL);

	//printf("Threads done\n");

	int signal = send(s,"1",strlen("1"),0);
	if ( signal != strlen("1") ) {
      		perror("send");
      		exit(1);
    	}

	fd_set rdfs;
	struct timeval tv;
	int retval;
	int n;

	
	//printf("before fd_set select\n");
	while(1){
		FD_ZERO(&rdfs);
		FD_SET(p, &rdfs);
		if(n<p) n=p;
		FD_SET(s, &rdfs);
		if(n<s)	n=s;
		FD_SET(send_sock, &rdfs);
		if(n<send_sock) n=send_sock;
		n++;	
		tv.tv_sec = 100;
		tv.tv_usec = 0;
		retval = select(n, &rdfs, NULL, NULL, &tv);
		//printf("return from select %d\n",retval);
		if(retval<0){
			perror("Error in select()\n");
		}
		else if(retval == 0){	
			perror("Timeout\n");
		}
		else{
			if(FD_ISSET(s, &rdfs)){
				memset(buff2,0,sizeof(buff2));
				rcv = recv(s, buff2, sizeof(buff2), 0);
				//puts(buff2);
				token=strtok(buff2,";");
				if(!strcmp(token,"Shutdown")){
					//printf("%s\n",token);
					send(p,"Shutdown",strlen("Shutdown"),0);
					send(send_sock,"Shutdown",strlen("Shutdown"),0);
					break;
				}
				//printf ("from master %s\n",token);
				else if(!strcmp(token,"Potato")){
					token=strtok(NULL,";");
					hop=atoi(token);
					//printf("%d\n",hop);
					if(hop==1){
						hop--;
						printf("I'm it\n");
						//token=strtok(NULL,";");
						memset(msg,0,sizeof(msg));
						sprintf(msg,"%s%s%d","Potato",";",self.id);
						//printf("%s\n",msg);
						send(s,msg,strlen(msg),0);
						send(p,"Shutdown",strlen("Shutdown"),0);
						send(send_sock,"Shutdown",strlen("Shutdown"),0);
						break;
					}
					else if(hop>1){
						hop--;
						int temp_neighbor = rand() % 2;
						//printf("selected ngb %d\n",temp_neighbor);
						if(temp_neighbor == 0){
							//token=strtok(NULL,";");
							//printf("%s\n",token);
							memset(msg,0,sizeof(msg));
							sprintf(msg,"%s%s%d%s%d","Potato",";",hop,";",self.id);
							//printf("%s\n",msg);
							printf("Sending potato to %d\n",self.left_id);
							//puts(msg);
							send(p,msg,strlen(msg),0);
						}
						else{
							//token=strtok(NULL,";");
							memset(msg,0,sizeof(msg));
							sprintf(msg,"%s%s%d%s%d","Potato",";",hop,";",self.id);
							printf("Sending potato to %d\n",self.right_id);
							//puts(msg);
							send(send_sock,msg,strlen(msg),0);
						}
					}
				}
			}
			else if(FD_ISSET(p, &rdfs)){
				memset(buff3,0,sizeof(buff3));
				//printf("received from p\n");
				rcv = recv(p, buff3, sizeof(buff3), 0);
				//puts(buff3);
				token=strtok(buff3,";");
				if(!strcmp(token,"Shutdown")){
					//printf("%s\n",token);
					send(p,"Shutdown",strlen("Shutdown"),0);
					send(send_sock,"Shutdown",strlen("Shutdown"),0);
					break;
				}
				//printf ("from left ngb %s\n",token);
				else if(!strcmp(token,"Potato")){
					token=strtok(NULL,";");
					hop=atoi(token);
					if(hop==1){
						hop--;
						printf("I'm it\n");
						token=strtok(NULL,";");
						memset(msg,0,sizeof(msg));
						sprintf(msg,"%s%s%s%s%d","Potato",";",token,",",self.id);
						//printf("%s\n",msg);
						send(s,msg,strlen(msg),0);
						send(p,"Shutdown",strlen("Shutdown"),0);
						send(send_sock,"Shutdown",strlen("Shutdown"),0);
						break;
					}
					else if(hop>1){
						hop--;
						int temp_neighbor = rand() % 2;
						if(temp_neighbor == 0){
							token=strtok(NULL,";");
							//printf("token %s\n",token);
							memset(msg,0,sizeof(msg));
							sprintf(msg,"%s%s%d%s%s%s%d","Potato",";",hop,";",token,",",self.id);
							printf("Sending potato to %d\n",self.left_id);
							//printf("%s\n",msg);
							send(p,msg,strlen(msg),0);
						}
						else{
							token=strtok(NULL,";");
							//printf("token %s\n",token);
							memset(msg,0,sizeof(msg));
							sprintf(msg,"%s%s%d%s%s%s%d","Potato",";",hop,";",token,",",self.id);
							printf("Sending potato to %d\n",self.right_id);
							//printf("%s\n",msg);
							send(send_sock,msg,strlen(msg),0);
						}
					}
				}
			}
			else if(FD_ISSET(send_sock, &rdfs)){
				memset(buff4,0,sizeof(buff4));
				//printf("received from send_sock\n");
				rcv = recv(send_sock, buff4, sizeof(buff4), 0);
				//puts(buff4);
				token=strtok(buff4,";");
				if(!strcmp(token,"Shutdown")){
					//printf("%s\n",token);
					send(p,"Shutdown",strlen("Shutdown"),0);
					send(send_sock,"Shutdown",strlen("Shutdown"),0);
					break;
				}
				//printf ("from right ngb %s\n",token);
				else if(!strcmp(token,"Potato")){
					token=strtok(NULL,";");
					hop=atoi(token);
					if(hop==1){
						hop--;
						printf("I'm it\n");
						token=strtok(NULL,";");
						memset(msg,0,sizeof(msg));
						sprintf(msg,"%s%s%s%s%d","Potato",";",token,",",self.id);
						//printf("%s\n",msg);
						send(s,msg,strlen(msg),0);
						send(p,"Shutdown",strlen("Shutdown"),0);
						send(send_sock,"Shutdown",strlen("Shutdown"),0);
						break;
					}
					else if(hop>1){
						hop--;
						int temp_neighbor = rand() % 2;
						if(temp_neighbor == 0){
							token=strtok(NULL,";");
							//printf("token %s\n",token);
							memset(msg,0,sizeof(msg));
							sprintf(msg,"%s%s%d%s%s%s%d","Potato",";",hop,";",token,",",self.id);
							printf("Sending potato to %d\n",self.left_id);
							//printf("%s\n",msg);
							send(p,msg,strlen(msg),0);
						}
						else{
							token=strtok(NULL,";");
							//printf("token %s\n",token);
							memset(msg,0,sizeof(msg));
							sprintf(msg,"%s%s%d%s%s%s%d","Potato",";",hop,";",token,",",self.id);
							printf("Sending potato to %d\n",self.right_id);
							//printf("%s\n",msg);
							send(send_sock,msg,strlen(msg),0);
						}
					}
				}
			}
		}
	}
	close(p);
	close(send_sock);
	return 0;
}
