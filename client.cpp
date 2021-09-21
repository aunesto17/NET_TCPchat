/* Client code in C++ */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
// c++ Librarys
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <thread>
#include <fcntl.h>

using namespace std;
// main socket for client
int socketCli;
// socket active
bool active;

void Connection(string IP, int port);
void Listen();

int main(void)
{
  Connection("127.0.0.1",55000);

  char buffer[128];
  int n;
  thread(Listen).detach();
  
  while( 1 )
  { 
    string command, msg, nickD;
    cout << "Ingrese su comando [R, M, S, Q]: ";
    cin >> command;
    //command = "";
    //getline(cin,command);
    //n = write(socketCli,command.c_str(),command.length());
    if(command.compare("R") == 0)
    {
      cout << "Ingrese su nick: ";
      cin >> msg;
      sprintf(buffer, "R%02d%s", msg.size(), msg.c_str());
      n = write(socketCli, buffer, msg.size()+3);
      bzero(buffer, 128);
    }
    else if (command.compare("M") == 0)   
    {
      cout << "Ingrese nick de dest.: ";
      cin >> nickD;
      cout << "Ingrese msg: ";
      cin >> msg;
      sprintf(buffer, "M%02d%s%02d%s", nickD.size(), nickD.c_str(), msg.size(), msg.c_str());
      //n = write(socketCli, buffer, msg.size()+5);
      n = write(socketCli, buffer, sizeof(buffer)/sizeof(*buffer));
      bzero(buffer, 128);
    }
    else if (command.compare("S") == 0)   
    {
      n = write(socketCli, "S", 1);
      if (n != 1)
        cout << "Error en el envio..." << endl; 
    }
    else if (command.compare("Q") == 0)   
    {
      n = write(socketCli, "Q", 1);
      if (n != 1)
        cout << "Error en el envio..." << endl; 
      active = false;
    }     
  }


  shutdown(socketCli, SHUT_RDWR);
  close(socketCli);
  return 0;
}


void Connection ( string IP, int port)
{
  struct sockaddr_in stSockAddr;
  socketCli = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  int Res;
  int n;

  if (-1 == socketCli)
  {
    perror("cannot create socket");
    exit(EXIT_FAILURE);
  }

  memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(port);
  Res = inet_pton(AF_INET, IP.c_str() , &stSockAddr.sin_addr);

  if (0 > Res)
  {
    perror("error: first parameter is not a valid address family");
    close(socketCli);
    exit(EXIT_FAILURE);
  }

  else if (0 == Res)
  {
    perror("char string (second parameter does not contain valid ipaddress");
    close(socketCli);
    exit(EXIT_FAILURE);
  }

  if (-1 == connect(socketCli, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
  {
    perror("connect failed");
    close(socketCli);
    exit(EXIT_FAILURE);
  }
  // no connection errors
  active = true;
}

void Listen()
{ 
  int n;
  char buffer[256];
  int tam;
  while(active)
  {
    
    bzero(buffer,256);
    n = read(socketCli, buffer, 1);
    if (n < 0) {
      perror("ERROR reading from socket");
      exit(0);
    }
    switch (buffer[0])
    {
    // recieve list of user in the server
    case 'L':
      n = read(socketCli, buffer, 2);
      buffer[2] = '\0';
      tam = atoi(buffer);
      n = read(socketCli, buffer, tam);
      buffer[n] = '\0';
      cout << endl << "Lista de usuarios: " << buffer << endl;
      break;
    // recieve command from user
    case 'M':
      n = read(socketCli, buffer, 2);
      buffer[2] = '\0';
      tam = atoi(buffer);
      n = read(socketCli, buffer, tam);
      buffer[n] = '\0';
      cout << endl << "Msg de " << buffer << endl;
      n = read(socketCli, buffer, 2);
      buffer[2] = '\0';
      tam = atoi(buffer);
      n = read(socketCli, buffer, tam);
      cout << ": " << buffer << endl;
      break;
    // signal to terminate
    case 'Q':
      active = false;
      break;
    default:
      break;
    }

    //cout<<buffer<<endl;
  }
}