#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>


#define INPUT_MSG_SIZE 512   /* part of input msg sent at one go*/
#define OUTPUT_MSG_SIZE 512  /* part of ouput msg received*/
#define USER 0
#define POLICE 1
#define ADMIN 2
#define NO_USER -1
#define INTERNAL_ERROR -2  // set when some internal error has been occured at server side
#define lines_to_print_in_mini_stat 8 // max no of transaction to be printed in mini statement
#define EXIT -3  // set when admin has unsuccessful debit or credit
#define SUCCESS 3 // set when admin has successful debit or credit 


// header start--------(list of function used )

char* Msg_From_Client(int sock_fd);
void Msg_To_Client(int sock_fd,char* str_to_send);
void handling_client(int client_fd);
void close_socket_from_server_side(int client_fd);
int is_terminate_msg(char * msg_from_client);
int authenticate_user(char* username,char* password);
int check_user(char *username);
void close_socket_for_internal_error(int client_fd);
void requests_of_user(char* username,char* password,int client_fd);
char* get_available_balance(char* username);
char* get_mini_stat(char* username);
void update_user_balance(char *username,int transaction,double balance);
int Client_Query(char *username, int client_fd);
void requests_of_police(int client_fd);
void requests_of_admin(int client_fd);
char* get_balance_all();

// header finish----------

