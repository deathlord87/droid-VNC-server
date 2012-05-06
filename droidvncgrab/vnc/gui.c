/*
droid VNC server  - a vnc server for android
Copyright (C) 2011 Jose Pereira <onaips@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "gui.h"

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         1024
#define QUEUE_SIZE          1

int hServerSocket;  /* handle to socket */
struct sockaddr_in Address; /* Internet socket address stuct */
int nAddressSize=sizeof(struct sockaddr_in);
char pBuffer[BUFFER_SIZE];
static int nHostPort;

int sendMsgToGui(char *buffer)
{
   int sock, n;
   unsigned int length;
   struct sockaddr_in server;
   
   sock= socket(AF_INET, SOCK_DGRAM, 0);
   if (sock < 0) perror("socket");

   bzero(&server,sizeof(server));
   server.sin_family = AF_INET;
   server.sin_addr.s_addr=inet_addr("127.0.0.1");
   server.sin_port = htons(13131);
   length=sizeof(struct sockaddr_in);

   n=sendto(sock,buffer,
            strlen(buffer),0,(struct sockaddr *)&server,length);
   if (n < 0) perror("Sendto");
   
//    L("Sent %s\n",buffer);
// n = recvfrom(sock,buffer,256,0,(struct sockaddr *)&from, &length);
// if (n < 0) error("recvfrom");
// write(1,"Got an ack: ",12);
// write(1,buffer,n);
   
   close(sock);
   return 0;
}  
  
int bindIPCserver()
{
    nHostPort=13132;

    L("Starting IPC connection...");

    /* make a socket */
    hServerSocket=socket(AF_INET,SOCK_DGRAM,0);

    if(hServerSocket == SOCKET_ERROR)
    {
        L("\nCould not make a socket\n");
        return 0;
    }

    /* fill address struct */
    Address.sin_addr.s_addr=INADDR_ANY;
    Address.sin_port=htons(nHostPort);
    Address.sin_family=AF_INET;


    /* bind to a port */
    if(bind(hServerSocket,(struct sockaddr*)&Address,sizeof(Address)) == SOCKET_ERROR)
    {
       L("\nCould not connect to IPC gui, another daemon already running?\n");
       sendMsgToGui("~SHOW|Could not connect to IPC gui, another daemon already running?\n");

        exit(-1);
    }


    L("binded to port %d\n",nHostPort);

    pthread_t thread;
    pthread_create( &thread,NULL,handle_connections,NULL);
 
  return 1;
}



void *handle_connections()
{
   L("\nWaiting for a connection\n");
   struct sockaddr_in from;
   int fromlen = sizeof(struct sockaddr_in);
   int n;
   
   while (1) {
       n = recvfrom(hServerSocket,pBuffer,BUFFER_SIZE,0,(struct sockaddr *)&from,&fromlen);
       if (n < 0) perror("recvfrom");
    
       L("Recebido: %s\n",pBuffer);
       
       if (strstr(pBuffer,"~PING|")!=NULL)
       {
	  char *resp="~PONG|";
	  n = sendto(hServerSocket,resp,strlen(resp),
                  0,(struct sockaddr *)&from,fromlen);
	  if (n  < 0) perror("sendto");
        }
        else if (strstr(pBuffer,"~KILL|")!=NULL)
	  close_app();
   }
}


void unbindIPCserver()
{
  close(hServerSocket);
}