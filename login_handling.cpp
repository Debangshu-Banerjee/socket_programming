#include<bits/stdc++.h>
#include<fstream>

using namespace std;

int login(string username,string password);
int detect_mode(string mode);

int detect_mode(string mode){
	int len = (int)mode.size();
	int count = 0;
	vector<int> modes;
	vector<string> mode_names;
	for(int i=0;i<len;i++){
		if(mode[i] == 'C'){ modes.push_back(0);mode_names.push_back("C for coustomer\n"); count++;}
		if(mode[i] == 'A'){ modes.push_back(1);mode_names.push_back("A for admin\n"); count++;}
		if(mode[i] == 'P'){ modes.push_back(2);mode_names.push_back("P for police\n"); count++;}
	}
	if(count <=0) {cout<<"Internal error\n"; return -1;}
	if(count == 1) return modes[0];
	else{
		cout<<"multiple modes are available for you type in one of the following\n";
		for(int i=0;i< (int)mode_names.size();i++) cout << mode_names[i]<< endl;
		char temp;
		cin>> temp;
		if(temp == 'C') return 0;
		if(temp == 'A') return 1;
		if(temp == 'P') return 2;	
	}
}

// returns mode of the user
int login(string username,string password){
	fstream login_file;
	string filename = "user_login";
	string word;
	login_file.open(filename.c_str());
	if(!login_file.is_open()){
		cout<<"Error: problem in opening login file\n";
		return -1; // error occurred
	}	
	bool valid_username = false;
	while(login_file >> word){
		if(valid_username){
			if(word == password){
				string mode;
				login_file >> mode;
				int detected_mode = detect_mode(mode);
				if(detected_mode >= 0 ) cout << "successful login\n";
				return detected_mode;
			}
			else{
				cout<<"Error: incorrect password\n";
				return -1;// invalid password error occurred
			}
		}
		if(word == username){
			valid_username = true;			
		}

	} 
	if(valid_username == false){
		cout<< "Error: no entries found with the given username\n";
		return -1; // invalid username error occurred
	}
}

int main(){
		string username,password;
		cin >> username >> password;
		cout<<login(username,password)<< endl;

}