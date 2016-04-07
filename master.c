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
#include "common.h"

/*void *func1(int sock){
	//read and print strings received over buffer
	char buff[1024];
	char *pch;
	int len;
	while(1){
		len = recv(sock,buff,255,0);
		if(len < 0){
			perror("Failed to receive!\n");
			exit(1);	
		}			
		pch = strtok(buff,":,");
		printf("Trace of potato:\n");
		while(pch != NULL){
			if(!strcmp(pch,"Potato")){
				continue;
			}
			else{
				printf("%s,",pch);
				pch = strtok(NULL,":,");
			}
		}
		close(sock);
		break;
	}
	return 0;
}*/

int main(int argc, char *argv[]){
	//argv[0]=.out file, argv[1]=port-number, argv[2]=number-of-players, argv[3]=hops
	//variables
	int sock, portnum, rc, number_of_players, hops, rcv;
	char potato[]="Potato;";
	char msg_to_ngb[1024];
	char temp[1024];
	struct hostent *hp, *ihp;
	struct sockaddr_in server,sock_to_client;	
	char buff[1024];
	char buff2[1024];
	char buff3[1024];
	char host[1024], *token;
	fd_set rdfs;
	struct timeval tv;
	struct in_addr **addr_list;
	int retval;

	//initial check
	//if(argc < 2){
	//	printf(stderr,"Insufficient arguments!\n");
	//	exit(1);
	//}
	gethostname(host,sizeof(host));
	hp = gethostbyname(host);
  		if ( hp == NULL ) {
			fprintf(stderr, "%s: host not found (%s)\n", argv[0], host);
			exit(1);
		}
	portnum = atoi(argv[1]);
	
	printf("Potato Master on %s\n",host);
	printf("Players = %s\n",argv[2]);
	printf("Hops = %s\n",argv[3]);	
	//printf("Reached here!");
	//create a socket
	sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock < 0){
		perror("Failed to create socket!\n");
		exit(1);
	}
	//bzero((char *) &server, sizeof(server));
	
	int sockopt = 1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEPORT,&sockopt,sizeof(int));

	server.sin_family = AF_INET;
	//printf("Reached here!");
	server.sin_port = htons(portnum);
	memcpy(&server.sin_addr, hp->h_addr_list[0], hp->h_length);
	//printf("Reached here!");
	//bind
	if(bind(sock, (struct sockaddr *) &server, sizeof(server)) < 0){
		perror("Failed to bind socket!\n");
		exit(1);
	}
	//printf("Reached here!");	
	//listen
	number_of_players = atoi(argv[2]);
	hops = atoi(argv[3]);
	if(number_of_players < 1){
		printf("Number of players must be greater than one.\n");
		exit(0);
	}
	rc = listen(sock,number_of_players);
	if ( rc < 0 ) {
    		perror("Failed to listen!\n");
    		exit(rc);
  	}
	player *player_list;
	player_list = (player *)malloc(sizeof (player) * number_of_players);
	//printf("Reached here!");

	//accept
	int player_count, rcv_sock[number_of_players], *left_ngb, *right_ngb;
	//pthread_t threads[number_of_players];				//create a listening thread for each player connecting

	left_ngb = malloc(number_of_players * sizeof (int));
	right_ngb = malloc(number_of_players * sizeof (int));
	int send_sock,len;
	struct sockaddr_in sock_n;

	//printf("Reached here!");

	for(player_count=0; player_count < number_of_players; player_count++){
		len=sizeof(server);
		rcv_sock[player_count] = accept(sock, (struct sockaddr *) &sock_to_client, &len);
		if(rcv_sock < 0){
			perror("Failed to accept client connection!\n");
			exit(1);
		}
		ihp = gethostbyaddr((char *) &sock_to_client.sin_addr, sizeof(struct in_addr), AF_INET);
		printf("player %d is on %s\n",player_count,ihp->h_name);

		memset(buff,0,sizeof(buff));
		rcv=recv(rcv_sock[player_count],buff,1024,0);
		
		if(rcv > -1){
			player_list[player_count].id=player_count;
			player_list[player_count].listen = atoi(buff);
            		addr_list = (struct in_addr **) ihp->h_addr_list;
            		char *ipstr = malloc(64);
            		memset(ipstr, 0, 64);
            		ipstr = inet_ntoa(*addr_list[0]);
			memset(player_list[player_count].ip,0,1024);
            		strcpy(player_list[player_count].ip, ihp->h_name);
			//close(rcv_sock[player_count]);
		}
	}
	for(player_count=0; player_count < number_of_players; player_count++){
		//setting up neighbors
        	if (player_count == 0) left_ngb[player_count] = player_list[number_of_players-1].id;
        	else left_ngb[player_count] = player_list[player_count-1].id;
        	if (player_count == number_of_players - 1) right_ngb[player_count] = player_list[0].id;
        	else right_ngb[player_count] = player_list[player_count+1].id;
		
    		//send neighbor information including left and right neigbor IDs and their IPs
		
		
		/*send_sock = socket(AF_INET, SOCK_STREAM, 0);
		if(send_sock<0){
			perror("Cannot connect to the neighbor..\n");
			exit(send_sock);
		}*/

		memset(msg_to_ngb,0,1024);
		memset(temp,0,1024);

		//initialise neighbor string and add player's id
		strcpy(msg_to_ngb,"NEIGHBORS;");
		sprintf(temp, "%d", player_list[player_count].id);
        	strcat(msg_to_ngb, temp);
       		strcat(msg_to_ngb, ";");

		//add left neighbor's id
		memset(temp, 0, 1024);
       		sprintf(temp, "%d", player_list[left_ngb[player_count]].id);
        	strcat(msg_to_ngb, temp);
        	strcat(msg_to_ngb, ";");

		//add left neighbor's ip address
        	strcat(msg_to_ngb, player_list[left_ngb[player_count]].ip);
        	strcat(msg_to_ngb, ";");
       		
		//add left neighbor's listening port
		memset(temp, 0, 1024);
        	sprintf(temp, "%d", player_list[left_ngb[player_count]].listen);
        	strcat(msg_to_ngb, temp);
        	strcat(msg_to_ngb, ";");

		//add right neighbor's id
        	memset(temp, 0, 1024);
        	sprintf(temp, "%d", player_list[right_ngb[player_count]].id);
        	strcat(msg_to_ngb, temp);
        	strcat(msg_to_ngb, ";");

		//add right neighbor's ip address
        	strcat(msg_to_ngb, player_list[right_ngb[player_count]].ip);
        	strcat(msg_to_ngb, ";");

		//add right neighbor's listening port
        	memset(temp, 0, 1024);
        	sprintf(temp, "%d", player_list[right_ngb[player_count]].listen);
        	strcat(msg_to_ngb, temp);

		/*sock_n.sin_family = AF_INET;
        	sock_n.sin_port = htons(player_list[player_count].listen);
        	inet_pton(AF_INET, player_list[player_count].ip, &sock_n.sin_addr);
		
		rc = connect(rcv_sock[player_count], (struct sockaddr *) &sock_n, sizeof(sock_n));
		if (rc == -1){
            		perror("Unable to connect");
            		exit(-1);
        	}*/
		//printf("This is information to ngb %s\n",msg_to_ngb);
		send(rcv_sock[player_count],msg_to_ngb,strlen(msg_to_ngb),0);
		
		//close(send_sock);

		//tc = pthread_create(&threads[player_count], NULL, RecvSock, (void *)rcv_sock);
      		//	if (tc){
         	//		printf("ERROR; return code from pthread_create() is %d\n", tc);
         	//		exit(-1);
      		//	}
		bzero(buff,1024);
	}
	
	for(player_count=0; player_count < number_of_players; player_count++){
		memset(buff3,0,sizeof(buff3));
		recv(rcv_sock[player_count], buff3, sizeof(buff3), 0);	
		if(!strcmp(buff3,"1"));
	}

	if(hops == 0){
		for(player_count=0; player_count < number_of_players; player_count++){
			send(rcv_sock[player_count],"Shutdown",strlen("Shutdown"),0);
			sleep(1);
			close(rcv_sock[player_count]);
			//sleep(1);
		}
		//printf("Trace of potato:\n");
		//printf("No trace!");
		exit(0);
	}

	else {
		int rnd_player;
		rnd_player = rand() % number_of_players;
		memset(buff2,0,sizeof(buff2));
		printf("All players present, sending potato to player %d\n",rnd_player);
		strcpy(buff2, potato);
    		sprintf(temp, "%d", hops);
    		strcat(buff2, temp);
		//printf("This is potato string before sending %s\n",buff2);
		int sendlen = send(rcv_sock[rnd_player],buff2,strlen(buff2),0);
		if ( sendlen != strlen(buff2) ) {
      			perror("send");
      			exit(1);
    		}
		//printf("This is potato string after sending %s\n",buff2);
		
		while(1){
			FD_ZERO(&rdfs);
			int n=0;
			for(player_count=0; player_count<number_of_players; player_count++){\
			if(rcv_sock[player_count] > 0){
				FD_SET(rcv_sock[player_count], &rdfs);
			}
			if(rcv_sock[player_count] > n){
				n = rcv_sock[player_count];
			}
		}
	
		n++;

		tv.tv_sec = 100;
		tv.tv_usec = 0;
	
	
		retval = select(n, &rdfs, NULL, NULL, &tv);

		if(retval<0){
			perror("Error in select()\n");
		}
		else if(retval == 0){
			perror("Timeout\n");
		}
		else{
			for(player_count=0; player_count<number_of_players; player_count++){
				if(FD_ISSET(rcv_sock[player_count], &rdfs)){
					memset(buff3,0,sizeof(buff3));
					rcv = recv(rcv_sock[player_count], buff3, sizeof(buff3), 0);
					break;
				}
			}
			break;
		}
		}
	
		for(player_count=0; player_count<number_of_players; player_count++){
			send(rcv_sock[player_count],"Shutdown",strlen("Shutdown"),0);
		}
	}
	printf("Trace of potato:\n");

    	token = strtok(buff3, ";");
	token = strtok(NULL, ";");
    	printf("%s\n", token);

	//for(player_count=0; player_count < number_of_players; player_count++){
	//	pthread_join(threads[player_count], NULL);
	//}

	for(player_count=0; player_count<number_of_players; player_count++){
    		close(rcv_sock[player_count]);
	}
	return 0;
}
