#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define msg	"Is there anybody out there"

typedef struct
{
	int id;
	char ip[17];
	int port;
	char name [30];
} serverList;
//char *getName;
int udpSocket;
struct sockaddr_in udpServer, udpClient;

socklen_t addrlen = sizeof(udpClient);
char buffer[255];
char ip[17];
u_short clientPort;
char *broadcastIP;
struct sockaddr_in broadcastAddr;
int broadcastPermission;
pthread_t sender, receiver;

int status;

/*
char *getName(void) {

    char hostname[50];
    gethostname(hostname, 50);

    puts(hostname);

    return hostname;
}
*/


void *sendMessages(void *arg)
{
	int i;
	//char name;
	char myStats[255];
	myStats[0] = '\0';
	strcat(myStats, "Hello from: Client\n\r\n\r");	
	//name = getName();
	//strcat(myStats, "Hello from: ");
	//strcat(myStats, name);
	//strcat(myStats, "\n\r\n\r");
	strcat(myStats, "\0");
	printf(myStats);
	for(i = 0;i<10;i++)
	{
		//printf("Enviando Mensaje\n");
		status = sendto(udpSocket ,myStats ,strlen(myStats),0,(struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
		sleep(1);
	}
	pthread_exit(NULL);
}

void *receiveMessages(void *arg)
{
	while(1)
	{
		//sleep(1);
		//printf("Esperando recibir mensaje\n");
		status = recvfrom(udpSocket, buffer, 255, 0, (struct sockaddr*)&udpClient, &addrlen );

		inet_ntop(AF_INET,&(udpClient.sin_addr),ip,INET_ADDRSTRLEN);
		clientPort = ntohs(udpClient.sin_port);

		
		printf("Servidor encontrado en:%s\n",ip);
		printf("%s\n",buffer);
		
	}
	pthread_exit(NULL);
}

int main(int argc, char* argv[])
{

if (argc < 2)
{
	fprintf(stderr,"Usage: %s <broadcastIP>\n", argv[0]);
	exit(1);
}

broadcastIP = argv[1];
//Creamos el Socket
udpSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
if(udpSocket == -1) {
fprintf(stderr,"Can't create UDP Socket");
return 1;
}

    udpServer.sin_family = AF_INET;
    inet_pton(AF_INET,"0.0.0.0",&udpServer.sin_addr.s_addr);
    udpServer.sin_port = htons(3000);

    status = bind(udpSocket, (struct sockaddr*)&udpServer,sizeof(udpServer));

    broadcastPermission = 1;
    status = setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission, sizeof(broadcastPermission));

    if(status != 0) {
fprintf(stderr,"Can't bind");
    }
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_addr.s_addr = inet_addr(broadcastIP);
    broadcastAddr.sin_port = htons(5000);

pthread_create(&sender, NULL, sendMessages, NULL);
pthread_create(&receiver, NULL, receiveMessages, NULL);

//
sleep(12);
printf("Selecciona a que te quieres conectar:\n");

while(1);
close(udpSocket);

return 0;

}
