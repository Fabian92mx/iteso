#include<stdio.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<stdlib.h>

int main(int args, char *argv[])
{

    struct sockaddr_in server,client;
    int s,n,ret;size_t fp;
	int port;
    char filename[20],copy[20],data[100],c[25], addr[16];
	
	if(args < 5) {
	fprintf(stderr,"Error: Missing Arguments\n");
	fprintf(stderr,"\tUSE: Address Port FileName SaveName\n");

	return 1;
	}
	strcpy(addr, argv[1]);

	port = atoi(argv[2]);
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    s=socket(AF_INET,SOCK_DGRAM,0);
    server.sin_family=AF_INET;
    server.sin_port= port;
    server.sin_addr.s_addr=inet_addr(addr);
    n=sizeof(server);
	strcpy(filename, argv[3]);    
	strcpy(copy, argv[4]);
    //printf("\nDescargando...\n");
    sendto(s,filename,sizeof(filename),0,(struct sockaddr *)&server,n);
    fp =open(copy, O_WRONLY | O_CREAT | O_TRUNC, mode);
    if(fp==-1)
    {
        printf("\nError file not found\n");
        exit(0);
    }
	printf("File found, downloading file\n");
    recvfrom(s,data,sizeof(filedata),0,NULL,NULL);
    while(1)
    {
        if(strcmp(data,"EOF")==0)
            break;
        ret=write(fp,data,strlen(filedata));
        bzero(data,100);
        recvfrom(s,data,sizeof(filedata),0,NULL,NULL);
    }
    printf("\nDownload finished\n");
return 0;
}
