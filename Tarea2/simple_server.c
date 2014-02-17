/**
*
**/

#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

int main(int args, char *argv[]) {

u_int port;
int server;
int client;
int localerror;
struct sockaddr_in myAddr;
struct sockaddr_in cliente_addr;
socklen_t clienteLen;	
int status;
char *filePath;
int file;
char byte;
int fileSize;
struct stat buf;
int fileLength;

    //Validamos los Arguemntos
if(args < 3) {
fprintf(stderr,"Error: Missing Arguments\n");
fprintf(stderr,"\tUSE: %s [PORT]\n",argv[0]);
fprintf(stderr,"\tSpecify file to transfer\n");
return 1;
}

port = atoi(argv[1]);
if(port < 1 || port > 65535) {
fprintf(stderr,"Port %i out of range (1-65535)\n",port);
return 1;
}

strcpy(filePath, argv[2]);
int fd = open("filePath", O_RDONLY);

if(fd == -1)
{
	fprintf(stderr, "Couldn't open file: %s\n",filePath);
	return 1;
}

//get the file size
fstat(fd, &buf);
fileSize = buf.st_size;

//Iniciamos la apertura del Socket
server = socket(PF_INET,SOCK_STREAM,0);
if(server == -1) {
localerror = errno;
fprintf(stderr, "Error: %s\n",strerror(localerror));
return 1;
}

//Nos adjudicamos el Puerto.
bzero(&myAddr,sizeof(myAddr));
myAddr.sin_family = AF_INET;
myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
myAddr.sin_port = htons(port);

status = bind(server, (struct sockaddr *)&myAddr, sizeof(myAddr));
if(status != 0) {
localerror = errno;
fprintf(stderr,"Can't Bind Port: %s\n",strerror(localerror));
close(server);
return 1;
}

//Nos ponemos el modo de Escucha
status = listen(server,5);
if(status == -1) {
localerror = errno;
fprintf(stderr,"Can't listen on socket(%s)\n",strerror(localerror));
close(server);
return 1;
}	

//Esperamos una Conexión
while(1) {
bzero(&cliente_addr,sizeof(cliente_addr));
client = accept(server,(struct sockaddr *)&cliente_addr,&clienteLen);
if(client == -1) {
localerror = errno;
fprintf(stderr,"Error acepting conection %s\n",strerror(localerror));
close(server);
return 1;
}

//Inicia el protocolo...
	
	//enviar la longitud del archivo.
	status=write(client,fileLength,4);

close(client);
}

//close(server);

return 0;
}
