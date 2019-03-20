#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
	printf("enter username\n");
	char* username = (char*)malloc(50*sizeof(char));
	char* password =  (char*)malloc(30*sizeof(char));
	scanf("%s",username);
	printf("enter password\n");
	scanf("%s",password);
	int mode;
	printf("enter mode 1 for user 2 for police 3 for admin\n");
	scanf("%d",&mode);
	char * user_login_entry = (char*)malloc(80 *sizeof(char));
	user_login_entry[0]='\0';
	strcat(user_login_entry,username);
	strcat(user_login_entry," ");
	strcat(user_login_entry,password);
	strcat(user_login_entry," ");

	if(mode== 1){
		strcat(user_login_entry,"C\n");
	}
	else if(mode == 2){
		strcat(user_login_entry,"P\n");
	}
	else if(mode == 3){
		strcat(user_login_entry,"A\n");
	}
	printf("%s\n",user_login_entry );

	FILE * fp = fopen("user_login","a");
	if(fp == NULL){
		printf("could not open user_login\n");
		return -1;
	}
	fprintf(fp, "%s", user_login_entry); 
	fclose(fp);
	if(mode == 1){
		FILE * fp1 = fopen(username,"a");
		if(fp1 != NULL ){fclose(fp1);}
		else{
			printf("could not create the file\n");
		}
	}
	return 0;
}