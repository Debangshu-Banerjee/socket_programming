#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int login_validation(char * username,char* password);

int login_validation(char * username, char * password){
	FILE * fp = fopen("user_login","r");
	if(fp == NULL ){
		perror("could not open user login file"); 
		return -1;
	}
	char single_line[1500];

	while(fgets(single_line,sizeof(single_line)-1, fp) != NULL){
		//printf("%s",single_line);

		char temp_username[300] ="";
		int length = strlen(single_line);
		int i=0;
		for(;i<length;i++){
			if(isspace(single_line[i])) break;
			temp_username[i]=single_line[i];
		}
		temp_username[i]='\0';


		if(strcmp(temp_username,username) != 0) continue;
		else{
			char temp_password[300]="";
			while(i<length && isspace(single_line[i])) i++;
			int j=0;
			for(;i<length;i++){
				if(isspace(single_line[i])) break;
				temp_password[j]=single_line[i];
				j++;
			}
			temp_password[j] = '\0';
			if(strcmp(temp_password,password) == 0){
				printf("successful login\n"); // -------------send through socket
				while(i<length && isspace(single_line[i])) i++;
				if(i>=length) {fclose(fp); return -1;}
				if(single_line[i] == 'C' ) {fclose(fp);return 0;}
				if(single_line[i] == 'A' ) {fclose(fp);return 1;}
				if(single_line[i] == 'P' ) {fclose(fp);return 2;}
				fclose(fp);
				return -1;
			}

		}

	}
	fclose(fp);
	return -1;

} 
int main(){
  char username[100];
  char password[100];
  scanf("%s %s",username,password);
  int x=login_validation(username,password);

  printf("%d\n",x);

}