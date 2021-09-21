/* Server code in C */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <thread>
#include <vector>
#include <fcntl.h>
#include <map> 
#include <iterator>

using namespace std;
 
//vector<int> clients;
map<string, int> clients;

/*
void Broadcast(int ConnectFD, string message) 
{
  //write(ConnectFD, message.c_str(),message.length());
  
  for (int i = 0; i < clients.size(); i++) {
    write(clients[i], message.c_str(),message.length());
  }
  

}
*/

void Connection( int &socketSer , int port);
void listenClient(int ConnectFD);
void readError(int n);

int main(void)
{
    int socketSer;
    char buffer[256];
    int n, tam;
    cout << "INICIALIZANDO SERVIDOR ..." << endl;
    Connection( socketSer, 55000);
    cout << "CONEXION INICIALIZADA ..." << endl;
    cout << "Listo, esperando clientes..." << endl;
    
    while(1)
    {
      int ConnectFD = accept(socketSer, NULL, NULL);
      cout << "Cliente " << ConnectFD << " solicita conexion al servidor." << endl;
      if(0 > ConnectFD)
      {
        perror("error accept failed");
        close(socketSer);
        exit(EXIT_FAILURE);
      }
      else
      {
        bzero(buffer,256);
        n = read(ConnectFD, buffer, 1);
        if (n < 0) {
          perror("ERROR reading from socket");
          exit(0);
        }
        else if (buffer[0] == 'R')
        {
          n = read(ConnectFD, buffer, 2);
          if (n < 0) {
            perror("ERROR reading from socket");
            exit(0);
          }
          else if (n == 2)
          {
            buffer[2] = '\0';
            tam = atoi(buffer);
            n = read(ConnectFD, buffer, tam);
            buffer[n] = '\0';
            clients.insert(pair<string, int>(string(buffer),ConnectFD));
            cout << "Cliente " << ConnectFD << " con nick \"" << string(buffer) << "\" registrado con exito." << endl;
          }
          else
            cout << "Protocolo no valido ..." << endl;
        }
      }
      //clients.push_back(ConnectFD);
      thread(listenClient, ConnectFD).detach();
    }
    close(socketSer);
    cout << "Bye" << endl;
    return 0;
}

void Connection( int &socketSer , int port)
{
    struct sockaddr_in stSockAddr;
    socketSer = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if(-1 == socketSer)
    {
      perror("can not create socket");
      exit(EXIT_FAILURE);
    }
    
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
   
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(port);
    stSockAddr.sin_addr.s_addr = INADDR_ANY;
 
    if(-1 == bind(socketSer,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
    {
      perror("error bind failed");
      close(socketSer);
      exit(EXIT_FAILURE);
    }
    
    if(-1 == listen(socketSer, 10))
    {
      perror("error listen failed");
      close(socketSer);
      exit(EXIT_FAILURE);
    }
   
}

void listenClient(int ConnectFD)
{
    bool active = true;
    char buffer[256];
    int n, tam, connD;
    map<string, int>::iterator itr;

    string msg, tmp, nickD;
    while(active)
    {
      bzero(buffer,256); 
      n = read(ConnectFD, buffer, 1);
      readError(n);
      if (n == 1)
      {
        switch (buffer[0])
        {
        // message to user request
        case 'M':
          cout << "Cliente " << ConnectFD << " envia mensaje a otro usuario." << endl;
          n = read(ConnectFD, buffer, 2);
          readError(n);
          buffer[2] = '\0';
          tam = atoi(buffer);
          n = read(ConnectFD, buffer, tam);
          readError(n);
          buffer[n] = '\0';
          nickD = buffer;
          n = read(ConnectFD, buffer, 2);
          readError(n);
          buffer[2] = '\0';
          tam = atoi(buffer);
          n = read(ConnectFD, buffer, tam);
          readError(n);
          buffer[n] = '\0';
          msg = buffer;
          // search for user with that nick
          for (itr = clients.begin(); itr != clients.end(); ++itr)
          {
            if (itr->first == nickD)
              connD = itr->second;
          }
          sprintf(buffer, "M%02d%s%02d%s", nickD.size(), nickD.c_str(), msg.size(), msg.c_str());
          cout << endl << "M buffer: " << buffer << endl;
          n = write(connD, buffer, sizeof(buffer)/sizeof(*buffer));
          cout << "Mensaje enviado con exito." << endl;
          break;
        // request of list of users currently online
        case 'S':
          cout << "Cliente " << ConnectFD << " solicita lista de usuarios." << endl;
          for (itr = clients.begin(); itr != clients.end(); ++itr)
          {
            tmp += itr->first + ";";
          }
          if (tmp.size() < 10)
            tmp.insert(0,"0" + to_string(tmp.size()));
          else
            tmp.insert(0,to_string(tmp.size()));
          tmp.insert(0,"L");
          n = write(ConnectFD, tmp.c_str(), tmp.size());
          break;
        default:
          break;
        }    
      }
      else
          cout << "Protocolo no valido ..." << endl;

      //if (n < 0) perror("ERROR reading from socket");
      //printf("Client %i: %s\n",ConnectFD,buffer);
      //cout<<"Type a message: ";
      //message="Client " + to_string(ConnectFD) + ":" + buffer;
      //getline(cin,message);
      //Broadcast(ConnectFD, message);
    }
    /*
    for(int i = 0; i < clients.size(); ++i) 
    {
      if (clients[i] == ConnectFD) 
      {
        shutdown(ConnectFD, SHUT_RDWR);
        close(ConnectFD);
        clients.erase(clients.begin() + i);
      }
    }
    */
}


void readError(int n)
{
  if (n < 0) {
    perror("ERROR reading from socket");
    exit(0);
  }
}