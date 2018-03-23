/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:

**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>
#include "dialog.h"
#include <time.h>
#include <curses.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>


//char * host = "localhost";
char host[100] = "localhost";
char * user;
char * password;
char * sport;
char * roomn;
char * romn;

char * mesg;

//char roomname[100] = "room1";

char * u1;
char * p1;

int havelogin = 1;
int port;

QTextEdit * usertext;
QTextEdit * passtext;
QTextEdit * roomtext;

QListWidget * roomsList;
QListWidget * usersList;

#define MAX_MESSAGES 100
#define MAX_MESSAGE_LEN 300
#define MAX_RESPONSE (20 * 1024)

int lastMessage = 0;

int open_client_socket(char * host, int port) {
    // Initialize socket address structure
    struct  sockaddr_in socketAddress;

    // Clear sockaddr structure
    memset((char *)&socketAddress,0,sizeof(socketAddress));

    // Set family to Internet
    socketAddress.sin_family = AF_INET;

    // Set port
    socketAddress.sin_port = htons((u_short)port);

    // Get host table entry for this host
    struct  hostent  *ptrh = gethostbyname(host);
    if ( ptrh == NULL ) {
        perror("gethostbyname");
        exit(1);
    }

    // Copy the host ip address to socket address structure
    memcpy(&socketAddress.sin_addr, ptrh->h_addr, ptrh->h_length);

    // Get TCP transport protocol entry
    struct  protoent *ptrp = getprotobyname("tcp");
    if ( ptrp == NULL ) {
        perror("getprotobyname");
        exit(1);
    }

    // Create a tcp socket
    int sock = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    // Connect the socket to the specified server
    if (connect(sock, (struct sockaddr *)&socketAddress,
            sizeof(socketAddress)) < 0) {
        perror("connect");
        exit(1);
    }

    return sock;
}

int sendCommand(char* host, int port, char* command, char* user,
        char* password, char* args, char* response) {
    int sock = open_client_socket( host, port);



    // Send command
    write(sock, command, strlen(command));
    write(sock, " ", 1);
    write(sock, user, strlen(user));
    write(sock, " ", 1);
    write(sock, password, strlen(password));
    write(sock, " ", 1);
    write(sock, args, strlen(args));
    write(sock, "\r\n",2);

    // Keep reading until connection is closed or MAX_REPONSE
    int n = 0;
    int len = 0;
    while ((n=read(sock, response+len, MAX_RESPONSE - len))>0) {
        len += n;
    }

    //printf("response:%s\n", response);

    close(sock);
}

void printUsage()
{
    printf("Usage: talk-client host port user password\n");
    exit(1);
}

void login() {
    char allroom[ MAX_RESPONSE ];
    char lmess[MAX_RESPONSE];
    QString userstring = usertext->toPlainText();
    QByteArray userba = userstring.toLatin1();
    user = strdup(userba.data());
    QString passstring = passtext->toPlainText();
    QByteArray passba = passstring.toLatin1();
    password = strdup(passba.data());

    roomsList->clear();
    usersList->clear();

    sendCommand(host, 9711, "LOGIN", user, password, "", lmess);
    sendCommand(host, 9711, "LIST-ROOMS", user, password, "", allroom);

    printf("%s\n", allroom);
    if (strstr(lmess,"OK")!=NULL) {
        havelogin = 0;
        //usertext->setText("");
        //passtext->setText("");
        QMessageBox * qm = new QMessageBox();
        qm->setText("Login success!");
        qm->exec();
        if (strstr(allroom, "ERROR") != NULL || strstr(allroom, "D") != NULL) {

        } else {

            char * str = strtok(allroom, "\r\n");
            //printf("%s\n", str);
            roomsList->addItem(str);
            int ct = 0;
            while (str != NULL) {
                str = strtok(NULL, "\r\n");
                roomsList->addItem(str);
            }
        }

    } else {
        QMessageBox * qm = new QMessageBox();
        qm->setText("Your password or username wrong!");
        qm->exec();
    }

    //free(user);
    //free(password);

}

