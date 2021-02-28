#include "../../inc/midi.h"
#define MAX 65536

int32_t		get_param_from_server_data(char *data, char *param_name)
{
	char *tmp = strstr(data, param_name);
	int32_t		param_nu;

	if (tmp)
	{
		param_nu = atoi(tmp + strlen(param_name));
		// free(tmp);
		return (param_nu);
	}
	return (0);
}


static void msg_listener(t_server_data *data)
{
	char *buff;
	buff = (char *)malloc(sizeof(char) * MAX);
	int ret;
	// infinite loop for listening
	for (;;)
	{
		bzero(buff, MAX);
		// read the message from client and copy it in buffer
		ret = read(data->connfd, buff, MAX);
		// print buffer which contains the client contents
		// char *lastmsg = strchr(buff, '|');

		if (ret == 0)
		{
			break;
		}
		if (strlen(buff) > 0)
		{
			char *lastmsg = strrchr(buff, '|');
			if (lastmsg)
			{
				data->temperature = get_param_from_server_data(lastmsg,"temperature :");
				data->light = get_param_from_server_data(lastmsg,"light :");
				printf("Temp nu%d\n\n", data->temperature);
				printf("Light nu%d\n\n", data->light);
			}

			printf("Last msg :%s\n", lastmsg);
			printf("From client: %s\n", buff);
		}
		sleep(2);
	}
}

static void connect_loop(t_server_data *data)
{
	// Accept the data packet from client and verification
	uint32_t len;
	struct sockaddr_in cli;
	len = sizeof(cli);

	for (;;)
	{
		data->connfd = accept(data->sockfd, (struct sockaddr *)&cli, &len);
		if (data->connfd < 0)
		{
			printf("server acccept failed...\n");
			exit(0);
		}
		else
		{
			printf("server acccept the client...\n");
		}
		// Function for chatting between client and server
		msg_listener(data);
	}
}

int32_t tcp_get_fresh_data(t_server_data *data)
{
	char *buff;
	buff = (char *)malloc(sizeof(char) * MAX);
	int ret;
	// infinite loop for listening

	bzero(buff, MAX);
	// read the message from client and copy it in buffer
	data->read_state = read(data->connfd, buff, MAX);
	// print buffer which contains the client contents
	// char *lastmsg = strchr(buff, '|');

	if (data->read_state > 0 && strlen(buff) > 0)
	{
		char *lastmsg = strrchr(buff, '|');
		if (lastmsg)
		{
			data->temperature = get_param_from_server_data(lastmsg,"temperature :");
			data->light = get_param_from_server_data(lastmsg,"light :");
			printf("Temp nu%d\n\n", data->temperature);
			printf("Light nu%d\n\n", data->light);
		}

		printf("Last msg :%s\n", lastmsg);
		printf("From client: %s\n", buff);
	}
	free(buff);
	return (data->read_state);
}

void tcp_connection(t_server_data *data)
{
	struct sockaddr_in servaddr;

	// socket create and verification
	data->sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (data->sockfd == -1)
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
	if ((bind(data->sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
	{
		printf("socket bind failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");

	// Now server is ready to listen and verification
	if ((listen(data->sockfd, 5)) != 0)
	{
		printf("Listen failed...\n");
		exit(0);
	}
	else
	{
		printf("Server listening..\n");
	}
	
	connect_loop(data);

	// After chatting close the socket
	close(data->sockfd);
}