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
#define USER 0
#define POLICE 1
#define ADMIN 2
#define NO_USER -1
#define INTERNAL_ERROR -2
#define EXIT -3
#define SUCCESS 3

// header start
char* Msg_From_Client(int sock_fd);
void Msg_To_Client(int sock_fd,char* str_to_send);
void handling_client(int client_fd);
void close_socket_from_server_side(int client_fd);
int is_terminate_msg(char * msg_from_client);
int authenticate_user(char* username,char* password);
int checkUser(char *username);
void close_socket_for_internal_error(int client_fd);
void requests_of_user(char* username,char* password,int client_fd);
char* get_available_balance(char* username);
char* get_mini_stat(char* username);
void update_user_balance(char *username,int transaction,double balance);
int Client_Query(char *username, int client_fd);

// header finish----------

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

int is_terminate_msg(char * msg_from_client){
	if(msg_from_client == NULL){
		return 1;
	}
	else if(strncmp(msg_from_client,"Terminate",9) == 0){
		return 1;
	}
	else{
		return 0;
	}
}
void close_socket_for_internal_error(int client_fd){
		printf("An internal error occurred\n");
		Msg_To_Client(client_fd,"INTERNAL_ERROR occurred. Try again later\n");
		char* msg_from_client;
		shutdown(client_fd, SHUT_WR);
		   while(1) {
        		msg_from_client = Msg_From_Client(client_fd);
        		if(msg_from_client == NULL)
            		break;
        		free(msg_from_client);
    		}
    	shutdown(client_fd, SHUT_RD);
    	close(client_fd);	
    	printf("%d closed due to internal error\n",client_fd);
}


// the termination initiated by the client
void close_socket_from_server_side(int client_fd){
		shutdown(client_fd, SHUT_RD);
		Msg_To_Client(client_fd,"Thanks...\n");
		shutdown(client_fd, SHUT_WR);
		close(client_fd);
		printf("%d  closed gracefully\n",client_fd);
}

int authenticate_user(char* username,char* password){
	char * line = NULL;
    size_t len = 0;
    ssize_t read;


	FILE *fp=fopen("user_login","r");
	if(fp == NULL){
		printf(" problem in opening user_login file\n");
		return INTERNAL_ERROR;
	}
	while((read = getline(&line, &len, fp)) != -1) 
	{
		char *token=strtok(line," ");

		if(token == NULL){fclose(fp); printf(" problem in opening user_login file entries\n"); return INTERNAL_ERROR;}
		
		if(strcmp(token,username)==0)
		{
			token=strtok(NULL," ");
			
			if(token == NULL) {fclose(fp);printf(" problem in opening user_login file entries\n"); return INTERNAL_ERROR; }
			
			if(strcmp(token,password)==0)
			{
				token=strtok(NULL," ");
				
				if(token == NULL) {fclose(fp);printf(" problem in opening user_login file entries\n"); return INTERNAL_ERROR;}
                
                if(token[0]=='C')
                {
					fclose(fp);
                    return USER;    //return the user type
                }
                else if(token[0]=='A')
                {
                    fclose(fp);
                    return ADMIN;
                }
                else if(token[0]=='P')
                {
                    fclose(fp);
                    return POLICE;
                }
            }
        }
    }
    if(line!=NULL)
        free(line);

    fclose(fp);
	return NO_USER;
}