void add_user() {
    // Try first to add user in case it does not exist.
    char response[ MAX_RESPONSE ];
    char allroom[MAX_MESSAGES];
    QString userstring = usertext->toPlainText();
    QByteArray userba = userstring.toLatin1();
    user = strdup(userba.data());
    QString passstring = passtext->toPlainText();
    QByteArray passba = passstring.toLatin1();
    password = strdup(passba.data());
    //printf("%s\n", user);
    if (strlen(user) < 1 || strlen(password) < 1) {
        QMessageBox * qm = new QMessageBox();
        qm->setText("Username and password must greater than 1 character!");
        qm->exec();
        return;
    }
    sendCommand(host, 9711, "ADD-USER", user, password, "", response);

    sendCommand(host, 9711, "LIST-ROOMS", user, password, "", allroom);

    if (strstr(response,"OK")!=NULL) {
        havelogin = 0;
        //usertext->setText("");
        //passtext->setText("");
        QMessageBox * qm = new QMessageBox();
        qm->setText("Create success! Login success!");
        qm->exec();

        if (strstr(allroom, "ERROR") != NULL || strstr(allroom, "D") != NULL) {

        } else {

            char * str = strtok(allroom, "\r\n");
            //printf("%s\n", str);
            roomsList->addItem(str);
            int ct = 0;
            while (str != NULL) {
                str = strtok(NULL, "\r\n");
                roomsList->addItem(str);
            }
        }

    } else {
        QMessageBox * qm = new QMessageBox();
        qm->setText("User already exist!");
        qm->exec();
    }
    //free(user);
    //free(password);
}

int j = 1;
void create_room() {
    char response[ MAX_RESPONSE ];
    /*QString userstring = usertext->toPlainText();
    QByteArray userba = userstring.toLatin1();
    user = strdup(userba.data());
    QString passstring = passtext->toPlainText();
    QByteArray passba = passstring.toLatin1();
    password = strdup(passba.data());*/

    if (havelogin) {
        QMessageBox * qm = new QMessageBox();
        qm->setText("Please Login First!");
        qm->exec();
        return;
    }
    QString roomstring = roomtext->toPlainText();
    QByteArray roomba = roomstring.toLatin1();
    roomn = strdup(roomba.data());

    //char * s = strcpy(roomn);
    if (strlen(roomn) < 2) {
        QMessageBox * qm = new QMessageBox();
        qm->setText("Room name must larger than 1 character!");
        qm->exec();
        return;
    }

    //char s[100];
    //sprintf(s,"room%d", j);
    sendCommand(host, 9711, "CREATE-ROOM", user, password, roomn, response);


    if (strstr(response, "OK") != NULL && strlen(roomn) > 1) {
        roomsList->addItem(roomn);
        roomtext->setText("");
    } else {
        QMessageBox * qm = new QMessageBox();
        qm->setText("Room name duplicated or nothing input!");
        qm->exec();
    }
    //free(user);
    //free(password);
}

