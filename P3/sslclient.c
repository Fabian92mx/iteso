//SSL-Client.c
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define FAIL    -1

int OpenConnection(const char *hostname, int port);
int parseEmail(char *src, int srcSize, char *dest, int destSize);
void ShowCerts(SSL* ssl);
SSL_CTX* InitCTX(void);
int main(int count, char *strings[]);
void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile);


int OpenConnection(const char *hostname, int port)
{   int sd;
    struct hostent *host;
    struct sockaddr_in addr;

    if ( (host = gethostbyname(hostname)) == NULL )
    {
        perror(hostname);
        abort();
    }
    sd = socket(PF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = *(long*)(host->h_addr);
    if ( connect(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
    {
        close(sd);
        perror(hostname);
        abort();
    }
    return sd;
}

SSL_CTX* InitCTX(void)
{   
	SSL_METHOD *method;
    SSL_CTX *ctx;

    OpenSSL_add_all_algorithms();  /* Load cryptos, et.al. */
    SSL_load_error_strings();   /* Bring in and register error messages */
    method = SSLv23_client_method();  /* Create new client-method instance */
    ctx = SSL_CTX_new(method);   /* Create new context */
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }

    //Obligar a usar SSLv3 y TSLv1
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
    return ctx;
}

void ShowCerts(SSL* ssl)
{   X509 *cert;
    char *line;
    char mail[30];
    cert = SSL_get_peer_certificate(ssl); /* get the server's certificate */
    if ( cert != NULL )
    {
	//Falta:
	//Validar que sea correcto el certificado de autoridad
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line);       /* free the malloc'ed string */
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
	parseEmail(line, strlen(line), mail, 30);
	printf("email: %s\n", mail);
	
        free(line);       /* free the malloc'ed string */
        X509_free(cert);     /* free the malloc'ed certificate copy */
    }
    else
        printf("No certificates.\n");
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


int main(int count, char *strings[])
{   SSL_CTX *ctx;
    int server;
    SSL *ssl;
    char buf[10240];
    int bytes;
    char *hostname, *portnum;

    if ( count != 3 )
    {
        printf("usage: %s <hostname> <portnum>\n", strings[0]);
        exit(0);
    }

    SSL_library_init();

 	hostname=strings[1];
 	portnum=strings[2];

    ctx = InitCTX();
    LoadCertificates(ctx, "device.csr", "device.key");
    
    server = OpenConnection(hostname, atoi(portnum));

    ssl = SSL_new(ctx);      /* create new SSL connection state */
    SSL_set_fd(ssl, server);    /* attach the socket descriptor */
   
 if ( SSL_connect(ssl) == FAIL )   /* perform the connection */
        ERR_print_errors_fp(stderr);
    else
    {   
		char *msg = "GET / HTTP/1.1\r\n\r\n";

        printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
        ShowCerts(ssl);        /* get any certs */
        
		SSL_write(ssl, msg, strlen(msg));   /* encrypt & send message */
        bytes = SSL_read(ssl, buf, sizeof(buf)); /* get reply & decrypt */
        
		buf[bytes] = 0;
        printf("Received: \"%s\"\n", buf);
        SSL_free(ssl);        /* release connection state */
    }
    close(server);         /* close socket */
    SSL_CTX_free(ctx);        /* release context */
    return 0;
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
	printf("src = %s, compare = %s\n", init, curCheck);
	printf("initOffset = %i, srcSize = %i\n", initOffset, srcSize);
	while(initOffset < srcSize)
	{
		printf("comparing init+%i (%c) to curCheck+%i(%c)\n", initOffset, *(init+initOffset), curCheckOffset, *(curCheck+curCheckOffset));
		if(*(init+initOffset) == *(curCheck+curCheckOffset))
		{
			printf("	they're equal. Incrementing curCheckOffset.\n");
			curCheckOffset++;
			
			printf("	checking if the comparison finished completely\n");
			printf("	if(%i == %i)\n", curCheckOffset, compareLen);
			if(curCheckOffset == compareLen)
			{
			printf("		Match has completed. Searching for next \\r.\n");
			init = init+initOffset+1;
			//printf("		checking %c\n", *(init+rSeeker));
			while(initOffset+rSeeker < srcSize && (*(init+rSeeker) != '\r' || *(init+rSeeker) != '\n' || *(init+rSeeker) != ' ' || *(init+rSeeker) != '\0'))
			{
				rSeeker++;
				//sleep(1);		
				printf("		checking %c\n", *(init+rSeeker));
			}
				printf("		email is %s\n", init);
				printf("		rSeeker = %i\n", rSeeker);
				bzero(dest, destSize);
				memcpy(dest, init, rSeeker);
				return 1;
			}
		}else
		{
			printf("	they're not equal. Resetting curCheckOffset\n");
			curCheckOffset = 0;
		}
		initOffset++;
	}
	
	return -1;
}

