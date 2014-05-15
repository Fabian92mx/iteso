//SSL-Server.c
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <resolv.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "openssl/ssl.h"
#include "openssl/err.h"

#define FAIL    -1

//functions
int parseEmail(char *src, int srcSize, char *dest, int destSize);
int OpenListener(int port);
SSL_CTX* InitServerCTX(void);
void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile);
void ShowCerts(SSL* ssl);
void Servlet(SSL* ssl);
int main(int count, char *strings[]);

//Global
char mail[30];
char email[] = "parres@iteso.mx";

int OpenListener(int port)
{   int sd;
    struct sockaddr_in addr;

    sd = socket(PF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if ( bind(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
    {
        perror("can't bind port");
        abort();
    }
    if ( listen(sd, 10) != 0 )
    {
        perror("Can't configure listening port");
        abort();
    }
    return sd;
}

SSL_CTX* InitServerCTX(void)
{   SSL_METHOD *method;
    SSL_CTX *ctx;

    OpenSSL_add_all_algorithms();  /* load & register all cryptos, etc. */
    SSL_load_error_strings();   /* load all error messages */
    method = SSLv23_server_method();  /* create new server-method instance */
    ctx = SSL_CTX_new(method);   /* create new context from method */
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
{   X509 *cert;
    char *line;

    cert = SSL_get_peer_certificate(ssl); /* Get certificates (if available) */
    if ( cert != NULL )
    {
        printf("Client certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
		parseEmail(line, strlen(line), mail, 30); //parsea el correo de la autoridad
		printf("\nParsed email: [%s]\n\n", mail);
     
		if(SSL_get_verify_result(ssl) == X509_V_OK) {
            printf("client verification with SSL_get_verify_result() succeeded.\n");                
		} else{
            printf("client verification with SSL_get_verify_result() fail.\n");      
		}
		
        free(line);
        X509_free(cert);
    }
    else
        printf("No certificates.\n");
}

void Servlet(SSL* ssl) /* Serve the connection -- threadable */
{   char buf[1024];
    char reply[1024];
	struct stat buff;
    int sd, bytes;
    const char* HTMLecho="<html><body><pre>%s</pre></body></html>\n\n";
	char* accept = "Autoridad confirmada\niniciando FT\n";

    if ( SSL_accept(ssl) == FAIL )     /* do SSL-protocol accept */
        ERR_print_errors_fp(stderr);
    else
    {
        ShowCerts(ssl);        /* get any certificates */
        bytes = SSL_read(ssl, buf, sizeof(buf)); /* get request */
        if ( bytes > 0 )
        {
            buf[bytes] = 0;
            printf("Client msg: \"%s\"\n", buf);
            sprintf(reply, HTMLecho, buf);   /* construct reply */
            SSL_write(ssl, reply, strlen(reply)); /* send reply */
			printf("Comparando:[%s]con[%s]\n",mail, email);			
			if(strcmp (mail,email) == 0)
			{
				printf("Enviando:\n%s\n",accept);
				SSL_write(ssl, accept, strlen(accept));


					//File Transfer


					int file = open("archivo.txt", O_RDONLY);
					

					//get the file size
					fstat(file, &buff);
					bzero(buf,1024);
					int fileSize = buff.st_size;
					printf("File Size: %i\n", fileSize);
					//envia filesize
					int writeBytes = 0;
					int length = 10;
					snprintf(buf, 100, "%i", fileSize);
					printf("Enviando %s al cliente\n", buf);
					while(writeBytes < length)
					{
						writeBytes = SSL_write(ssl, buf + writeBytes, length - writeBytes);
						printf("Se escribieron %i bytes de b%i al cliente\n", writeBytes, length);
					}
					//recibe confirmacion
										
					bzero(buf,1024);
					printf("Esperando confirmacion de cliente...\n");
					bytes = SSL_read(ssl, buf, 2);
					buf[bytes] = 0;
					printf("Ciente ha confirmado: %s\n", buf);
					//envia archivo				
					printf("Enviando archivo al cliente\n");
					int readBytes = 0;
					writeBytes = 0;
					bzero(buf,1024);
						while((readBytes = read(file, buf, 1024)) > 0)
						{
							printf("Se escribiran %i bytes al cliente\n", readBytes);
							writeBytes = 0;
							while(writeBytes < readBytes)
							{
								writeBytes = SSL_write(ssl, buf + writeBytes, readBytes - writeBytes);
								printf("Se escribieron %i bytes de %i al cliente\n", writeBytes, readBytes);
							}
						}
					printf("Archivo enviado al cliente\n");
					//End of FT
			}
			else
			{
				printf("\nNo se puede confirmar autoridad\n");
			}
        }
        else
            ERR_print_errors_fp(stderr);
    }
    sd = SSL_get_fd(ssl);       /* get socket connection */
    SSL_free(ssl);         /* release SSL state */
    close(sd);          /* close connection */
}


int parseEmail(char *src, int srcSize, char *dest, int destSize)
{
		if(src == NULL || dest == NULL)
			return -1;

		char compare[] = "emailAddress=";
		char *init = src;
		char *curCheck = compare;
		int initOffset = 0;
		int curCheckOffset = 0;
		int rSeeker = 0;
		int compareLen = strlen(compare);
		//printf("src = %s, compare = %s\n", init, curCheck);
		//printf("initOffset = %i, srcSize = %i\n", initOffset, srcSize);
		while(initOffset < srcSize)
		{
			//printf("comparing init+%i (%c) to curCheck+%i(%c)\n", initOffset, *(init+initOffset), curCheckOffset, *(curCheck+curCheckOffset));
			if(*(init+initOffset) == *(curCheck+curCheckOffset))
			{
				//printf(" they're equal. Incrementing curCheckOffset.\n");
				curCheckOffset++;

				//printf(" checking if the comparison finished completely\n");
				//printf(" if(%i == %i)\n", curCheckOffset, compareLen);
				if(curCheckOffset == compareLen)
				{
					//printf(" Match has completed. Searching for next \\r.\n");
					init = init+initOffset+1;
					//printf(" checking %c\n", *(init+rSeeker));
				while(initOffset+rSeeker < srcSize && (*(init+rSeeker) != '\r' || *(init+rSeeker) != '\n' || *(init+rSeeker) != ' ' || *(init+rSeeker) != '\0'))
				{
					rSeeker++;
					//sleep(1);
					//printf(" checking %c\n", *(init+rSeeker));
				}
				//printf(" email is %s\n", init);
				//printf(" rSeeker = %i\n", rSeeker);
				bzero(dest, destSize);
				memcpy(dest, init, rSeeker);
				return 1;
				}
			}else
			{
				//printf(" they're not equal. Resetting curCheckOffset\n");
				curCheckOffset = 0;
			}
		initOffset++;
	}

	return -1;
}


int main(int count, char *strings[])
{   SSL_CTX *ctx;
    int server;
    char *portnum;

    if ( count != 2 )
    {
        printf("Usage: %s <portnum>\n", strings[0]);
        exit(0);
    }

    SSL_library_init();

    portnum = strings[1];

    ctx = InitServerCTX();        /* initialize SSL */

    LoadCertificates(ctx, "rootCA2.pem", "rootCA2.key"); /* load certs */

	SSL_CTX_load_verify_locations(ctx, "rootCA2.pem", "."); 
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
	SSL_CTX_set_verify_depth(ctx,1);

    server = OpenListener(atoi(portnum));    /* create server socket */

    while (1)
    {   struct sockaddr_in addr;
        socklen_t len = sizeof(addr);
        SSL *ssl;

        int client = accept(server, (struct sockaddr*)&addr, &len);  /* accept connection as usual */
        printf("Connection: %s:%d\n",inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        ssl = SSL_new(ctx);              /* get new SSL state with context */
        SSL_set_fd(ssl, client);      /* set connection socket to SSL state */
        Servlet(ssl);         /* service connection */
	
    }
    close(server);          /* close server socket */
    SSL_CTX_free(ctx);         /* release context */
}