void enter_room(QListWidgetItem* item) {
    if (havelogin) {
        QMessageBox * qm = new QMessageBox();
        qm->setText("Please Login First!");
        qm->exec();
        return;
    }
    char er[MAX_RESPONSE];
    char ur[MAX_RESPONSE];
    //char mr[MAX_MESSAGES];
    char response[MAX_RESPONSE];
    int i = 0;

    /*QString userstring = usertext->toPlainText();
    QByteArray userba = userstring.toLatin1();
    user = strdup(userba.data());
    QString passstring = passtext->toPlainText();
    QByteArray passba = passstring.toLatin1();
    password = strdup(passba.data());*/


    while (true) {
        if (roomsList->item(i) == item) {


            usersList->clear();
            QString ron = item->text();
            QByteArray rba = ron.toLatin1();
            romn = strdup(rba.data());

            sendCommand(host, 9711, "ENTER-ROOM", user, password, romn, er);
            //QMessageBox * qm = new QMessageBox();
            //qm->setText("Enter Success!");
            //qm->exec();
            sendCommand(host, 9711, "GET-USERS-IN-ROOM", user, password, romn, ur);
            char * str = strtok(ur, "\r\n");
            usersList->addItem(str);
            int ct = 0;
            while (str != NULL) {
                str = strtok(NULL, "\r\n");
                usersList->addItem(str);

            }

            char * take = new char[999];
            strcpy(take, romn);
            strcat(take, " ");
            strcat(take, user);
            strcat(take, " enter the room!");
            //printf("%s\n", take);

            sendCommand(host, 9711, "SEND-MESSAGE", user, password, take, response);
            //printf("%s\n", user);
            //printf("%s\n", response);

            break;
        }
        i++;
    }

    //free(user);
    //free(password);
}

void leave_room() {
}

void get_messages() {

}

void send_message(char * msg) {

}

void print_users_in_room() {
}

void print_users() {
}

void printPrompt() {
    printf("talk> ");
    fflush(stdout);
}

void printHelp() {
    printf("Commands:\n");
    printf(" -who   - Gets users in room\n");
    printf(" -users - Prints all registered users\n");
    printf(" -help  - Prints this help\n");
    printf(" -quit  - Leaves the room\n");
    printf("Anything that does not start with \"-\" will be a message to the chat room\n");
}

void * getMessagesThread(void * arg) {
    // This code will be executed simultaneously with main()
    // Get messages to get last message number. Discard the initial Messages

    while (1) {
        // Get messages after last message number received.

        // Print messages

        // Sleep for ten seconds
        usleep(2*1000*1000);
    }
}

void startGetMessageThread()
{
    pthread_create(NULL, NULL, getMessagesThread, NULL);
}




void Dialog::loginaction() {
    login();
}

void Dialog::sendAction()
{
    if (havelogin) {
        QMessageBox * qm = new QMessageBox();
        qm->setText("Please Login First!");
        qm->exec();
        return;
    }
    printf("Send Button\n");
    QString imstring = inputMessage->toPlainText();
    QByteArray imba = imstring.toLatin1();
    mesg = strdup(imba.data());
    char * take = new char[999];
    strcpy(take, romn);
    strcat(take, " ");
    strcat(take, mesg);
    printf("%s\n", take);

    char response[ MAX_RESPONSE ];

    /*QString userstring = usertext->toPlainText();
    QByteArray userba = userstring.toLatin1();
    user = strdup(userba.data());
    QString passstring = passtext->toPlainText();
    QByteArray passba = passstring.toLatin1();
    password = strdup(passba.data());*/

    sendCommand(host, 9711, "SEND-MESSAGE", user, password, take, response);

    inputMessage->setText("");
    if (strstr(response, "OK") != NULL) {
        //allMessages->append(mesg);
    } else {
        QMessageBox * qm = new QMessageBox();
        qm->setText("Something wrong??");
        qm->exec();
    }

    free(take);
}

void Dialog::newUserAction()
{
    add_user();
}

void Dialog::enteraction(QListWidgetItem* item) {
    allMessages->clear();
    enter_room(item);
}

void Dialog::leaveaction() {
    if (havelogin) {
        QMessageBox * qm = new QMessageBox();
        qm->setText("Please Login First!");
        qm->exec();
        return;
    }
    char response[MAX_MESSAGES];
    char sendr[MAX_MESSAGES];

    char * take = new char[999];
    strcpy(take, romn);
    strcat(take, " ");
    strcat(take, user);
    strcat(take, " has leave the room.");
    printf("%s\n", take);
    sendCommand(host, 9711, "SEND-MESSAGE", user, password, take, sendr);

    sendCommand(host, 9711, "LEAVE-ROOM", user, password, romn, response);

    if (strstr(response, "OK") != NULL) {
        QMessageBox * qm = new QMessageBox();
        qm->setText("You have leave!");
        qm->exec();

        romn = NULL;
        usersList->clear();
        allMessages->clear();

    } else {
        QMessageBox * qm = new QMessageBox();
        qm->setText("Leave fail??");
        qm->exec();
    }
}