// helper function to receive msg from client
char* Msg_From_Client(int sock_fd) {
    int num_pkts_currently_received = 0;
    int n = read(sock_fd, &num_pkts_currently_received, sizeof(int)); // no of pkts of size 512 to be received in this go
    if(n <= 0) {
        return NULL; // if error occurred  in receiving the messages 
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

// check wheather the client is saying to terminate the connection or not
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

// suffered an internal error and has to close the socket
void close_socket_for_internal_error(int client_fd){
		printf("An internal error occurred\n");
		Msg_To_Client(client_fd,"INTERNAL_ERROR occurred. Try again later\n");
		char* msg_from_client;
		shutdown(client_fd, SHUT_WR);  // no more packets will be written from server side
		   while(1) {
        		msg_from_client = Msg_From_Client(client_fd); // receive the packets cient has already sent
        		if(msg_from_client == NULL)
            		break;
        		free(msg_from_client);
    		}
    	shutdown(client_fd, SHUT_RD); // give up receiving more messages 
    	close(client_fd);  // frees up resources 
    	printf("%d closed due to internal error\n",client_fd);
}


// the termination initiated by the client
void close_socket_from_server_side(int client_fd){
		shutdown(client_fd, SHUT_RD); // client has already stopped sending messages so stop listening
		Msg_To_Client(client_fd,"Thanks...\n");
		shutdown(client_fd, SHUT_WR); // give up writing
		close(client_fd);// free up resources 
		printf("%d  closed gracefully\n",client_fd);
}

// if the username and password is correct returns the mode of client
int authenticate_user(char* username,char* password){
	char * line = NULL;
    size_t len = 0;
    ssize_t read;


	FILE *fp=fopen("user_login","r");
	if(fp == NULL){
		printf(" problem in opening user_login file\n"); // if user login file is corrupted 
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
// check the username is a valid user mode holder or not
int check_user(char *user)
{
	FILE *fp=fopen("user_login","r");
	char * line = NULL;
    size_t len = 0;
    ssize_t read;

	while((read = getline(&line, &len, fp)) != -1)
	{
		char *token=strtok(line," ");

		if(token == NULL){fclose(fp); printf(" problem in user_login file entries\n"); return 0;}

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


// helper function for sending msg to client
void Msg_To_Client(int sock_fd,char* str_to_send){
	if(str_to_send == NULL) return;
	int num_pkts_to_send_currently = 0;   
	num_pkts_to_send_currently = (int)(strlen(str_to_send) - 1)/OUTPUT_MSG_SIZE + 1; // fragment the messages in packets of size 512
	int n = write(sock_fd, &num_pkts_to_send_currently, sizeof(int));
	char * msg_to_send = (char*)malloc(num_pkts_to_send_currently * OUTPUT_MSG_SIZE);
	strcpy(msg_to_send, str_to_send);
	int i;
	for(i=0;i<num_pkts_to_send_currently;i++){
		int n= write(sock_fd,msg_to_send,OUTPUT_MSG_SIZE);
		msg_to_send = msg_to_send + OUTPUT_MSG_SIZE;
	}
}

// returns the currently available balance of the user called from user mode
char* get_available_balance(char* username){
	FILE * fp = fopen(username,"r");
	char* line = NULL;
	size_t len = 0;
    ssize_t read;
    if(fp == NULL){   // problem with trasaction history file 
    	char *available_bal=(char *)malloc(100*sizeof(char));
		strcpy(available_bal,"Could not find the file having your transaction history.\n");
		return available_bal;
	}
    if((read = getline(&line, &len, fp)) != -1){
    	char *token,*prevtoken;
    	prevtoken=(char *)malloc(400*sizeof(char));
    	token=strtok(line," \n");
    	while(token!=NULL)
    	{
    		strcpy(prevtoken,token);
    		token=strtok(NULL," \n");
    	}
    	fclose(fp);
    	return prevtoken; // prevtoken holds the value of available balance 
    }
    else{
    	fclose(fp); // if the person has not participated in any transaction return 0 as available balance
    	char *available_bal=(char *)malloc(2*sizeof(char));
    	available_bal[0]='0';
    	available_bal[1]='\0';
    	return available_bal;
    }

}

// get balance related information of all "users" called from police mode
char * get_balance_all(){
	FILE *fp=fopen("user_login","r");
	char *retstr=(char *)malloc(3000*sizeof(char));
	if(fp == NULL){
		strcpy(retstr,"Could not open the user login file please terminate the connection\n");
		return retstr;
	}                   // internal error
	char * line = NULL;
    size_t len = 0;
    ssize_t read;
    
    retstr[0]='\0';

	while((read = getline(&line, &len, fp)) != -1) 
	{
		char *token=strtok(line," \n");
		char *token1=strtok(NULL," \n");
		char *token2=strtok(NULL," \n");
		if(token == NULL || token1 == NULL || token2 == NULL){
			strcat(retstr,"problem with this entry\n");
			continue;
		}
		if(token2[0]=='C')
		{
			strcat(retstr,token);
			strcat(retstr," ---- ");
			strcat(retstr,get_available_balance(token));
			strcat(retstr,"\n");
        }
    }

    return retstr;
}	
// mini statement of a particular user prints at max 8 transaction histories
char* get_mini_stat(char* username){
	FILE * fp = fopen(username,"r");
	char* mini_statement = (char*)malloc(10000*sizeof(char));
	if(fp == NULL){
		strcpy(mini_statement,"Could not find the file having your transaction history.\n");
		return mini_statement;
	}
	 mini_statement[0] = '\0';

	char * line = NULL;
    size_t len = 0;
    ssize_t read;

    int count_lines =0 ;   // prints at maximum 8 recent transaction history

    while(count_lines<lines_to_print_in_mini_stat && (read = getline(&line, &len, fp)) != -1){
		strcat(mini_statement,line);
		count_lines++;
	}

	fclose(fp);

	if(strlen(mini_statement) == 0){
		strcpy(mini_statement,"You have not participated in any transation yet.\n");
	}

	return mini_statement;

}

// open the transaction history file and add the recent transaction at the top the file
void update_user_balance(char *username,int transaction,double balance){
	FILE *fp=fopen(username,"r");
	char * line = NULL;
	char c=(transaction == 1)?'C':'D';
	char *line1=(char *)malloc(sizeof(char)*10000);
    size_t len = 0;
    ssize_t read;
	time_t ltime; /* calendar time */

	ltime=time(NULL); /* get current cal time */

	// using the systems time of the server to find when the transaction took place
	sprintf(line1,"%.*s %c %f\n",(int)strlen(asctime(localtime(&ltime)))-1,asctime(localtime(&ltime)),c,balance);

	while((read = getline(&line, &len, fp)) != -1)
		strcat(line1,line);
														// for easy access of the latest transaction history appending
														// at the top of the file
	fclose(fp);
	fp=fopen(username,"w");
	fwrite(line1, sizeof(char), strlen(line1), fp);
	fclose(fp);
}

// client query helps admin to either credit or debit from an account
int Client_Query(char *username, int client_fd){
	Msg_To_Client(client_fd,"\n1) Type `Credit` to credit balance to user account.\n2) Type `Debit` to debit balance from an account.\n3) Type `Terminate` to quit\n");
	int query_flag = -1;
	char* msg_from_client = NULL;

	while(1){  
		msg_from_client = Msg_From_Client(client_fd);
		if(msg_from_client == NULL){
			return EXIT;                                         // termination request from admin
		}
		else if(strncmp(msg_from_client,"Terminate",9)== 0){
			return EXIT;
		}
		else if(strncmp(msg_from_client,"Credit",6)== 0) {   // credit requests
			query_flag = 1;
		}
		else if(strncmp(msg_from_client,"Debit",5)== 0){
			query_flag = 2;                                  // debit requsests
		}
		else{
			query_flag = -1;
		}

		if(msg_from_client!= NULL)free(msg_from_client);

		char* temp = get_available_balance(username);
		if(strncmp(temp,"Could",5) == 0){
			return INTERNAL_ERROR;        // transaction history file is not found
		}
		double balance=strtod(temp,NULL);

		switch(query_flag) {
			case 1:
				Msg_To_Client(client_fd, "Enter Amount to be credited\n");
				while(1){
					msg_from_client = Msg_From_Client(client_fd);
					double amount = strtod(msg_from_client, NULL);

					if(amount<0){
						Msg_To_Client(client_fd, "Enter valid non negative amount\n"); // checking the balance is not negative
					}
					else {
						balance += amount; 
						update_user_balance(username, query_flag, balance);
						Msg_To_Client(client_fd, "Credit successful\n1) Type username of account holder to perform transactions.\n2) Type `Terminate` to quit\n");
	    				return SUCCESS;     // messages for next query 
					}
				}
			case 2:
				Msg_To_Client(client_fd, "Enter Amount to be debited\n");
				while(1){
					msg_from_client = Msg_From_Client(client_fd);
					double amount = strtod(msg_from_client, NULL);
										// messages for next query
					if(amount<0){
						Msg_To_Client(client_fd, "Enter valid amount\n"); // checking the balance is not negative
					}
					else if(amount > balance) {
						char* temp1 = (char*) malloc(150*sizeof(char));
						strcpy(temp1,"Insufficient Balance!\n Enter valid amount which is non negative and <= ");
						strcat(temp1,temp); //  checking there is enough money availble or not
						strcat(temp1," \n");
						Msg_To_Client(client_fd,temp1);
						if(temp1 != NULL) free(temp1);
					}							
					else {
						balance -= amount;
						update_user_balance(username, query_flag, balance);
						Msg_To_Client(client_fd, "Debit successful\n1) Type username of account holder to perform transactions.\n2) Type `Terminate` to quit\n");
	    				return SUCCESS;
					}							// messages for next query
				}
			default:
				Msg_To_Client(client_fd, "Unknown Query\n1) Type `Credit` to credit balance to an account.\n2) Type `Debit` to debit balance from an account.\n3) Type `Terminate` to quit\n");
		}
	}
}
// function handles requests of user mode
void requests_of_user(char* username,char* password,int client_fd){
		Msg_To_Client(client_fd,"You are logged in as user.\n1) Type `Balance` to see available balance of your account.\n2) Type `Mini_stat` to see mini statement of your account.\n3) Type `Terminate` to quit\n");
		int user_flag = -1;
		char* msg_from_client = NULL;
		while(1){                             // handling multiple request at one go 
			msg_from_client = Msg_From_Client(client_fd);
			if(msg_from_client == NULL){
				user_flag = 0;
				break;                                             // termination requset
			}
			else if(strncmp(msg_from_client,"Terminate",9)== 0){
				user_flag = 0;
				break;
			}
			else if(strncmp(msg_from_client,"Balance",7)== 0) {       // balance query
				user_flag = 1;
			}
			else if(strncmp(msg_from_client,"Mini_stat",18)== 0){      // mini statement query
				user_flag = 2;
			}
			else{
				user_flag = -1;
			}

			if(msg_from_client != NULL) free(msg_from_client);
			char* out_bal= (char*) malloc(1000*sizeof(char));
			char* out_mini_stat = (char*)malloc(10000*sizeof(char));

			strcpy(out_bal,"\nResponse: Available Balance ===============================\n\n");
			strcpy(out_mini_stat,"\nResponse: Mini Statement ===============================\n\n");

			switch(user_flag){
				case 1:
					strcat(out_bal,get_available_balance(username));
					Msg_To_Client(client_fd,strcat(out_bal,"\n ===============================================\n\nType your next Query or Type `Terminate` to quit.\n"));
					if(out_bal != NULL) free(out_bal);
					break;
				case 2:
			 		strcat(out_mini_stat,get_mini_stat(username));
					Msg_To_Client(client_fd,strcat(out_mini_stat,"\n=============================================\n\nType your next Query or Type `Terminate` to quit.\n"));
					if(out_mini_stat != NULL) free(out_mini_stat);
					break;
				default:
					Msg_To_Client(client_fd, "Unknown Query.\n1) Type `Balance` to see available balance of your account.\n2) Type `Mini_stat` to see mini statement of your account.\n3) Type `Terminate` to quit\n");
					break;
			}
		}

		close_socket_from_server_side(client_fd);  // gracefully close the connection after communication is over 


}

// handling requests of the admin
void requests_of_admin(int client_fd){
	Msg_To_Client(client_fd,"You are logged in as admin.\n1) Type username of account holder to perform transactions.\n2) Type `Terminate` to quit\n");

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
		else if(check_user(msg_from_client))
		{
			char *username=(char *)malloc(40*sizeof(char));
			strcpy(username,msg_from_client);

			int query_result = Client_Query(username, client_fd); // handles debits and credit from 
			if(query_result == INTERNAL_ERROR){ 
				close_socket_for_internal_error(client_fd);   // closing connection due to internal error at server side
				return;
			}	
			if(query_result == EXIT ) break;
		}
		else{
			Msg_To_Client(client_fd,"\n\nWrong Username. Please enter a valid user\n\n");
		}
	}
	close_socket_from_server_side(client_fd);	// gracefully close the connection after communication is over 

}

void requests_of_police(int client_fd){
	Msg_To_Client(client_fd,"You are logged in as police.\n1) Type `Balance_all` to see available balance of all users.\n2) Type `Mini_stat` to see mini statement of an account.\n3) Type `Terminate` to quit\n");
	int police_flag = -1;
	char* msg_from_client = NULL;
	while(1)
	{
		msg_from_client = Msg_From_Client(client_fd);

		if(msg_from_client == NULL){
			police_flag = 0;
			break;
		}
		else if(strncmp(msg_from_client,"Terminate",9)== 0){      // termination request
			police_flag = 0;
			break;
		}
		else if(strncmp(msg_from_client,"Balance_all",11)== 0) {
			police_flag = 1;                                            // balace of all users 
		}
		else if(strncmp(msg_from_client,"Mini_stat",9)== 0){
			police_flag = 2;
		}                                           // mini statement of a particular user 
		else{
			police_flag = -1;
		}

		if(msg_from_client != NULL) free(msg_from_client);

		char* out_bal = (char*) malloc(3000*sizeof(char));
		char* out_mini_stat = (char*)malloc(10000*sizeof(char));

		switch(police_flag){
			case 1:
				strcpy(out_bal,"\nResponse: balance of all user =======================\n");
				strcat(out_bal,get_balance_all());
				Msg_To_Client(client_fd,strcat(out_bal,"\n======================================\n\nType your next Query or Type `Terminate` to quit.\n"));
				if(out_bal != NULL) free(out_bal);
				break;
			case 2:
				Msg_To_Client(client_fd, "Enter username or Type `Terminate` to exit\n");
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
					else if(check_user(msg_from_client)) {
						char *username=(char *)malloc(sizeof(char)*40);
						strcpy(username,msg_from_client);
						strcpy(out_mini_stat,"\nResponse: Mini Statement of ");
						strcat(out_mini_stat,username);
						strcat(out_mini_stat," ===============================\n\n");
				 		strcat(out_mini_stat,get_mini_stat(username));
						Msg_To_Client(client_fd,strcat(out_mini_stat,"\n====================================\n\nType your next Query or Type `Terminate` to quit.\n"));
						if(out_mini_stat != NULL) free(out_mini_stat);
						if(username != NULL) free(username);
						if(msg_from_client != NULL) free(msg_from_client);
						break;
					}
					else{
						strcpy(out_mini_stat,"\n");
						strcat(out_mini_stat,"\nInvalid user.\nEnter username or Type `Terminate` to exit\n");
						Msg_To_Client(client_fd,out_mini_stat);
					}
				}
				break;
			default:
				Msg_To_Client(client_fd, "Unknown Query.\n1) Type `Balance_all` to see balance of all users.\n2) Type `Mini_stat` to see mini statement of an account.\n3) Type `Terminate` to quit\n");
				break;
		}
		if(police_flag == 0) {
			break;
		}
	}
	close_socket_from_server_side(client_fd);
}






// handle the client requests also closes the ongoing connection gracefully
void handling_client(int client_fd){

	   int authorisation = NO_USER;
	   int try =0;
	   char* username;
       char* password;
       char* msg_from_client;
	   while(1){
       		if(try){
       			msg_from_client = Msg_From_Client(client_fd);  // if its current clients 2nd try to login
       			if(is_terminate_msg(msg_from_client)){
       	 			close_socket_from_server_side(client_fd);return; // termination during login 
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
       		if(msg_from_client != NULL)free(msg_from_client);

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
       		if(msg_from_client != NULL) free(msg_from_client);

       		authorisation = authenticate_user(username,password);
       		printf("new user logged in with authorisation %d\n",authorisation );
       		try = 1;
       		if(authorisation == NO_USER){
       			Msg_To_Client(client_fd,"Unathorised Login.Type Terminate to terminate or any key to try again.\n");
       			if(username != NULL) free(username);
       			if(password != NULL) free(password);
       		}
       		else if(authorisation == INTERNAL_ERROR){
       			close_socket_for_internal_error(client_fd);  // closing socket due to internal error 
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
			    requests_of_admin(client_fd);
				break;
			case POLICE:
			    requests_of_police(client_fd);
				break;
			default:
				close_socket_for_internal_error(client_fd);
				break;
		}

}

int main(int argc, char **argv){

	int sock_fd,client_fd,port_no;
	struct sockaddr_in serv_addr, cli_addr;
	if(argc < 2){
		printf("Input the desired port no \n");  // check correct no of argument is passed 
		return -1;
	}

	memset((void*)&serv_addr, 0, sizeof(serv_addr));
	port_no=atoi(argv[1]);

	sock_fd=socket(AF_INET, SOCK_STREAM, 0);

	serv_addr.sin_port = htons(port_no);         //seting the port number passed as argument
	serv_addr.sin_family = AF_INET;              //setting DOMAIN
	serv_addr.sin_addr.s_addr = INADDR_ANY;      //permits any incoming IP

	if(bind(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
	    printf("Error on binding.\n");
	    return -1; 
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
	    switch(fork()){ // creating a new child process to handle each connection 
	        case -1:
	            printf("Error in fork.\n");
	            close_socket_for_internal_error(client_fd);
	            break;
	        case 0:
	            close(sock_fd); // in child process don't use original socket fd 
	            printf("%d accepted\n",client_fd);
	            handling_client(client_fd);
	            exit(EXIT_SUCCESS);
	            break;
	        default:
	            break;
	    }
	}

}
