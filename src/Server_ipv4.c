#include        <sys/types.h>   /* basic system data types */
#include        <sys/socket.h>  /* basic socket definitions */
#include        <sys/time.h>    /* timeval{} for select() */
#include        <time.h>                /* timespec{} for pselect() */
#include        <netinet/in.h>  /* sockaddr_in{} and other Internet defns */
#include        <arpa/inet.h>   /* inet(3) functions */
#include        <errno.h>
#include        <fcntl.h>               /* for nonblocking */
#include        <netdb.h>
#include        <signal.h>
#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include        <unistd.h>

#define MAXLINE 1024
#define SA      struct sockaddr
#define LISTENQ 4

int close_connection(int connection){
		close(connection);
	}


int chat_function_udp(int listenfd, struct sockaddr_in cliaddr_1, struct sockaddr_in cliaddr_2){
	int k;
	char buf[100];
	
	while(1) {

		k=recv(listenfd, buf, sizeof(buf), 0);
		if(k==-1){
			printf("Error in receiving from Client 1");
			}
		if(strncmp(buf,"end",3)==0){
			sendto(listenfd, buf, sizeof(buf), 0,  (SA *) &cliaddr_2, sizeof(cliaddr_2));
			break;
		}
		
		printf("Client 1: %s",buf);
		sendto(listenfd, buf, sizeof(buf), 0,  (SA *) &cliaddr_2, sizeof(cliaddr_2));
		if(k==-1){
			printf("Error in sending to Client 2");
		}
		
		k=recv(listenfd, buf, sizeof(buf), 0);
		if(k==-1){
			printf("Error in receiving from Client 2");
		}
		printf("Client 2: %s ", buf);
		
		if(strncmp(buf,"end",3)==0){
			sendto(listenfd, buf, sizeof(buf), 0,  (SA *) &cliaddr_1, sizeof(cliaddr_1));
			break;
		}
		sendto(listenfd, buf, sizeof(buf), 0,  (SA *) &cliaddr_1, sizeof(cliaddr_1));
		if(k==-1){
			printf("Error in sending to Client 1");
		}	
	}
}


int server_udp(char **argv){
    char buf[100]; 
    int listenfd, len, k; 
    struct sockaddr_in servaddr, cliaddr_1, cliaddr_2; 
	long port =  strtol(argv[2], NULL, 10); ///kowersja char na int
    bzero(&servaddr, sizeof(servaddr)); 
  
    //tworzymy UDP socket
    listenfd = socket(AF_INET, SOCK_DGRAM, 0);         
    servaddr.sin_addr.s_addr = inet_addr(argv[1]); 
    servaddr.sin_port = htons(port); 
    servaddr.sin_family = AF_INET;  
   
    //bind adress servera do deksryptora gniazda 
    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)); 
    
	while(1){
		
		//SETUP pierwszego klienta
		//otrzymanie datagramu
		len = sizeof(cliaddr_1);
		printf("Waiting for Client 1... \n");
		k = recvfrom(listenfd, buf, sizeof(buf), 0, (SA *)&cliaddr_1, &len);	   
		//wyslij powiadomienie ze server jest polaczony
		sendto(listenfd, "0", MAXLINE, 0,  (SA *)&cliaddr_1, sizeof(cliaddr_1));
		
		
		// SETUP drugiego clienta
		// otrzymanie datagramu
		len = sizeof(cliaddr_2); 
		printf("Waiting for Client 2... \n");
		k = recvfrom(listenfd, buf, sizeof(buf), 0, (SA *)&cliaddr_2,&len); 
		// wyslij powiadomienie ze server jest polaczony
		sendto(listenfd, "1", MAXLINE, 0, (SA *)&cliaddr_2, sizeof(cliaddr_2));
		
		
		// wyslij dane do pierwszego klienta ze drugi klient jest polaczony
		sendto(listenfd, "1", MAXLINE, 0,  (SA *)&cliaddr_1, sizeof(cliaddr_1));
		
		printf("Connection between Clients is established! \n\n");
		
		chat_function_udp(listenfd, cliaddr_1, cliaddr_2);
			
		printf(">------------------- RESET CONNECTION -------------------< \n\n");
	}
}

int main(int argc, char **argv){
	
	if (argc !=3){
			fprintf(stderr, "Correct arguments are [ip_address] [port_number]\n");
			return 1;
	}
	else{
		printf(">------------------- SERVER UDP START -------------------< \n\n");
		server_udp(argv);
	}
	exit(0);
}
