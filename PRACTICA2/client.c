#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>

#define msg	"Is there anybody out there"
#define BUFFERSIZE 256

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




void *sendMessages(void *arg)
{
	int i;
	//char name;
	char myStats[255];
	myStats[0] = '\0';
	strcat(myStats, "Hello from: Fabian\n\r\n\r");	
	//name = getName();
	//strcat(myStats, "Hello from: ");
	//strcat(myStats, name);
	//strcat(myStats, "\n\r\n\r");
	strcat(myStats, "\0");
	printf(myStats);
	//for(i = 0;i<2;i++)
	//{
		//printf("Enviando Mensaje\n");
		status = sendto(udpSocket ,myStats ,strlen(myStats),0,(struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
		sleep(1);
	//}
	pthread_exit(NULL);
}

void *receiveMessages(void *arg)
{
	while(1)
	{
		//sleep(1);
		//printf("Esperando recibir mensaje\n");
		bzero(&buffer,255);
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
	int server;
	char comando[50];
	char file [20];
	char *filePath;
	int i = 0;
	char no [3];
	char filenme [200];
	char * buf;
	buf = (char*) calloc(1, 255);
	int arch = 0;
	int readBytes = 0;
	int writeBytes = 0;
	int totalFileSize =0;
	char archivo [100];
	int fd;
	int totalReadBytes = 0;
	struct sockaddr_in server_addr;
	socklen_t tcp_len = sizeof(server_addr);
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
	sleep(4);

		pthread_cancel(receiver);
		char ip[17],temp[10];
		int puerto;
		printf("Selecciona a que IP te quieres conectar: ");
		gets(ip);
		fflush(stdin);
		printf("Selecciona a que puerto: ");
		fgets(temp,10,stdin);
		puerto = atoi(temp);
		printf("ip: %s Num puerto: %d\n",ip,puerto);
		bzero(temp, 10);
	//connect

	struct sockaddr_in dest;

	bzero(&dest,sizeof(dest));
	dest.sin_family = AF_INET;	
	status = inet_pton(AF_INET,ip,&dest.sin_addr.s_addr);
	dest.sin_port = htons(5000);

	printf("enviando connect\n");
		status = sendto(udpSocket ,"Connect" ,strlen("Connect"),0,(struct sockaddr*)&dest, sizeof(dest));
	printf("enviado Connect\n");
	printf("esperando respuesta\n");
		bzero(&buffer,255);
	status = recvfrom(udpSocket, buffer, 255, 0, (struct sockaddr*)&udpClient, &addrlen );
	printf("\nnos enviaron: %s\n",buffer);
	printf("abriendo socket\n");

	server = socket(PF_INET,SOCK_STREAM,0);
	if(server == -1) {
		fprintf(stderr, "Error: %s\n",strerror(errno));
		return 1;
	}
	printf("socket abierto\n");
	bzero(&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;	
	status = inet_pton(AF_INET,ip,&server_addr.sin_addr.s_addr);
	server_addr.sin_port = htons(puerto);
	printf("socket configurado, durmiendo 3\n");
	sleep(3);
	printf("Listo para conectar\n");
	//while(1);
	status = connect(server,(struct sockaddr *)&server_addr,sizeof(server_addr));

	printf("Conectado\n");
	//comandos
	while(1)
	{
		bzero(comando, 50);
		printf("\nIngresa un comando:");
		gets(comando);
		strcat(comando, "\r\n");
		//printf("\nComando %s\n",comando);
		

	
		//Casos de respuesta según comando
		if(strcmp(comando,"PING\r\n")==0)
		{
			status = write(server, comando, strlen(comando));
			bzero(comando, 50);
			printf("Esperando respuesta\n");
			status = recvfrom(server, comando, 255, 0,(struct sockaddr *)&server_addr,&tcp_len);
			//status = read(server, comando, 18);	
			printf("%s\n",comando);
			bzero(comando,50);
		}
		else if(strcmp(comando,"FILELIST\r\n")==0)
		{
			status = write(server, comando, strlen(comando));

			bzero(no,3);
			bzero(filenme,200);
			status = recvfrom(server, no, 3, 0, (struct sockaddr *) &server_addr, &tcp_len);
			arch = atoi(no);
			printf("cantidad de archivos: %d\n", arch);
			sleep(1);
			status = recvfrom(server, filenme, 200, 0, (struct sockaddr *) &server_addr, &tcp_len);
			printf("%s\r\n",filenme);
			bzero(filenme,200);
		}
		else if(strcmp(comando,"GETFILE\r\n")==0)
		{
			printf("\nIngresa archivo a leer:");
			gets(file);
			printf("\nIngresa nombre archivo destino:");
			gets(archivo);
			strcat(comando, file);
			strcat(comando, "\r\n");
			printf("\nComando: %s\n",comando);
			status = write(server, comando, strlen(comando));
			//Esperar respuesta de tamaño:
			status = read(server,buf, 10);
			printf("El tamaño del archivo es: %s \n",buf);
			totalFileSize = atoi(buf);
				
			readBytes = 0;
			writeBytes = 0;
			//Abrir/crear archivo para recivir
			if ((fd = open(archivo, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP))==-1)
			{
			printf("Error al abrir el archivo\n");
			return 1;
			}
			//Leer el archivo
			totalReadBytes = 0;
			while(totalReadBytes < totalFileSize && (readBytes = read(server, buf, 255)) > 0)
	{	
		writeBytes = 0;
		while(writeBytes < readBytes)
		{
			writeBytes = write(fd, buf + writeBytes, readBytes - writeBytes);
		}
		printf("Se leyeron %i bytes de %i del servidor\n", readBytes, totalFileSize);
		totalReadBytes += readBytes;	
	}
			
		}
		else if(strcmp(comando,"GETFILEPART\r\n")==0)
		{
			
		}
		else if(strcmp(comando,"GETFILESIZE\r\n")==0)
		{
			printf("\nIngresa archivo a obtener tamaño:");
			gets(file);
			strcat(comando, file);
			strcat(comando, "\r\n");
			printf("\nComando: %s\n",comando);
			status = write(server, comando, strlen(comando));
			
			status = read(server,buf, 10);
printf("El tamaño del archivo es: %s \n",buf);
		}
		else
		{
			printf("Comando no valido.\n");		
		}
	sleep(1);
	}


	//terminar
	while(1);
	close(udpSocket);

	return 0;

}
