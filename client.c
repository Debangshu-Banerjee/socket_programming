#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define INPUT_MSG_SIZE 512
#define OUTPUT_MSG_SIZE 512

char* Msg_From_Server(int sock_fd);
void Msg_To_Server(int sock_fd,char* str_to_send);
void close_socket_from_client(int sock_fd);
void close_socket_due_to_internal_error(int sock_fd);

void close_socket_from_client(int sock_fd){
	Msg_To_Server(sock_fd,"Terminate");
	shutdown(sock_fd, SHUT_WR);
	char *msg_from_server;
	while(1) {
        msg_from_server = Msg_From_Server(sock_fd);

        if(msg_from_server == NULL)
            break;
        printf("%s\n", );
        free(msg_from_server);
    }
    shutdown(sock_fd, SHUT_RD);
   	close(sock_fd);
   	printf("closed socket gracefully\n");
}

void close_socket_due_to_internal_error(int sock_fd){
		shutdown(sock_fd, SHUT_RD);
		shutdown(sock_fd, SHUT_WR);
		close(sock_fd);
		printf("closed socket gracefully\n");
}

// helper function to receive msg from server
char* Msg_From_Server(int sock_fd) {
    int num_pkts_currently_received = 0;
    int n = read(sock_fd, &num_pkts_currently_received, sizeof(int));
    if(n <= 0) {
        return NULL;
    }
    char *str = (char*)malloc(num_pkts_currently_received*INPUT_MSG_SIZE);
    memset(str, 0, num_pkts_currently_received*INPUT_MSG_SIZE);
    char *str_p = str;
    int i;
    for(i = 0; i < num_pkts_currently_received; i++) {
        int n = read(sock_fd, str, INPUT_MSG_SIZE);
        str = str+INPUT_MSG_SIZE;
    }
    return str_p;
}



// helper function for sending msg to server
void Msg_To_Server(int sock_fd,char* str_to_send){
	if(str_to_send == NULL) return;
	int num_pkts_to_send_currently = 0;
	num_pkts_to_send_currently = (int)(strlen(str_to_send) - 1)/OUTPUT_MSG_SIZE + 1;
	int n = write(sock_fd, &num_pkts_to_send_currently, sizeof(int));
	char * msg_to_send = (char*)malloc(num_pkts_to_send_currently * OUTPUT_MSG_SIZE);
	strcpy(msg_to_send, str_to_send);
	int i;
	for(i=0;i<num_pkts_to_send_currently;i++){
		int n= write(sock_fd,msg_to_send,OUTPUT_MSG_SIZE);
		msg_to_send = msg_to_send + OUTPUT_MSG_SIZE;		
	}
}

int main(int argc,char** argv){

	int sock_fd,port_no;
	struct sockaddr_in serv_addr;

	  
	char *msg_from_server;
    char msg_to_server[512];
	

	sock_fd=socket(AF_INET, SOCK_STREAM, 0);
	port_no = atoi(argv[2]);

	memset(&serv_addr, 0, sizeof(serv_addr));  
	serv_addr.sin_family = AF_INET;         //setting DOMAIN
    serv_addr.sin_port = htons(port_no);     //setting server port number passed as an argument
    if(!(inet_aton(argv[1], &serv_addr.sin_addr)))   printf("server address is not valid\n"); // setting server host name passed as an argument
  
    if(connect(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
     	printf("can not connect to the server\n");
     	return -1;
    }
    else{
     	printf("connection established\nlogin to avail the functionalities\n");
    }

    while(1){

     	msg_from_server = Msg_From_Server(sock_fd);
     	if(msg_from_server == NULL){
     		close_socket_due_to_internal_error(sock_fd);
     		break;
     	}
     	printf("%s",msg_from_server);
		if(strncmp(msg_from_server, "INTERNAL_ERROR", 14 ) == 0){
			close_socket_due_to_internal_error(sock_fd);
			break;
		}
		scanf("%s",msg_to_server);
		if(strncmp(msg_to_server,"Request:",8) == 0){
			strcat(msg_to_server," ");
			char temp[400];
			scanf("%s",temp);
			strcat(msg_to_server,temp);

		}    	
		if(strncmp(msg_to_server,"Terminate",9) == 0){
			close_socket_from_client(sock_fd);
			break;
		}
		Msg_To_Server(sock_fd,msg_to_server);

    }
    return 0;
}