int checkUser(char *user)
{
	FILE *fp=fopen("user_login","r");
	char * line = NULL;
    size_t len = 0;
    ssize_t read;

	while((read = getline(&line, &len, fp)) != -1) 
	{
		char *token=strtok(line," ");

		if(token == NULL){fclose(fp); printf(" problem in opening user_login file entries\n"); return 0;}

		if(strcmp(token,user)==0)
		{
			token=strtok(NULL," ");
			token=strtok(NULL," ");
			if(token[0]=='C')
			{
				fclose(fp);
				return 1;
			}
        }
    }

    fclose(fp);
    return 0;


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

char* get_available_balance(char* username){

}

char* get_mini_stat(char* username){


}

void update_user_balance(char *username,int transaction,double balance)
{
	FILE *fp=fopen(username,"r");
	char * line = NULL;
	char c=(transaction == 1)?'C':'D';
	char *line1=(char *)malloc(sizeof(char)*10000);
    size_t len = 0;
    ssize_t read;
	time_t ltime; /* calendar time */

	ltime=time(NULL); /* get current cal time */
	sprintf(line1,"%.*s %c %f\n",(int)strlen(asctime(localtime(&ltime)))-1,asctime(localtime(&ltime)),c,balance);

	while((read = getline(&line, &len, fp)) != -1)
		strcat(line1,line);

	fclose(fp);
	fp=fopen(username,"w");
	fwrite(line1, sizeof(char), strlen(line1), fp);
	fclose(fp);
}

int Client_Query(char *username, int client_fd)
{
	Msg_To_Client(client_fd,"You are logged in as user.\n
			1) Type `Credit` to credit balance to an account.\n
			2) Type `Debit` to debit balance from an account.\n
			3) Type `Terminate` to quit\n");
	int query_flag = -1;
	char* msg_from_client = NULL;
	while(1){
		msg_from_client = Msg_From_Client(client_fd);
		if(msg_from_client == NULL){
			return EXIT;
		}
		else if(strncmp(msg_from_client,"Terminate",9)== 0){
			return EXIT;
		}
		else if(strncmp(msg_from_client,"Credit",6)== 0) {
			query_flag = 1;
		}
		else if(strncmp(msg_from_client,"Debit",5)== 0){
			query_flag = 2;
		}
		else{
			query_flag = -1;
		}

		free(msg_from_client);
		double balance=strtod(get_available_balance(username),NULL);

		switch(query_flag) {
			case 1:
				Msg_To_Client(client_fd, "Enter Amount to be credited");
				while(1){	
					msg_from_client = Msg_From_Client(client_fd);
					double amount = strtod(msg_from_client, NULL);

					if(amount<=0){
						Msg_To_Client(client_fd, "Enter valid amount");
					}
					else {
						balance += amount;
						update_user_balance(username, query_flag, balance);
						Msg_To_Client(client_fd, "Credit successful\n
						1) Type username of account holder to perform transactions.\n
	    				2) Type `Terminate` to quit\n");
	    				return SUCCESS;
					}
				}
			case 2:
				Msg_To_Client(client_fd, "Enter Amount to be debited");
				while(1){	
					msg_from_client = Msg_From_Client(client_fd);
					double amount = strtod(msg_from_client, NULL);

					if(amount<=0){
						Msg_To_Client(client_fd, "Enter valid amount");
					}
					else if(amount > balance) {
						Msg_To_Client(client_fd, "Insufficient Balance!\n Enter valid amount to debit");
					}
					else {
						balance -= amount;
						update_user_balance(username, query_flag, balance);
						Msg_To_Client(client_fd, "Debit successful\n
						1) Type username of account holder to perform transactions.\n
	    				2) Type `Terminate` to quit\n");
	    				return SUCCESS;
					}
				}
			default:
				Msg_To_Client(client_fd, "Unknown Query\n
					1) Type `Credit` to credit balance to an account.\n
					2) Type `Debit` to debit balance from an account.\n
					3) Type `Terminate` to quit\n");
		}
	}
}