void Dialog::newroomaction() {
    create_room();
}


int k = -1;
void Dialog::timerAction()
{

    char allm[ MAX_RESPONSE ];
    char allu[MAX_RESPONSE];
    char allroom[MAX_RESPONSE];

    char * take = new char[999];
    if (!havelogin) {

        /*sendCommand(host, 9711, "LIST-ROOMS", user, password, "", allroom);

        if (strstr(allroom, "ERROR") != NULL || strstr(allroom, "D") != NULL) {

        } else {
            roomsList->clear();
            char * str = strtok(allroom, "\r\n");
            //printf("%s\n", str);
            roomsList->addItem(str);
            int ct = 0;
            while (str != NULL) {
                str = strtok(NULL, "\r\n");
                roomsList->addItem(str);
            }
        }*/

        if (romn != NULL) {
            strcpy(take, "-1");
            strcat(take, " ");
            strcat(take, romn);

            //printf("%s\n", take);
            sendCommand(host, 9711, "GET-MESSAGES", user, password, take, allm);

            //char * str = strtok(allm, "\r\n");

            if (strstr(allm, "ERROR") != NULL || strstr(allm, "NO") != NULL) {

            } else {
                allMessages->clear();
                allMessages->append(allm);
                /*while (str != NULL) {
                    str = strtok(NULL, "\r\n");
                    allMessages->append(str);
                }*/
            }

            sendCommand(host, 9711, "GET-USERS-IN-ROOM", user, password, romn, allu);

            usersList->clear();
            char * s = strtok(allu, "\r\n");
            usersList->addItem(s);

            while (s != NULL) {
                s = strtok(NULL, "\r\n");
                usersList->addItem(s);

            }


        }
    }




    //free(take);

    //printf("Timer wakeup\n");
    //messageCount++;

    //char message[50];
    //sprintf(message,"Timer Refresh New message %d",messageCount);
    //allMessages->append(message);
}

