#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

pthread_t thread_tcp_service;
int hdiscSocket;
int tcpSocket;
int status;
struct sockaddr_in UDP_Server, UDP_Client, TCP_Server;
socklen_t addrlen = sizeof(UDP_Server);
char buffer[255];
char clientip[17];
int clientPort;
int BCPermission;
int tcpPort;
char *charTcpPort;
char *name;

int getFileCount()
{
	DIR *dp;
	struct dirent *ep;     
	dp = opendir ("./");
	int i = 0;
	if (dp != NULL)
	{
		while (ep = readdir (dp))
		i++;
		(void) closedir (dp);
	}
	else
		perror ("Couldn't open the directory");
	
	printf("se cuenta con %d archivos\n",i);
	return i;
}

void *tcp_service(void *arg)
{
	printf("Iniciando conexion\n");
	struct sockaddr_in clientAddress;
	socklen_t clienteLen;
	int client;

	memcpy(arg, &clientAddress, sizeof(clientAddress));
	
	//hacer conexion TCP con cliente solicitante
	tcpSocket = socket(PF_INET,SOCK_STREAM,0);
	if(tcpSocket == -1)
	{
		fprintf(stderr, "Error: %s\n",strerror(errno));
		pthread_exit(NULL);
	}
	
	//Nos adjudicamos el Puerto.
	bzero(&TCP_Server,sizeof(TCP_Server));
	TCP_Server.sin_family = AF_INET;
	TCP_Server.sin_addr.s_addr = htonl(INADDR_ANY);
	TCP_Server.sin_port = htons(tcpPort);

	status = bind(tcpSocket, (struct sockaddr *)&TCP_Server, sizeof(TCP_Server));
	if(status != 0)
	{
		fprintf(stderr,"Can't Bind Port: %s\n",strerror(errno));
		close(tcpSocket);
		pthread_exit(NULL);
	}
	status = listen(tcpSocket,5);
	if(status == -1)
	{
		fprintf(stderr,"Can't listen on socket(%s)\n",strerror(errno));
		close(tcpSocket);
		pthread_exit(NULL);
	}
	
	bzero(&clientAddress,sizeof(clientAddress));
	client = accept(tcpSocket,(struct sockaddr *)&clientAddress,&clienteLen);
	if(client == -1)
	{
		fprintf(stderr,"Error acepting conection %s\n",strerror(errno));
		close(tcpSocket);
		pthread_exit(NULL);
	}
	
	printf("Conexion establecida\n");
	//esperar comando
	//atender comando
	//desconectar
}

void commandService(char *command)
{
	//separar command en comando y argumentos
	//identificar commando
	//hacer lo necesario
		//ping(): responde PONG
		//filelist(): responde numero de archivos. en un for envia todos los nombres de archivo
		//getfile(name): responde tamano de archivo, transmite archivo.
		//getFileSize(name): responde tamano de archivo
		//getFilePart(name, firstbyte, lastbyte): transfiere el archivo name desde firstbyte hasta lastbyte.
}

int main(int argc, char *argv[])
{

	if (argc < 3)
	{
		fprintf(stderr,"Usage: %s <Server Name> <TCP Port>\n", argv[0]);
		exit(1);
	}

	tcpPort = atoi(argv[2]);
	charTcpPort = argv[2];
	name = argv[1];

	BCPermission = 1;
	
	//Creamos el Socket
	hdiscSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(hdiscSocket == -1)
	{
		fprintf(stderr,"Can't create UDP Socket");
		return 1;
	}
	
	UDP_Server.sin_family = AF_INET;
	inet_pton(AF_INET,"0.0.0.0",&UDP_Server.sin_addr.s_addr);
	UDP_Server.sin_port = htons(5000);
	
	status = bind(hdiscSocket, (struct sockaddr*)&UDP_Server,sizeof(UDP_Server));

	if(status != 0)
	{
		fprintf(stderr,"Can't bind");
	}


	status = setsockopt(hdiscSocket, SOL_SOCKET, SO_BROADCAST, (void *) &BCPermission, sizeof(BCPermission));
	
	char myStats[255];
	myStats[0] = '\0';
	strcat(myStats, "Hi ");
	strcat(myStats, name);
	strcat(myStats, "\n\r");
	strcat(myStats, "Puerto: ");
	strcat(myStats, charTcpPort);
	strcat(myStats, " Archivos: ");
	char numArchivos[3];
	snprintf(numArchivos, 3, "%d", getFileCount());
	strcat(myStats, numArchivos);
	strcat(myStats, "\n\r\n\r");
	strcat(myStats, "\0");
	printf(myStats);

	//hacer por siempre
	while(1)
	{
		//espera un mensaje del cliente
		bzero(&buffer, 255);
		
		recvfrom(hdiscSocket, buffer, 255, 0, (struct sockaddr*)&UDP_Client, &addrlen);
		inet_ntop(AF_INET,&(UDP_Client.sin_addr),clientip,INET_ADDRSTRLEN);
		clientPort = ntohs(UDP_Client.sin_port);
		
		printf("Recibimos: [%s:%i] %s\n",clientip,clientPort,buffer);
		//si es de hostDiscovery
		if(buffer[0] == 'H')
		{
			//responder mi direccion TCP, puerto TCP, nombre y cantidad de archivos que tengo
			printf("Respondiendo mensaje de hostDiscovery\n");
			status = sendto(hdiscSocket , myStats ,strlen(myStats),0,(struct sockaddr*)&UDP_Client, sizeof(UDP_Client));

		}else if(buffer[0] == 'C')//si es de conexion TCP
		{
			sleep(10);
			//avisar que okas
			status = sendto(hdiscSocket , "Vengase mijo",strlen("Vengase mijo"),0,(struct sockaddr*)&UDP_Client, sizeof(UDP_Client));
			//crear hilo tcp_service
			pthread_create(&thread_tcp_service, NULL, tcp_service, (void *)&UDP_Client);
		}
	}
	
	return 0;
}
