#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<string.h>

int main(int args, char *argv[])
{
    struct sockaddr_in server,client;
    int port,serversock,n,fp,end;
    char addr[16],filename[20],buffer[100];
	
	strcpy(addr, argv[1]);
	port = atoi(argv[2]);

    serversock=socket(AF_INET,SOCK_DGRAM,0);
    server.sin_family=AF_INET;
    server.sin_port=port;
    server.sin_addr.s_addr=inet_addr(addr);
    bind(serversock,(struct sockaddr *)&server,sizeof(server));
	printf("Connection established\n");
    n=sizeof(client);
    recvfrom(serversock,filename,sizeof(filename), 0,(struct sockaddr *)&client,&n);
    fp=open(filename,O_RDONLY);
    while(1)
    {
        end=read(fp,buffer,sizeof(buffer));
        if(end==0)
            break;
        sendto(serversock,buffer,sizeof(buffer),0,(struct sockaddr *)&client,n);
        bzero(buffer,100);
    }
    strcpy(buffer,"EOF");
    sendto(serversock,buffer,sizeof(buffer),0,(struct sockaddr *)&client,n);
	printf("File sent succesfully\n");
return 0;
}
