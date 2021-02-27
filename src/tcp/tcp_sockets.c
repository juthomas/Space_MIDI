#include "../../inc/midi.h"
#define MAX 80

static void msg_listener(int sockfd)
{
	char buff[MAX];
	int ret;
	// infinite loop for listening
	for (;;)
	{
		bzero(buff, MAX);
		// read the message from client and copy it in buffer
		ret = read(sockfd, buff, sizeof(buff));
		// print buffer which contains the client contents
		if (ret == 0)
		{
			break;
		}
		if (strlen(buff) > 0)
		{
			printf("From client: %s\n", buff);
		}
	}
}

static void connect_loop(int sockfd)
{
	// Accept the data packet from client and verification
	uint32_t len;
	struct sockaddr_in cli;
	int connfd;
	len = sizeof(cli);
	
	for (;;)
	{
		connfd = accept(sockfd, (struct sockaddr *)&cli, &len);
		if (connfd < 0)
		{
			printf("server acccept failed...\n");
			exit(0);
		}
		else
		{
			printf("server acccept the client...\n");
		}
		// Function for chatting between client and server
		msg_listener(connfd);
	}
}

void tcp_connection()
{
	int sockfd;
	struct sockaddr_in servaddr;

	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		printf("socket creation failed...\n");
		exit(0);
	}
	else
	{
		printf("Socket successfully created..\n");
	}
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	// Binding newly created socket to given IP and verification
	if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
	{
		printf("socket bind failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");

	// Now server is ready to listen and verification
	if ((listen(sockfd, 5)) != 0)
	{
		printf("Listen failed...\n");
		exit(0);
	}
	else
	{
		printf("Server listening..\n");
	}
	connect_loop(sockfd);

	// After chatting close the socket
	close(sockfd);
}