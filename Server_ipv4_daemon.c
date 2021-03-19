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
#include		<syslog.h>
#include		<unistd.h>

#define MAXLINE 1024
#define SA      struct sockaddr
#define LISTENQ 4
#define	MAXFD	64

int daemon_init(const char *pname, int facility, uid_t uid)
{
	int		i, p;
	pid_t	pid;

	if ( (pid = fork()) < 0)
		return (-1);
	else if (pid)
		exit(0);			//skasowanie rodzica

	//dziecko 1 kontynuuje

	if (setsid() < 0)			//stanie sie leaderem sesji
		return (-1);

	signal(SIGHUP, SIG_IGN);
	if ( (pid = fork()) < 0)
		return (-1);
	else if (pid)
		exit(0);			//kasowanie dziecka pierwszego

	//dziecko drugie kontynuuje

	chdir("/tmp");				//zmiana folderu
//	chroot("/tmp");

	//zamkniecie deskryptora plikow
	for (i = 0; i < MAXFD; i++){
		close(i);
	}

	//przekierowanie stdin, stdout i stderr do /dev/null
	p= open("/dev/null", O_RDONLY);
	open("/dev/null", O_RDWR);
	open("/dev/null", O_RDWR);

	openlog(pname, LOG_PID, facility);
	
	setuid(uid); //zmiana uzwytkownika
	syslog (LOG_NOTICE, "Daemon started!");
	return (0);				//sukces
}

int close_connection(int connection){
		close(connection);
	}

int chat_function_udp(int listenfd, struct sockaddr_in cliaddr_1, struct sockaddr_in cliaddr_2){
	int k;
	char buf[100];
	
	while(1) {

		k=recv(listenfd, buf, sizeof(buf), 0);
		if(k==-1){
			syslog (LOG_ERR, "Error in receiving from First Client");
			}
		if(strncmp(buf,"end",3)==0){
			sendto(listenfd, buf, sizeof(buf), 0,  (SA *) &cliaddr_2, sizeof(cliaddr_2));
			break;
		}
		
		syslog (LOG_INFO, "First Client: %s",buf);
		sendto(listenfd, buf, sizeof(buf), 0,  (SA *) &cliaddr_2, sizeof(cliaddr_2));
		if(k==-1){
			syslog (LOG_ERR, "Error in sending to Second Client");
		}
		
		k=recv(listenfd, buf, sizeof(buf), 0);
		if(k==-1){
			syslog (LOG_ERR, "Error in receiving from Second Client");
		}
		syslog (LOG_INFO, "Second Client: %s",buf);
		
		if(strncmp(buf,"end",3)==0){
			sendto(listenfd, buf, sizeof(buf), 0,  (SA *) &cliaddr_1, sizeof(cliaddr_1));
			break;
		}
		sendto(listenfd, buf, sizeof(buf), 0,  (SA *) &cliaddr_1, sizeof(cliaddr_1));
		if(k==-1){
			syslog (LOG_ERR, "Error in sending to First Client");
		}	
	}
}

int server_udp(char **argv){
    char buf[100]; 
    int listenfd, len, k; 
    struct sockaddr_in servaddr, cliaddr_1, cliaddr_2; 
	long port =  strtol(argv[2], NULL, 10); //zamiana char na int
    bzero(&servaddr, sizeof(servaddr)); 
  
    // utworznie gniazda UDP 
    listenfd = socket(AF_INET, SOCK_DGRAM, 0);         
    servaddr.sin_port = htons(port); 
    servaddr.sin_family = AF_INET;  
   	if( inet_pton(AF_INET, argv[1], &servaddr.sin_addr) != 1 ){
		syslog (LOG_ERR, "ERROR: Address format error");
		return -1;
	}
   
    // przywiazanie adresu serwera do deskryptora gniazda
	if ( bind( listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
			syslog(LOG_ERR,"bind error : %s", strerror(errno));
			return 1;
	}
    
	while(1){
		
		// SETUP pierwszego klienta
		// otrzymanie datagramu
		len = sizeof(cliaddr_1);
		syslog (LOG_NOTICE, "Waiting for First Client... ");
		k = recvfrom(listenfd, buf, sizeof(buf), 0, (SA *)&cliaddr_1, &len);	   
		// wyslanie powiadomienia ze polaczyl sie z serwerem
		sendto(listenfd, "0", MAXLINE, 0,  (SA *)&cliaddr_1, sizeof(cliaddr_1));
		
		
		// SETUP drugiego klienta
		// otrzymanie datagramu
		len = sizeof(cliaddr_2); 
		syslog (LOG_NOTICE, "Waiting for Second Client... ");
		k = recvfrom(listenfd, buf, sizeof(buf), 0, (SA *)&cliaddr_2,&len); 
		//  wyslanie powiadomienia ze polaczyl sie z serwerem
		sendto(listenfd, "1", MAXLINE, 0, (SA *)&cliaddr_2, sizeof(cliaddr_2));
		
		
		// wyslanie powiadomienia do pierwszego klienta ze drugi klient sie polaczyl
		sendto(listenfd, "1", MAXLINE, 0,  (SA *)&cliaddr_1, sizeof(cliaddr_1));
		
		syslog (LOG_NOTICE, "Connection between Clients is established!");
		
		chat_function_udp(listenfd, cliaddr_1, cliaddr_2);
			
		syslog (LOG_NOTICE, ">------------------- RESET CONNECTION -------------------< \n");
	}
}

int main(int argc, char **argv){
	
	if (argc !=3){
			fprintf(stderr, "Correct arguments are [ip_address] [port_number]\n");
			return 1;
	}
	else{
		setlogmask(LOG_UPTO(LOG_INFO));
		daemon_init(argv[0], LOG_LOCAL1, 1000);
		syslog(LOG_NOTICE, ">------------------- SERVER UDP START -------------------< \n");
		server_udp(argv);
	}
	exit(0);
}