Dialog::Dialog()
{
    createMenu();

    QVBoxLayout *mainLayout = new QVBoxLayout;

    // Rooms List
    QVBoxLayout * roomsLayout = new QVBoxLayout();
    QLabel * roomsLabel = new QLabel("Rooms");
    roomsList = new QListWidget();
    roomsLayout->addWidget(roomsLabel);
    roomsLayout->addWidget(roomsList);

    // Users List
    QVBoxLayout * usersLayout = new QVBoxLayout();
    QLabel * usersLabel = new QLabel("Users");
    usersList = new QListWidget();
    usersLayout->addWidget(usersLabel);
    usersLayout->addWidget(usersList);

    // Layout for rooms and users
    QHBoxLayout *layoutRoomsUsers = new QHBoxLayout;
    layoutRoomsUsers->addLayout(roomsLayout);
    layoutRoomsUsers->addLayout(usersLayout);

    // Textbox for all messages
    QVBoxLayout * allMessagesLayout = new QVBoxLayout();
    QLabel * allMessagesLabel = new QLabel("Messages");
    allMessages = new QTextEdit;
    allMessagesLayout->addWidget(allMessagesLabel);
    allMessagesLayout->addWidget(allMessages);

    // Textbox for input message
    QVBoxLayout * inputMessagesLayout = new QVBoxLayout();
    QLabel * inputMessagesLabel = new QLabel("Type your message:");
    inputMessage = new QTextEdit;
    inputMessagesLayout->addWidget(inputMessagesLabel);
    inputMessagesLayout->addWidget(inputMessage);

    //  textbox for username and password
    //QWidget win;
    //win.setFixedSize(50, 50);
    QHBoxLayout * ulayout = new QHBoxLayout();

    QHBoxLayout * playout = new QHBoxLayout();
    QHBoxLayout * roomname = new QHBoxLayout();
    QLabel * ul = new QLabel("Username:");
    QLabel * pl = new QLabel("Password:");
    QLabel * rl = new QLabel("Input your room name if you want to create!");
    usertext = new QTextEdit;
    usertext->setFixedHeight(30);
    passtext = new QTextEdit;
    passtext->setFixedHeight(30);
    roomtext = new QTextEdit;
    roomtext->setFixedHeight(30);

    ulayout->addWidget(ul);
    ulayout->addWidget(usertext);
    playout->addWidget(pl);
    playout->addWidget(passtext);
    roomname->addWidget(rl);
    roomname->addWidget(roomtext);

    // Send and new account buttons
    QHBoxLayout * layoutButtons = new QHBoxLayout;
    QPushButton * sendButton = new QPushButton("Send");
    sendButton->setStyleSheet("background-color: red");
    QPushButton * newUserButton = new QPushButton("Create Account and Login");
    newUserButton->setStyleSheet("background-color: blue");
    QPushButton * newroombutton = new QPushButton("Create Room");
    newroombutton->setStyleSheet("background-color: orange");
    QPushButton * loginbutton = new QPushButton("Login");
    loginbutton->setStyleSheet("background-color: yellow");
    QPushButton * leroom = new QPushButton("Leave");
    leroom->setStyleSheet("background-color: gray");
    layoutButtons->addWidget(sendButton);
    layoutButtons->addWidget(newUserButton);
    layoutButtons->addWidget(newroombutton);
    layoutButtons->addWidget(loginbutton);
    layoutButtons->addWidget(leroom);
    // Setup actions for buttons
    connect(sendButton, SIGNAL (clicked(bool)), this, SLOT (sendAction()));
    connect(newUserButton, SIGNAL (clicked(bool)), this, SLOT (newUserAction()));
    connect(newroombutton, SIGNAL(clicked(bool)), this, SLOT(newroomaction()));
    connect(loginbutton, SIGNAL(clicked(bool)),this, SLOT(loginaction()));
    connect(roomsList,SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(enteraction(QListWidgetItem*)));
    connect(leroom, SIGNAL(clicked(bool)), this, SLOT(leaveaction()));

    // Add all widgets to window
    mainLayout->addLayout(layoutRoomsUsers);
    mainLayout->addLayout(allMessagesLayout);
    mainLayout->addLayout(inputMessagesLayout);
    mainLayout->addLayout(ulayout);
    mainLayout->addLayout(playout);
    mainLayout->addLayout(roomname);
    mainLayout->addLayout(layoutButtons);



    // Populate rooms
    /*for (int i = 0; i < 20; i++) {
        char s[50];
        sprintf(s,"Room %d", i);
        roomsList->addItem(s);
    }

    // Populate users
    for (int i = 0; i < 20; i++) {
        char s[50];
        sprintf(s,"User %d", i);
        usersList->addItem(s);
    }

    for (int i = 0; i < 20; i++) {
        char s[50];
        sprintf(s,"Message %d", i);
        allMessages->append(s);
    }*/

    // Add layout to main window
    setLayout(mainLayout);

    setWindowTitle(tr("CS240 IRC Client"));
    //timer->setInterval(5000);

    messageCount = 0;

    timer = new QTimer(this);
    connect(timer, SIGNAL (timeout()), this, SLOT (timerAction()));
    timer->start(3000);
}


void Dialog::createMenu()

{
    menuBar = new QMenuBar;
    fileMenu = new QMenu(tr("&File"), this);
    exitAction = fileMenu->addAction(tr("E&xit"));
    menuBar->addMenu(fileMenu);

    connect(exitAction, SIGNAL(triggered()), this, SLOT(accept()));
}
