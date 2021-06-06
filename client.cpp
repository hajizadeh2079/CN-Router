#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <dirent.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;


typedef struct message_buffer {
    long msg_type;
    char msg_text[1024];
} message_buffer;


vector<string> convert_string_to_vector(string str) {
    vector<string> temp;
    istringstream ss(str);
    string word;
    while(ss >> word)
        temp.push_back(word);
    return temp;
}


void handle_cmd(string cmd, string &ip) {
    vector<string> cmd_vector = convert_string_to_vector(cmd);
    if (cmd_vector[0] == "Set")
        ip = cmd_vector[2];
    else if (cmd_vector[0] == "Get");
    else if (cmd_vector[0] == "Join");
    else if (cmd_vector[0] == "Leave");
    else if (cmd_vector[0] == "Select");
    else if (cmd_vector[0] == "Send" && cmd_vector[1] == "file");
    else if (cmd_vector[0] == "Send" && cmd_vector[1] == "message");
    else if (cmd_vector[0] == "Show");
    else if (cmd_vector[0] == "Sync");
    else if (cmd_vector[0] == "Sign");
}


vector<string> split_frame(string str) {
    vector<string> v;
    stringstream ss(str);
    while (ss.good()) {
        string substr;
        getline(ss, substr, ':');
        v.push_back(substr);
    }
    return v;
}


void send_message(string msg, int key) {
    int msgid = msgget(key, 0666 | IPC_CREAT);
    message_buffer msg_buff;
    msg_buff.msg_type = 1;
    strcpy(msg_buff.msg_text, msg.c_str());
    msgsnd(msgid, &msg_buff, sizeof(msg_buff), 0);
}


int main(int argc, char const *argv[]) {
    string name = argv[1];
    string server_ip = argv[2];
    string router_ip = argv[3];
    int port = atoi(argv[4]);
    string ip;
    while (true) {
        string cmd;
        cin >> cmd;
        handle_cmd(cmd, ip);
    }
    exit(0);
}