void requests_of_user(char* username,char* password,int client_fd){
		Msg_To_Client(client_fd,"You are logged in as user.\n
			1) Type `Request: Bal` to see available balance of your account.\n
			2) Type `Request: Mini_stat` to see mini statement of your account.\n
			3) Type `Terminate` to quit\n");
		int user_flag = -1;
		char* msg_from_client = NULL;
		while(1){
			msg_from_client = Msg_From_Client(client_fd);
			if(msg_from_client == NULL){
				user_flag = 0;
				break;
			}
			else if(strncmp(msg_from_client,"Terminate",9)== 0){
				user_flag = 0;
				break;
			}
			else if(strncmp(msg_from_client,"Request: Bal",12)== 0) {
				user_flag = 1;
			}
			else if(strncmp(msg_from_client,"Request: Mini_stat",18)== 0){
				user_flag = 2;
			}
			else{
				user_flag = -1;
			}


			if(msg_from_client != NULL) free(msg_from_client);
			char* out_bal= (char*) malloc(1000*sizeof(char));
			char* out_mini_stat = (char*)malloc(10000*sizeof(char));
			
			switch(user_flag){
				case 1:
					strcpy(out_bal,get_available_balance(username));
					Msg_To_Client(client_fd,strcat(out_bal,"\n======================================\n\nType your next Query or Type `Terminate` to quit.\n"));
					if(out_bal != NULL) free(out_bal);
					break;
				case 2:
			 		strcpy(out_mini_stat,get_mini_stat(username));
					Msg_To_Client(client_fd,strcat(out_mini_stat,"\n====================================\n\nType your next Query or Type `Terminate` to quit.\n"));
					if(out_mini_stat != NULL) free(out_mini_stat);
					break;
				default:
					Msg_To_Client(client_fd, "Unknown Query.\n1) Type `Request: Bal` to see available balance of your account.\n2) Type `Request: Mini_stat` to see mini statement of your account.\n3) Type `Terminate` to quit\n");
					break;
			}
		}

		close_socket_from_server_side(client_fd);


}

void requests_of_admin(){
	Msg_To_Client(client_fd,"You are logged in as admin.\n
			1) Type username of account holder to perform transactions.\n
    		2) Type `Terminate` to quit\n");

	while(1)
	{
		char *msg_from_client = NULL;
		msg_from_client = Msg_From_Client(client_fd);

		if(msg_from_client == NULL) {
			break;
		}
		else if(strncmp(msg_from_client,"Terminate",9)== 0) {
			break;
		}
		else if(checkUser(msg_from_client))
		{
			char *username=(char *)malloc(40*sizeof(char));
			strcpy(username,msg_from_client);

			int query_result = Client_Query(username, client_fd);

			if(query_result == EXIT)
				break;
		}
		else
			Msg_To_Client(client_fd,"Wrong Username. Please enter a valid user");
	}


}

void requests_of_police(int client_fd){
	Msg_To_Client(client_fd,"You are logged in as police.\n
			1) Type `Request: Bal` to see balance of all users.\n
 		    2) Type `Request: Mini_stat` to see mini statement of an account.\n
    		3) Type `Terminate` to quit\n");
	int police_flag = -1;
	char* msg_from_client = NULL;
	while(1)
	{
		msg_from_client = Msg_From_Client(client_fd);

		if(msg_from_client == NULL){
			police_flag = 0;
			break;
		}
		else if(strncmp(msg_from_client,"Terminate",9)== 0){
			police_flag = 0;
			break;
		}
		else if(strncmp(msg_from_client,"Request: Bal",12)== 0) {
			police_flag = 1;
		}
		else if(strncmp(msg_from_client,"Request: Mini_stat",18)== 0){
			police_flag = 2;
		}
		else{
			police_flag = -1;
		}

		free(msg_from_client);

		char* out_bal = (char*) malloc(1000*sizeof(char));
		char* out_mini_stat = (char*)malloc(10000*sizeof(char));

		switch(police_flag){
			case 1:
				strcpy(out_bal,getBalanceAll());
				Msg_To_Client(client_fd,strcat(out_bal,"\n======================================\n\nType your next Query or Type `Terminate` to quit.\n"));
				if(out_bal != NULL) free(out_bal);
				break;
			case 2:
				Msg_To_Client(client_fd, "Enter UserName or Type `Terminate` to exit");
				while(1) {
					msg_from_client = Msg_From_Client(client_fd);
					if(msg_from_client == NULL) {
						police_flag = 0;
						break;
					}
					else if(strncmp(msg_from_client,"Terminate",9)== 0) {
						police_flag = 0;
						break;
					}
					else if(checkUser(msg_from_client)) {
						char *username=(char *)malloc(sizeof(char)*40);
						strcpy(username,msg_from_client);
				 		strcpy(out_mini_stat,get_mini_stat(username));
						Msg_To_Client(client_fd,strcat(out_mini_stat,"\n====================================\n\nType your next Query or Type `Terminate` to quit.\n"));
						if(out_mini_stat != NULL) free(out_mini_stat);
						break;
					}
				}
				break;
			default:
				Msg_To_Client(client_fd, "Unknown Query.\n	
						1) Type `Request: Bal` to see balance of all users.\n
			 		    2) Type `Request: Mini_stat` to see mini statement of an account.\n
    					3) Type `Terminate` to quit\n");
				break;
		}
		if(police_flag == 0) {
			break;
		}
	}
	close_socket_from_server_side(client_fd);
}






