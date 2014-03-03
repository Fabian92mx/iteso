/**
@file server.c
@brief Funciones generales para trabajar con un Socket de tipo TCP

@authors Javier de la Mora y Fabián Escobar
@date Mar/2014

*/
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "server.h"
#include "tcp.h"
#include "debug.h"
#include "global.h"
#include "errno.h"

void clientProccess(const int clientSocket);

int startServer(const unsigned int port) {

	int serverSocket;
	int clientSocket;
	char clientIP[16];
	unsigned int clientPort;

	int pid;

	serverSocket = newTCPServerSocket4("0.0.0.0",port,5);
	if(serverSocket == -1) {
		error(errno,"Can't create Socket");
		return FALSE;
	}
	debug(4,"Server Socket Created");

	while(TRUE) {
		clientSocket = waitConnection4(serverSocket,clientIP,&clientPort);
	debug(2,"Connected Client %s:%u",clientIP,clientPort);

	pid = fork();
	if(pid == -1) {
		debug(0,"Can't Fork!");
		close(clientSocket);
		continue;
	} else if(pid == 0 ) {
		// Soy el Hijo.
		clientProccess(clientSocket);
	} else if (pid > 0 ) {
		// Soy el Padre
		close(clientSocket);
		}
	}

	return TRUE;

}


void clientProccess(const int clientSocket) {

	char *buffer;
	char *firstLine;
	char html[250];
	int file;
	int readBytes;
	int firstFlag;

	debug(2,"Inicio del proceso del Cliente\n");


	buffer = calloc(255,1);
	firstLine = calloc(255,1);
	firstFlag = TRUE;
	while(readTCPLine4(clientSocket,buffer,254)>0) {
		debug(4,"%s",buffer);
		if(strcmp(buffer,"\r\n")==0) {
			break;
		}
		if(firstFlag) {
			strcpy(firstLine,buffer);
			firstFlag = FALSE;
		}
		bzero(buffer,255);
	}

	// PROCESAR EL GET
	// TIP para esto: strtok

	//RESPONDER CON 200 OK SI EXISTE
	strcpy(html, "HTTP/1.1 200 OK\r\n");
	sendTCPLine4(clientSocket, html,strlen(html));
	//strcpy(html, "Content-Type: \r\n");
	//sendTCPLine4(clientSocket, html,strlen(html));
	//strcpy(html, "Content-Lenght: 457838592\r\n");
	//sendTCPLine4(clientSocket, html,strlen(html));
	strcpy(html, "\r\n");
	sendTCPLine4(clientSocket, html,strlen(html));

	//ENVIAR EL ARCHIVO
	file = open("archivo.html",O_RDONLY);
	if(file == -1) {
		error(errno, "No se pudo abrir el archivo");
	}

	while((readBytes = read(file,buffer,255))>0) {
		sendTCPLine4(clientSocket,buffer,readBytes);
	}

	//RESPONDER CON 404 NOT FOUND SI NO EXISTE

	//strcpy(html, "<html><head><title>404 NOT FOUND</title></head><body>NO EXISTE!!!</body></html>");
	//sendTCPLine4(clientSocket, html,strlen(html));

	//CERRAMOS LA COMUNICACIÓN
	close(clientSocket);
	free(buffer);
	free(firstLine);

	return;
}
