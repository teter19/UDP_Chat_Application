#include        <sys/types.h>   /* basic system data types */
#include        <sys/socket.h>  /* basic socket definitions */
#include        <netinet/in.h>  /* sockaddr_in{} and other Internet defns */
#include        <arpa/inet.h>   /* inet(3) functions */
#include        <errno.h>
#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include        <unistd.h>

#define MAXLINE 1024
#define SA      struct sockaddr

int chat_function(int sockfd, int is_first){
	char buf[100];
	int k;
	for ( ; ; ){
        if(is_first == 1){
			printf("Me: ");
			fgets(buf,100,stdin);
			k=send(sockfd,buf,100,0);
			
			if(strncmp(buf,"end",3)==0){
				printf(">------------------- YOU CLOSED CONNECTION -------------------< \n\n");
				break;
			}
			
			if(k==-1){
				printf("Error in sending");
				exit(1);
			}

			k=recv(sockfd,buf,100,0);
			
			if(strncmp(buf,"end",3)==0){
				printf(">------------------- CONNECTION CLOSED ------------------< \n\n");
				break;
			}
			
			if(k==-1){
				printf("Error in receiving");
				exit(1);
			}

			printf("Peer: %s",buf);
		}
		if(is_first == 0){
			k=recv(sockfd,buf,100,0);
			
			if(strncmp(buf,"end",3)==0){
				printf(">------------------- CONNECTION CLOSED ------------------< \n\n");
				break;
			}
			
			if(k==-1){
				printf("Error in receiving");
				exit(1);
			}

			printf("Peer: %s",buf);
			
			printf("Me: ");
			fgets(buf,100,stdin);
			k=send(sockfd,buf,100,0);
			
			if(strncmp(buf,"end",3)==0){
				printf(">------------------- YOU CLOSED CONNECTION ------------------< \n\n");
				break;
			}
			
			if(k==-1){
				printf("Error in sending");
				exit(1);
			}
		}
	}
}

int udp_connection(char **argv){
    char buf[100]; 
    int sockfd, k; 
	int is_first = 0;
	long port =  strtol(argv[2], NULL, 10); ///zamiana char na int
    struct sockaddr_in servaddr; 
      
    //wyczysc servaddr
    bzero(&servaddr, sizeof(servaddr)); 
    servaddr.sin_addr.s_addr = inet_addr(argv[1]); 
    servaddr.sin_port = htons(port); 
    servaddr.sin_family = AF_INET; 
      
    //tworzenie gniazda UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
      
    //polaczenie z serverem 
    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) 
    { 
        printf("\n Error : Connection Failed \n"); 
        exit(0); 
    }

    //wyslij odpowiedz 
	k=send(sockfd,"",100,0); //prosba o polaczenie z serwerem
	
	while(1){
		k = recv(sockfd,buf,100,0);
		if(k==-1){
            printf("Error in receiving");
            exit(1);
        }
		if(strtol(buf, NULL, 10) == 0){
			is_first = 1;
			printf("Waiting for second Client... \n");
		}
		else{
			printf("Connection with Second Client is established! \n\n");
			break;
		}
	}
	
	chat_function(sockfd, is_first);
	close(sockfd);
 
	
}
int main(int argc, char **argv)
{
	if (argc !=3){
			fprintf(stderr, "Not enough arguments: no ip address or no port ");
			return 1;
	}
	else{
		printf(">------------------- CLIENT UPD START ------------------< \n\n");
		udp_connection(argv);
	}
	
	
	exit(0);
}
