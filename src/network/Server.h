/*
    Cinnamon UCI chess engine
    Copyright (C) Giuseppe Cannella

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once


#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../blockingThreadPool/Thread.h"
#include "Iparser.h"
#include "../perft/remote/Message.h"


class Server : public Thread {
public:
    static const int MAX_MSG_SIZE = 2048;

    Server(int port, Iparser *parser);

    void run();

    void endRun();

    ~Server();

    void sendMsg(string msg) {
        Message m("msg");
        string s=m.getSerializedString();
        write(client_sock, s.c_str(), strlen(s.c_str()) + 1);
    }

private:
    int client_sock;
    int sockfd = -1;

    int socket_desc;

    void dispose();

    Iparser *parser;
};
