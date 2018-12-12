#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <strings.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>

#define DLS_PORT 1442

#define DLS_VERSION "DLS 1.0"

#define TRUE 1
#define FALSE 0

#define VERSION_LENGTH 8
#define SENDER_MAX_LENGTH 44
#define LOG_MAX_LENGTH 256
#define REQUEST_SIZE sizeof(DLSRequest)

#define TIMER_LENGTH 19

void getCurrentTimeString(char* buffer);

typedef struct dlsRequest_t
{
	char version[VERSION_LENGTH];
	char sender[SENDER_MAX_LENGTH];
	char log[LOG_MAX_LENGTH];

} DLSRequest;

int sockaddrInSize;
int server, client;

struct sockaddr_in local;
struct sockaddr_in distant;

int main()
{
		printf("===== DLS Server =====\n\n");

		//////////////
		// Starting //
		//////////////
		printf("[LOG] Starting...\n");

		sockaddrInSize = sizeof(struct sockaddr_in);

		//////////////////////////////
		// Init Local Socket Adress //
		//////////////////////////////
		bzero(&local, sizeof(local));
		local.sin_family = AF_INET;
		local.sin_port = htons(DLS_PORT);
		local.sin_addr.s_addr = INADDR_ANY;
		bzero(&(local.sin_zero), 8);

		////////////////////////////
		// Creating Server Socket //
		////////////////////////////
		server = socket(AF_INET, SOCK_STREAM, 0);
		if(server == -1)
		{
			printf("[ERROR] Cannot create socket\n");
			return 1;
		}

		///////////////////////////
		// Binding Server Socket //
		///////////////////////////
		if(bind(server, (struct sockaddr*) &local, sockaddrInSize) == -1)
		{
			printf("[ERROR] Cannot open socket\n");
			return 1;
		}

		////////////////////////////////
		// Listening on Server Socket //
		////////////////////////////////
		if(listen(server, 5) == -1)
		{
			printf("[ERROR] Cannot listen on socket\n");
			return 1;
		}

		printf("[LOG] Server started\n");

		/////////////////
		// Server Loop //
		/////////////////
		while(TRUE)
		{
			//////////////////////
			// Client Connexion //
			//////////////////////
			printf("[LOG] Waiting client...\n");

			client = accept(server, (struct sockaddr*) &distant, (socklen_t *) &sockaddrInSize);

			printf("[LOG] New client %i connected\n", distant.sin_addr.s_addr);

			// Delegate Discussion task
			if(fork() == 0)
			{
				///////////////////////
				// Receiving Request //
				///////////////////////
				DLSRequest request;
				int readed = recv(client, (char*) &request, REQUEST_SIZE, 0);

				if(readed <  (int) REQUEST_SIZE)
				{
					printf("[WARNING] Unknown request from %i\n", distant.sin_addr.s_addr);

					////////////////////////////
					// Answer For Bad Request //
					////////////////////////////
					int writed = send(client, "Unable to process the request", 29, 0);

					if(writed < 29)// Unable to answer
					{
						printf("[WARNING] Unable to dial with %i, closing connection...\n", distant.sin_addr.s_addr);
					}
					else
					{
						printf("[WARNING] End of miss connection with %i\n", distant.sin_addr.s_addr);
					}
				}
				else
				{
					/////////////////////////////////////////////////
					// Placing End of String for Malformed Request //
					/////////////////////////////////////////////////
					request.version[VERSION_LENGTH-1] = '\0';
					request.sender[SENDER_MAX_LENGTH-1] = '\0';	
					request.log[LOG_MAX_LENGTH-1] = '\0';
					
					if(strcmp(request.version, DLS_VERSION) == 0)
					{
						printf("[LOG] Request received from %i\n", distant.sin_addr.s_addr);

						//////////////////
						// Creating Log //
						//////////////////

						//Creating file name
						char fileName[SENDER_MAX_LENGTH + 14];
						sprintf(fileName, "/tmp/log/%s.log", request.sender);

						int logFile = open(fileName, O_WRONLY | O_APPEND | O_CREAT, S_IRWXU);

						//Unable to open file
						if(logFile == -1)
						{
							printf("[WARNING] Unable to open or create log file\n");

							int writed = send(client, "Unable to open or create log", 28, 0);
							if(writed < 29)// Unable to answer
							{
								printf("[WARNING] Unable to answer to %i, closing connection...\n", distant.sin_addr.s_addr);
							}
							else
							{
								printf("[WARNING] End of dialing with %i\n", distant.sin_addr.s_addr);
							}
						}
						else
						{
							int logLength = LOG_MAX_LENGTH + TIMER_LENGTH + 4;
							char logMessage[logLength];
							char timer[TIMER_LENGTH];

							getCurrentTimeString(timer);
							sprintf(logMessage, "[%s] %s\n", timer, request.log);

							logLength = (strchr(logMessage, '\n') - logMessage) + 1;

							int loged = write(logFile, logMessage, logLength);
							close(logFile);

							if(loged < logLength)
							{
								printf("[WARNING] Error while writing log\n");

								int writed = send(client, "Unable to write log", 19, 0);
								if(writed < 29)// Unable to answer
								{
									printf("[WARNING] Unable to answer to %i, closing connection...\n", distant.sin_addr.s_addr);
								}
								else
								{
									printf("[WARNING] End of dialing with %i\n", distant.sin_addr.s_addr);
								}
							}
							else
							{
								printf("[LOG] Request processed. Sending answer...\n");

								////////////////////////////
								// Answer For Good Request //
								////////////////////////////
								int writed = send(client, "Request processed", 17, 0);

								if(writed < 17)
								{
									printf("[WARNING] Unable to answer to %i\n", distant.sin_addr.s_addr);
								}
								else
								{
									printf("[LOG] End of dialing with %i\n", distant.sin_addr.s_addr);
								}
							}
						}
					}
					else
					{
						printf("[WARNING] Incorrect request version\n");

						int writed = send(client, "Incorrect request version", 25, 0);
						if(writed < 25)// Unable to answer
						{
							printf("[WARNING] Unable to answer to %i, closing connection...\n", distant.sin_addr.s_addr);
						}
						else
						{
							printf("[WARNING] End of dialing with %i\n", distant.sin_addr.s_addr);
						}
					}
				}

				////////////////////////
				// Closing Connection //
				////////////////////////
				close(client);
				return 0;
			}
		}

		return 0;
}


void getCurrentTimeString(char* buffer)
{
	time_t actual;
	struct tm* local;

	time(&actual);
	local = localtime(&actual);

	sprintf(buffer, "%d:%d:%d %d/%d/%d", local->tm_hour, local->tm_min, local->tm_sec, local->tm_mday, local->tm_mon, local->tm_year);
}
