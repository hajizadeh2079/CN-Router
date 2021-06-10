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

#define SIZE_LIMIT 16

using namespace std;


typedef struct message_buffer {
    long msg_type;
    char msg_text[1024];
} message_buffer;


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


void send_message(vector<string> &data, int key) {
    message_buffer msg_buff;
    int msgid = msgget(key, 0666 | IPC_CREAT);
    while (data[2].size() > SIZE_LIMIT) {
        string split_data = data[2].substr(0, SIZE_LIMIT);
        string msg = data[0] + ':' + data[1] + ':' + split_data + ":0";
        data[2].erase(0, SIZE_LIMIT);
        msg_buff.msg_type = 1;
        strcpy(msg_buff.msg_text, msg.c_str());
        msgsnd(msgid, &msg_buff, sizeof(msg_buff), 0);
    }
    string msg = data[0] + ':' + data[1] + ':' + data[2] + ":1";
    msg_buff.msg_type = 1;
    strcpy(msg_buff.msg_text, msg.c_str());
    msgsnd(msgid, &msg_buff, sizeof(msg_buff), 0);
}


void receive_message(int system_number, vector<string> &data, message_buffer &msg_buff, int msgid) {
    vector<string> all_data;
    int sender = atoi(data[0].c_str());
    all_data.push_back(data[2]);
    while (!atoi(data[3].c_str())) {
        msgrcv(msgid, &msg_buff, sizeof(msg_buff), 1, 0);
        string frame(msg_buff.msg_text);
        data = split_frame(frame);
        all_data.push_back(data[2]);
    }
    string msg = "";
    for (int i = 0; i < all_data.size(); i++)
        msg += all_data[i];
    cout << system_number << " receives " << msg << " from " << sender << endl;
}


int main(int argc, char const *argv[]) {
    int router_number = -1;
    int system_number = atoi(argv[0]);
    message_buffer msg_buff;
    int key = system_number + 1000;
    int msgid = msgget(key, 0666 | IPC_CREAT);
    while (true) {
        if (msgrcv(msgid, &msg_buff, sizeof(msg_buff), 1, IPC_NOWAIT) != -1) {
            string frame(msg_buff.msg_text);
            vector<string> data = split_frame(frame);
            int sender = atoi(data[0].c_str());
            int receiver = atoi(data[1].c_str());
            if (sender == 0)
                router_number = atoi(data[2].c_str());
            else {
                if (sender == system_number)
                    send_message(data, router_number);
                else if (receiver == system_number || receiver == 0)
                    receive_message(system_number, data, msg_buff, msgid);
            }
        }
    }
    exit(0);
}