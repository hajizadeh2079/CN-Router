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


class Console {
public:
    void run() {
        string str;
        while (true) {
            getline(cin, str);
            cmd = convert_string_to_vector(str);
            process_cmd();
        }
    }

    void process_cmd() {
        if (cmd[0] == "MyRouter")
            create_router(atoi(cmd[1].c_str()), atoi(cmd[2].c_str()));
        if (cmd[0] == "MySystem")
            create_system(atoi(cmd[1].c_str()));
        if (cmd[0] == "Connect")
            connect_system_router(atoi(cmd[1].c_str()), atoi(cmd[2].c_str()), atoi(cmd[3].c_str()));
        if (cmd[0] == "Connect-R")
            connect_router_router(atoi(cmd[1].c_str()), atoi(cmd[2].c_str()), atoi(cmd[3].c_str()), atoi(cmd[4].c_str()));
        if (cmd[0] == "Send") {
            string data = "";
            for (int i = 3; i < cmd.size(); i++) {
                data += cmd[i];
                if (i != cmd.size() - 1)
                    data += " ";
            }
            send_system_system(atoi(cmd[1].c_str()), atoi(cmd[2].c_str()), data);
        }
        if (cmd[0] == "Send-M") {
            string data = "";
            for (int i = 3; i < cmd.size(); i++) {
                data += cmd[i];
                if (i != cmd.size() - 1)
                    data += " ";
            }
            send_multicast(atoi(cmd[1].c_str()), cmd[2], data);
        }
        if (cmd[0] == "Join") {
            join_group(atoi(cmd[1].c_str()), cmd[2]);
        }
    }

    void join_group(int system_number, string group_name) {
        message_buffer msg_buff;
        int key = system_number + 1000;
        int msgid = msgget(key, 0666 | IPC_CREAT);
        msg_buff.msg_type = 1;
        string msg = to_string(system_number) + ':' + "-1" + ':' + group_name;
        strcpy(msg_buff.msg_text, msg.c_str());
        msgsnd(msgid, &msg_buff, sizeof(msg_buff), 0);
    }

    void send_system_system(int sender, int receiver, string data) {
        message_buffer msg_buff;
        int key = sender + 1000;
        int msgid = msgget(key, 0666 | IPC_CREAT);
        msg_buff.msg_type = 1;
        string msg = to_string(sender) + ':' + to_string(receiver) + ':' + data;
        strcpy(msg_buff.msg_text, msg.c_str());
        msgsnd(msgid, &msg_buff, sizeof(msg_buff), 0);
    }

    void send_multicast(int sender, string group_name, string data) {
        message_buffer msg_buff;
        int key = sender + 1000;
        int msgid = msgget(key, 0666 | IPC_CREAT);
        msg_buff.msg_type = 1;
        string msg = to_string(sender) + ':' + group_name + ':' + data;
        strcpy(msg_buff.msg_text, msg.c_str());
        msgsnd(msgid, &msg_buff, sizeof(msg_buff), 0);
    }

    void connect_system_router(int system_number, int router_number, int port_number) {
        message_buffer msg_buff;
        
        int key = system_number + 1000;
        int msgid = msgget(key, 0666 | IPC_CREAT);
        msg_buff.msg_type = 1;
        string msg = "0:" + to_string(system_number) + ':' + to_string(router_number);
        strcpy(msg_buff.msg_text, msg.c_str());
        msgsnd(msgid, &msg_buff, sizeof(msg_buff), 0);

        key = router_number;
        msgid = msgget(key, 0666 | IPC_CREAT);
        msg_buff.msg_type = 1;
        msg = "0:" + to_string(router_number) + ':' + to_string(system_number) + ':' + to_string(port_number);
        strcpy(msg_buff.msg_text, msg.c_str());
        msgsnd(msgid, &msg_buff, sizeof(msg_buff), 0);
    }

    void connect_router_router(int router_number1, int port_number1, int router_number2, int port_number2) {
        message_buffer msg_buff;
        
        int key = router_number1;
        int msgid = msgget(key, 0666 | IPC_CREAT);
        msg_buff.msg_type = 1;
        string msg = "0:" + to_string(router_number1) + ':' + to_string(router_number2) + ':' + to_string(port_number1) + ":Sw";
        strcpy(msg_buff.msg_text, msg.c_str());
        msgsnd(msgid, &msg_buff, sizeof(msg_buff), 0);

        key = router_number2;
        msgid = msgget(key, 0666 | IPC_CREAT);
        msg_buff.msg_type = 1;
        msg = "0:" + to_string(router_number2) + ':' + to_string(router_number1) + ':' + to_string(port_number2) + ":Sw";
        strcpy(msg_buff.msg_text, msg.c_str());
        msgsnd(msgid, &msg_buff, sizeof(msg_buff), 0);
    }

    void create_router(int number_of_ports, int router_number) {
        if (fork() == 0) {
            char num_of_ports[16], router_no[16];
            sprintf(num_of_ports, "%d", number_of_ports);
            sprintf(router_no, "%d", router_number);
            char *args[]={num_of_ports, router_no, NULL};
            execvp("./router.out", args);
        }
    }

    void create_system(int system_number) {
        if (fork() == 0) {
            char sys_no[16];
            sprintf(sys_no, "%d", system_number);
            char *args[]={sys_no, NULL};
            execvp("./system.out", args);
        }
    }

    vector<string> convert_string_to_vector(string str) {
        vector<string> temp;
        istringstream ss(str);
        string word;
        while(ss >> word)
            temp.push_back(word);
        return temp;
    }
private:
    vector<string> cmd;
};


int main(int argc, char *argv[]) {
    system("ipcrm --all=msg");
    Console console;
    console.run();
    return 0;
}