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
#include <map>
#include <set>
#include <sstream>

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


void send_message(string msg, int key) {
    int msgid = msgget(key, 0666 | IPC_CREAT);
    message_buffer msg_buff;
    msg_buff.msg_type = 1;
    strcpy(msg_buff.msg_text, msg.c_str());
    msgsnd(msgid, &msg_buff, sizeof(msg_buff), 0);
}


void process_frame(vector<string> data, int *port_table, int *is_router, int number_of_ports, int router_number,map<string, set<int> >  &group_table) {
    int sender = atoi(data[0].c_str());
    int receiver = atoi(data[1].c_str());
    int key;
    string msg;
    if (sender == 0) {
        port_table[atoi(data[3].c_str())] = atoi(data[2].c_str());
        if (data.size() == 5)
            is_router[atoi(data[3].c_str())] = 1;
    }
    else if (receiver == -1) {
        for (int i = 0; i < number_of_ports; i++) {
            if (port_table[i] == sender) {
                group_table[data[2]].insert(i);
                break;
            }
        }
        msg = to_string(router_number) + ':' + data[1] + ':' + data[2] + ':' + data[3] + ':' + to_string(router_number);
        for (int i = 0; i < number_of_ports; i++) {
            if (is_router[i] == 1) {
                if (data.size() > 4 && port_table[i] == atoi(data[4].c_str()))
                    continue;
                key = port_table[i];
                send_message(msg, key);
            }
        }
    }
    else if (receiver == 0) {
        msg = data[0] + ':' + data[1] + ':' + data[2] + ':' + data[3] + ':' + to_string(router_number);
        for (auto i : group_table[data[1]]) {
            if (is_router[i] == 1) {
                if (data.size() > 4 && port_table[i] == atoi(data[4].c_str()))
                    continue;
                key = port_table[i];
                send_message(msg, key);
            }
            else {
                key = port_table[i] + 1000;
                send_message(msg, key);
            }
        }
    }
    else {
        bool find = false;
        for (int i = 0; i < number_of_ports; i++) {
            if (port_table[i] == receiver && is_router[i] == 0) {
                find = true;
                msg = data[0] + ':' + data[1] + ':' + data[2] + ':' + data[3];
                key = receiver + 1000;
                send_message(msg, key);
                break;
            }
        }
        if (!find) {
            msg = data[0] + ':' + data[1] + ':' + data[2] + ':' + data[3] + ':' + to_string(router_number);
            for (int i = 0; i < number_of_ports; i++) {
                if (is_router[i] == 1) {
                    if (data.size() > 4 && port_table[i] == atoi(data[4].c_str()))
                        continue;
                    key = port_table[i];
                    send_message(msg, key);
                }
            }
        }
    }
}


int main(int argc, char const *argv[]) {
    int number_of_ports = atoi(argv[0]);
    int router_number = atoi(argv[1]);
    int *port_table = (int*)malloc(number_of_ports * sizeof(int));
    int *is_router = (int*)malloc(number_of_ports * sizeof(int));
    map<string, set<int> > group_table;
    for (int i = 0; i < number_of_ports; i++) {
        port_table[i] = -1;
        is_router[i] = 0;
    }
    message_buffer msg_buff;
    int key = router_number;
    int msgid = msgget(key, 0666 | IPC_CREAT);
    while (true) {
        if (msgrcv(msgid, &msg_buff, sizeof(msg_buff), 1, IPC_NOWAIT) != -1) {
            string frame(msg_buff.msg_text);
            vector<string> data = split_frame(frame);
            process_frame(data, port_table, is_router, number_of_ports, router_number, group_table);
        }
    }
    exit(0);
}