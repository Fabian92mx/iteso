/**
*
**/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>

#define BUFFERSIZE 256

int main(int args, char *argv[]) {

u_int port;
int server;
int client;
int localerror;
struct sockaddr_in server_addr;
socklen_t clienteLen;	
int status;
char *buffer;
char *sizeOfFile;
char *nombre;
int size = 0;
int tamano = 0;
int fd;

    //Validamos los Arguemntos
if(args < 3) {
fprintf(stderr,"Error: Missing Arguments\n");
fprintf(stderr,"\tUSE: %s [ADDR] [PORT]\n",argv[0]);
return 1;
}

//Iniciamos la apertura del Socket
server = socket(PF_INET,SOCK_STREAM,0);
if(server == -1) {
localerror = errno;
fprintf(stderr, "Error: %s\n",strerror(localerror));
return 1;
}

port = atoi(argv[2]);

bzero(&server_addr,sizeof(server_addr));
server_addr.sin_family = AF_INET;	
status = inet_pton(AF_INET,argv[1],&server_addr.sin_addr.s_addr);
server_addr.sin_port = htons(port);

status = connect(server,(struct sockaddr *)&server_addr,sizeof(server_addr));

if(status != 0) {
localerror = errno;
printf("Error al conectarnos (%s)\n",strerror(localerror));
return 1;
}

printf("Conectado\n");
/*
//obtener el tamaño del nombre del archivo
nombre = "algo.txt\0";
size = strlen(nombre);
printf("El tamaño del nombre es: %i \n",size);
//enviar tamaño

status=write(server,size,3);
*/

int readbytes = 0;
int writebytes = 0;
buffer = (char *) calloc(1,BUFFERSIZE);

		
 if (fd = open("archivoRecivido.txt", O_WRONLY | O_CREAT )==-1)
	{
	printf("Error al abrir el archivo");
	}

while(readBytes = read(server, buffer, BUFFERSIZE) > 0)
	{	
		writeBytes = 0;
		while(writeBytes < readBytes)
		{
			writeBytes = write(file, buffer + writeBytes, readBytes - writeBytes);
		}
		printf("Se leyeron %i bytes de %i del servidor\n", writeBytes, readBytes);	
	}
	
toWrite = (char *) calloc(1,tamano);
	//Escribir lo recibido:
status = read(server,toWrite,tamano);
printf("Se escribió:\n %s\n",toWrite);
close(fd);
//free(sizeOfFile);
close(server);
}
