#include <sys/types.h>
#include <strings.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <resolv.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "tcp.h"
#include "debug.h"
#include "defaults.h"
#define OUTNAME "test.zip"
#define FILEPATH "/home/fabian/HTTPS/Practica1/imagen.bmp"
#define NOTFOUND "/home/fabian/HTTPS/Practica1/notFound.html"

void doGet(SSL* clientSocket, const char *fileName);
int sendLine(SSL* clientSocket, const char* writeBuffer);
SSL_CTX* InitServerCTX(void);
void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile);
void ShowCerts(SSL* ssl);

void start_protocol(SSL* clientSocket) 
{

	char readBuffer[BUFFERSIZE];
	int readBytes;
	char *cmdString;
	char *ptr;
	char *token;
	char *accessRoute;

	
		cmdString = (char *) calloc(255,1);
		while((readBytes = SSL_read(clientSocket,readBuffer,MAXBYTESREAD))>0)
		{
				cmdString = (char *) realloc(cmdString,strlen(cmdString)+readBytes+1);
				strncat(cmdString,readBuffer,readBytes);
				//Read the last 4 characters to determine the EOF
				ptr = cmdString+(strlen(cmdString)-4);
				if(strcmp(ptr,"\r\n\r\n")==0) 
				{	
					*ptr = '\0';
					break;
				}
		}	
			
			debug(4,"<- %s",cmdString);

			token = strtok(cmdString," \t\b");
			debug(4,"%s",token);
			if(strcmp(token,"GET")==0)
			{
				token = strtok(NULL," \t\b");
				if(token == NULL) 
				{
					sendLine(clientSocket, "ERROR ( Acces Route )\r\n\r\n");
				}
				else
				{
					accessRoute = token;
					token = strtok(NULL," \t\b"); 
					if(strcmp(token,"HTTP"))
					{
						doGet(clientSocket,accessRoute);
					}
					else
					{
						sendLine(clientSocket, "Missing Protocol\r\n");
					}
				}
			}
			else
			{
				sendLine(clientSocket, "Unkown command\r\n\r\n");
			}	
			
			if(cmdString != NULL)
				free(cmdString);
	
	
}

int start_server(const char iface[], const u_short port, const u_short maxClients) 
{
	int serverSocket;
	int clientSocket;
	char clientIP[18];
	u_int clientPort;
    	int forkID;
	int localerror;
	SSL_CTX *ctx;
	SSL *ssl;
	int sd;
	
	SSL_library_init();
	ctx = InitServerCTX();
	LoadCertificates(ctx, "cert.pem", "key.pem"); 
	serverSocket = newTCPServerSocket4(iface,port,maxClients);

	if(serverSocket == -1) 
    {
		return 0;
	}
	
    while(1)
    {
        bzero(clientIP,sizeof(clientIP));
        clientPort = 0;

        debug(1,"%s","Waiting for a Client...");
        clientSocket = waitConnection4(serverSocket,clientIP,&clientPort); 
        if(clientSocket == -1)
        {                        
            debug(1,"%s","ERROR: Invalid Client Socket");
            continue;    
        }

        debug(2,"Connected Client %s:%u",clientIP,clientPort);

        forkID = fork();
        if(forkID == 0)
        {
	    ssl = SSL_new(ctx); 
	    SSL_set_fd(ssl, clientSocket);
	    SSL_accept(ssl);
	    ShowCerts(ssl);
            start_protocol(clientSocket);
	    sd = SSL_get_fd(ssl); /* get socket connection */
    	    SSL_free(ssl); /* release SSL state */
    	    close(sd); /* close connection */
	    SSL_CTX_free(ctx);
            debug(2,"Close connection (%s:%u)",clientIP,clientPort);
        }
        else if(forkID > 0)
        {
            closeTCPSocket(clientSocket);
        }
        else
        {
            localerror = errno;
            debug(0,"ERROR, Cant Fork for Client %s",strerror(localerror));
            return 1;
        }

    }

	closeTCPSocket(serverSocket);	
	return 1;
	
}

void doGet(SSL* clientSocket, const char  baseDir[])
{
	char *writeBuffer = (char *) malloc(256);
	u_int writeBytes = 0;
	char *readBuffer;
	u_int readBytes = 0;
	int fd;
	int localError;
	struct stat fs;
	debug(1,"%s",baseDir);
	
	
	if(strcmp(baseDir,"/download")==0)
	{
		fd = open(FILEPATH,O_RDONLY);
		debug(1,"%s","Si se encontro la palabra download");
		sendLine(clientSocket, "HTTP/1.1 200 OK\r\n");	
		sprintf(writeBuffer,"Content-Disposition: attachment; filename=%s\r\n",OUTNAME);
		sendLine(clientSocket,writeBuffer);
		writeBuffer = (char *) malloc(256);
		fstat(fd, &fs);
		sprintf(writeBuffer,"Content-Length: %u\r\n",(u_int)fs.st_size);
		sendLine(clientSocket,writeBuffer);
	}
	
	else
	{
		fd = open(NOTFOUND,O_RDONLY);
		debug(1,"%s","Abriendo error 404");
		sendLine(clientSocket, "HTTP/1.1 240 No Content\r\n");	
		sendLine(clientSocket,"Content-Type: text/html\r\n");
	}
	
	if(fd == -1)
	{	
		localError = errno;
		debug(1,"Can't open Requested File (%s)",strerror(localError));
		sprintf(writeBuffer,"NOT_FOUND file\r\n\r\n");
		sendLine(clientSocket, writeBuffer);
		free(writeBuffer);
		return;
	}
	
	sendLine(clientSocket,"\r\n");
	readBuffer = (char *) malloc(BUFFERSIZE);
	
	while((readBytes = read(fd,readBuffer,BUFFERSIZE))>0) 
	{
		writeBytes = 0;
		while(writeBytes < readBytes) 
		{
			writeBytes += SSL_write(clientSocket,readBuffer+writeBytes,readBytes-writeBytes);
		}
	}
	
	free(readBuffer);
	free(writeBuffer);
	close(fd);
	
}

int sendLine(SSL* clientSocket, const char* writeBuffer)
{
	u_int writeBytes = 0;
	
	writeBytes += SSL_write(clientSocket,writeBuffer+writeBytes,strlen(writeBuffer)-writeBytes);
	
	while(writeBytes < strlen(writeBuffer))
	{
		writeBytes += SSL_write(clientSocket,writeBuffer+writeBytes,strlen(writeBuffer)-writeBytes);
	}
	debug(4,"-> %s",writeBuffer);	
	
	return writeBytes;
}

SSL_CTX* InitServerCTX(void)
{ 
	SSL_METHOD *method;
    SSL_CTX *ctx;

    OpenSSL_add_all_algorithms(); /* load & register all cryptos, etc. */
    SSL_load_error_strings(); /* load all error messages */
    method = SSLv23_server_method(); /* create new server-method instance */
    ctx = SSL_CTX_new(method); /* create new context from method */
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}

void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile)
{
 	/* set the local certificate from CertFile */
    if ( SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    /* set the private key from KeyFile (may be the same as CertFile) */
    if ( SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    /* verify private key */
    if ( !SSL_CTX_check_private_key(ctx) )
    {
        fprintf(stderr, "Private key does not match the public certificate\n");
        abort();
    }
}

void ShowCerts(SSL* ssl)
{ 
	X509 *cert;
    char *line;

    cert = SSL_get_peer_certificate(ssl); /* Get certificates (if available) */
    if ( cert != NULL )
    {
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);
        X509_free(cert);
    }
    else
        printf("No certificates.\n");
}
