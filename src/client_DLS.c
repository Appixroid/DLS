#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <strings.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <netdb.h>

#define LOCALHOST "127.0.0.1"
#define DLS_PORT 1442

#define DLS_VERSION "DLS 1.0"

#define TRUE 1
#define FALSE 0

#define VERSION_LENGTH 8
#define SENDER_MAX_LENGTH 44
#define LOG_MAX_LENGTH 256
#define REQUEST_SIZE sizeof(DLSRequest)

typedef struct dlsRequest_t
{
	char version[VERSION_LENGTH];
	char sender[SENDER_MAX_LENGTH];
	char log[LOG_MAX_LENGTH];

} DLSRequest;

int server;

struct sockaddr_in serv_addr;
struct hostent* server_host;

int main()
{
	printf("===== DLS Client =====\n");

	//////////////////////////
	// Connection to Server //
	//////////////////////////
	printf("[LOG] Connecting...\n");

	server_host = gethostbyname(LOCALHOST);
	if(!server_host)
	{
		printf("[ERROR] Unknown host\n");
		return 1;
	}

	// Opening Socket
	bzero(&serv_addr, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(DLS_PORT);
	bcopy(server_host->h_addr_list[0], &serv_addr.sin_addr.s_addr, server_host->h_length);

	server = socket(AF_INET, SOCK_STREAM, 0);

	if(server == -1)
	{
		printf("[ERROR] Unable to create socket\n");
		return 1;
	}

	if(connect(server, (struct sockaddr*) &serv_addr, sizeof(struct sockaddr)) == -1)
	{
		printf("[ERROR] Unable to connect server\n");
		return 1;
	}

	printf("[LOG] Connection done. Creating request...\n");

	//////////////////////
	// Creating Request //
	//////////////////////
	DLSRequest request;

	strcpy(request.version, DLS_VERSION);

	printf("Logger Software :\n");
	int readed = read(2, request.sender, SENDER_MAX_LENGTH - 1);
	if(readed < SENDER_MAX_LENGTH)
	{
		request.sender[readed-1] = '\0';
	}
	else
	{
		request.sender[SENDER_MAX_LENGTH - 1] = '\0';
	}

	printf("Log :\n");
	readed = read(2, request.log, LOG_MAX_LENGTH - 1);
	if(readed < LOG_MAX_LENGTH)
	{
		request.log[readed-1] = '\0';
	}
	else
	{
		request.log[LOG_MAX_LENGTH - 1] = '\0';
	}

	printf("[LOG] Sending request...\n");

	/////////////////////
	// Sending Request //
	/////////////////////
	int writed = send(server, (char *) &request, REQUEST_SIZE, 0);

	if(writed < (int) REQUEST_SIZE)
	{
		printf("[ERROR] Error while sending the request\n");
		close(server);
		return 1;
	}
	else
	{
		char answer[50];
		recv(server, answer, 50, 0);

		printf("[LOG] Server say \"%s\"\n", answer);
	}

	/////////////
	// Closing //
	/////////////
	close(server);
	return 0;
}
