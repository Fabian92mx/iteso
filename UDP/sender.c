#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int bcSock;
    struct sockaddr_in broadcastAddr;
struct sockaddr_in udpServer, udpClient;
socklen_t addrlen = sizeof(udpClient);
    char *broadcastIP;
    unsigned short broadcastPort;
    char *buffer;
    char *sendString;
    
    int broadcastPermission;
    unsigned int sendStringLen;

int status;
int i;

    if (argc < 4)
    {
        fprintf(stderr,"Usage: %s <IP Address> <Port> <Send String>\n", argv[0]);
        exit(1);
    }

    broadcastIP = argv[1]; /* First arg: broadcast IP address */
    broadcastPort = atoi(argv[2]); /* Second arg: broadcast port */
    sendString = argv[3]; /* Third arg: string to broadcast */

bcSock = socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
if(bcSock == -1) {
fprintf(stderr,"Can't create Socket");
return 1;
}
  
    broadcastPermission = 1;
    status = setsockopt(bcSock, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission, sizeof(broadcastPermission));
    if(status == -1) {
fprintf(stderr,"Can't set Brodcast Option");
return 1;
    }

    memset(&broadcastAddr, 0, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
inet_pton(AF_INET,broadcastIP,&broadcastAddr.sin_addr.s_addr);
    broadcastAddr.sin_port = htons(broadcastPort);

    sendStringLen = strlen(sendString);
buffer = (char*)calloc(255,sizeof(char));
status = sendto(bcSock,sendString,sendStringLen,0,(struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
printf("Send %i bytes to brodcast addr\n",status);
//printf("esperando respuesta\n");

status = recvfrom(bcSock, buffer, 255, 0, (struct sockaddr*)&udpClient, &addrlen );
fflush(stdout);
printf("Recibimos: %s\n",buffer);
sendString =("dos");
status = sendto(bcSock,sendString,sendStringLen,0,(struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
status = recvfrom(bcSock, buffer, 255, 0, (struct sockaddr*)&udpClient, &addrlen );

fflush(stdout);
printf("Recibimos: %s\n",buffer);


}
