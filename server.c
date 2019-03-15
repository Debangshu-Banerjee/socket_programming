#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>


#define INPUT_MSG_SIZE 512
#define OUTPUT_MSG_SIZE 512

char* Msg_From_Client(int sock_fd);
void Msg_To_Client(int sock_fd,char* str_to_send);
void handling_client(int client_fd);


// helper function to receive msg from server
char* Msg_From_Client(int sock_fd) {
    int num_pkts_currently_received = 0;
    int n = read(sock_fd, &num_pkts_currently_received, sizeof(int));
    if(n <= 0) {
        //shutdown(sock_fd, SHUT_WR); /* needs attention*/
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
void Msg_To_Client(int sock_fd,char* str_to_send){
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

void handling_client(int client_fd){
       char* username;
       char* password;
       char* msg_from_client;
       username = (char*)malloc(150*sizeof(char));
       password = (char*)malloc(150*sizeof(char));
       Msg_To_Client(clint_fd,"Enter username:\n");
       msg_from_client = 


}

int main(int argc, char **argv){

	int sock_fd,client_fd,port_no;
	struct sockaddr_in serv_addr, cli_addr;

	memset((void*)&serv_addr, 0, sizeof(serv_addr));
	port_no=atoi(argv[1]);

	sock_fd=socket(AF_INET, SOCK_STREAM, 0);

	serv_addr.sin_port = htons(port_no);         //seting the port number passed as argument
	serv_addr.sin_family = AF_INET;              //setting DOMAIN
	serv_addr.sin_addr.s_addr = INADDR_ANY;      //permits any incoming IP

	if(bind(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
	    printf("Error on binding.\n");
	    return -1; // attention needed
	}

	int socket_reuse=1;
	setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &socket_reuse, sizeof(int));

	listen(sock_fd, 7); 
	int client_size=sizeof(cli_addr);

	int count = 0;
	while(1) {

	    memset(&cli_addr, 0, sizeof(cli_addr));
	    if((client_fd = accept(sock_fd, (struct sockaddr*)&cli_addr, &client_size)) < 0) {
	        printf("Error on accept.\n");
	        exit(EXIT_FAILURE);
	    }

	    switch(fork()) {
	        case -1:
	            printf("Error in fork.\n"); // needs attention
	            break;
	        case 0: {	        	
	            close(sock_fd);
	            exit(EXIT_SUCCESS);
	            break;
	        }
	        default:
	            close(client_fd);
	            break;
	    }
	}

}