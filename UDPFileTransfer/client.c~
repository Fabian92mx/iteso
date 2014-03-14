/*Client.c*/


#include <stdio.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <stdlib.h>

#include <sys/time.h>
long currentTimeMillis() ;

int main(int argc, char* argv[]) {

int udpSocket;
struct sockaddr_in udpServer, udpClient;
socklen_t addrlen = sizeof(udpClient),len;
int status;
char buffer[255] = "";
int i;
int option;
int fd;
int contador=0;
long start, end;
char ip[17];
u_short clientPort;
char *tok;
int fileSize;

int totalReadBytes, readBytes;
char fileBuffer[10240];
int writeBytes;

//Creamos el Socket
udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
if(udpSocket == -1) {
fprintf(stderr,"Can't create UDP Socket");
return 1;
}

udpServer.sin_family = AF_INET;
inet_pton(AF_INET,"127.0.0.1",&udpServer.sin_addr.s_addr);
udpServer.sin_port = htons(5000);

status = sendto(udpSocket,argv[1],strlen(argv[1]),0,(struct sockaddr*)&udpServer, sizeof(udpServer));
bzero(buffer,255);
status = recvfrom(udpSocket, buffer, 255, 0, (struct sockaddr*)&udpClient, &addrlen );

inet_ntop(AF_INET,&(udpClient.sin_addr),ip,INET_ADDRSTRLEN);
clientPort = ntohs(udpClient.sin_port);

printf("Recibimos: [%s:%i] %s",ip,clientPort,buffer);

tok = strtok(buffer," ");
if(strcmp(tok,"READY")!=0) {
fprintf(stderr,"El server nos mando algo raro...\n");
return -1;
}

tok = strtok(NULL," ");
fileSize = atoi(tok);

fd = open(argv[2],O_WRONLY | O_CREAT | O_TRUNC);
if(fd == -1) {
fprintf(stderr, "Cant creat the file");
return -1;
}

status = sendto(udpSocket,"OK",strlen("OK"),0,(struct sockaddr*)&udpServer, sizeof(udpServer));

totalReadBytes = 0;
while(totalReadBytes < fileSize) {
readBytes = recvfrom(udpSocket,fileBuffer,10240,0,(struct sockaddr*)&udpClient, &addrlen );
while(writeBytes < readBytes) {
writeBytes += write(fd,fileBuffer+writeBytes,readBytes-writeBytes);
}
totalReadBytes += writeBytes;
}

close(fd);

bzero(buffer,255);
status = recvfrom(udpSocket, buffer, 255, 0, (struct sockaddr*)&udpClient, &addrlen );

printf("Recibimos: [%s:%i] %s",ip,clientPort,buffer);

close(udpSocket);

return 0;
}

long currentTimeMillis() {
        long t;
        struct timeval tv;
        gettimeofday(&tv, 0);
        t = (tv.tv_sec*1000)+tv.tv_usec;
        return t;
}
