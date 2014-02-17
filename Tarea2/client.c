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

int main(int args, char *argv[]) {

u_int port;
int server;
int client;
int localerror;
struct sockaddr_in server_addr;
socklen_t clienteLen;	
int status;
char *cadena;

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
cadena = (char *) calloc(1,5);
status = read(server, cadena, 5);
		int tamano = 0;
		tamano = cadena;
		int fd = open("archivoRecivido.txt", O_CREAT, S_IWUSR);
		printf("El tamaño del archivo a escribir es de ; %i \n", tamano);
toWrite = (char *) calloc(1,tamano);
	//Escribir lo recibido:
	
status = read(server,toWrite,tamano);
printf("Se escribió:\n %s\n",toWrite);

free(cadena);
close(server);
}