// also closes the ongoing connection gracefully
void handling_client(int client_fd){
		
	   int authorisation = NO_USER;
	   int try =0;
	   char* username;
       char* password;
       char* msg_from_client;
	   while(1){       		
       		if(try){
       			msg_from_client = Msg_From_Client(client_fd);
       			if(is_terminate_msg(msg_from_client)){
       	 			close_socket_from_server_side(client_fd);return;
       			}       			
       		}
       		username = (char*)malloc(150*sizeof(char));
      		password = (char*)malloc(150*sizeof(char));
       		Msg_To_Client(client_fd,"Enter username:\n");
       		msg_from_client = Msg_From_Client(client_fd);
       		int i=0;
       		if(is_terminate_msg(msg_from_client)){
       	 		close_socket_from_server_side(client_fd);return;
       		}
       		else{
       			i=0;
       			while(msg_from_client[i]!='\0' && msg_from_client[i]!='\n'){
					username[i]=msg_from_client[i];
					i++;
	      		}
       		}
       		username[i] = '\0';
       		free(msg_from_client);

       		Msg_To_Client(client_fd,"Enter password:\n");
       		msg_from_client = Msg_From_Client(client_fd);

       		if(is_terminate_msg(msg_from_client)){
       	 		close_socket_from_server_side(client_fd);return;
       		}
       		else{
       			i =0;
       			while(msg_from_client[i]!='\0' && msg_from_client[i]!='\n'){
					password[i]=msg_from_client[i];
					i++;
	      		}
       		}
       		password[i] = '\0';
       		free(msg_from_client);

       		authorisation = authenticate_user(username,password);
       		printf("%d authorisation\n",authorisation );
       		try = 1;
       		if(authorisation == NO_USER){
       			Msg_To_Client(client_fd,"Unathorised Login.Type Terminate to terminate or any key to try again.\n");
       			if(username != NULL) free(username);
       			if(password != NULL) free(password);
       		}
       		else if(authorisation == INTERNAL_ERROR){
       			close_socket_for_internal_error(client_fd);
       			if(username != NULL) free(username);
       			if(password != NULL) free(password);
       			return;
       		}
       		else{
       			break;
       		}
       	}	
       	switch(authorisation){
			case USER:
			    requests_of_user(username,password,client_fd);
				break;
			case ADMIN:
				Msg_To_Client(client_fd,"logged in as admin\n");
			    requests_of_admin();
				break;	
			case POLICE:
			    Msg_To_Client(client_fd,"logged in as police\n");
			    requests_of_police();
				break;	
			default:
				close_socket_for_internal_error(client_fd);
				break;
		}

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
	            printf("%d accepted\n",client_fd);
	            handling_client(client_fd);
	            exit(EXIT_SUCCESS);
	            break;
	        }
	        default:
	            close(client_fd);
	            break;
	    }
	}

}