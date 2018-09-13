#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <thread>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "Node.h"

using namespace std;

void sender(void *);
void receiver(void *);
void receiverProcessor(int, int);


/* DYNAMIC MEMORY:
 *
 * Node *this_node
 */

int main(int argc, char **argv)
{

   // validate command line arguments
   if(argc != 4)
   {
      cerr << "[-] Error : Invalid number of arguments for node process." << endl;
      cerr << "[-] Usage : node <id> <hostname> <port>" << endl;
      return -1;
   }

   stringstream ss;         // type converter
   const string NID_S = argv[1];      // node id str
   const string HOSTNAME_S = argv[2]; // node hostname str
   const string PORT_S = argv[3];     // node port str
   int NID;                   // node id int
   int PORT;                  // node port int
   vector< thread > threads;  // list of threads

   // convert the string nid to int
   ss << NID_S;
   ss >> NID;
   ss.str("");

   // convert the string port to int
   ss << PORT_S;
   ss >> PORT;
   ss.str("");

   Node *this_node = new Node(NID, HOSTNAME_S, PORT); // This Node
   cout << "Hello from node (" << NID << " " << HOSTNAME_S << " " << PORT << ")" << endl;

   /*
    * Parse the configuration file for our id in
    * the adjacency list section and fill our adjacency list
    */

   // start sender and receiver threads with this node
   threads.push_back(thread(sender, this_node));
   threads.push_back(thread(receiver, this_node));

   // join all threads before exiting main thread
   for(int i = 0; i < threads.size(); i++)
   {
      threads[i].join();
   }

   // delete this node's memory
   if(this_node)
   {
      delete this_node;
   }
   this_node = nullptr;

   return 0;
}

/**
 * Sender thread
 * @param n This node
 */
void sender(void *n)
{
   // TODO - sender needs to setup socket connections with all of its neighbors

   Node *this_node = (Node*)n;
   cout << "Inside sender thread - Node " << this_node->nid << endl;
}

/**
 * Receiver thread
 * Client packet format - <packet header> <[init msg|report msg|done msg]>
 * @param n This node.
 */
void receiver(void *n)
{
   Node *this_node = (Node*)n;
   int server_sd; // receiver socket desc
   int client_sd; // client socket desc
   struct sockaddr_in server_addr; // receiver address struct
   int addr_len = sizeof(server_addr);
   vector<thread> threads; // receiver processing threads
   
   cout << "Inside receiver thread - Node " << this_node->nid << endl;

   // create the receiver socket
   if((server_sd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
   {
      cerr << "[-] Error : Failed creating receiver socket in node " << this_node->nid << endl;
      return;
   }

   // setup the address structure
   server_addr.sin_family = AF_INET;
   server_addr.sin_addr.s_addr = INADDR_ANY;
   server_addr.sin_port = htons(this_node->port);

   // bind the socket to the port
   bind(server_sd, (struct sockaddr *)&server_addr, sizeof(server_addr));
   
   while(true)
   {
      // listen for incoming connections
      if(listen(server_sd, this_node->adj_lst.size()) < 0)
      {
         cerr << "[-] Error : Failed listening on receiver socket in node " << this_node->nid << endl;
         return;
      }

      // accept the client connection
      if((client_sd = accept(server_sd, (struct sockaddr *)&server_addr, (socklen_t*)&addr_len)) < 0)
      {
         cerr << "[-] Error : Failed accepting client connection in receiver in node " << this_node->nid << endl;
         return;
      }

      // start a new receiver processor thread for this client
      threads.push_back(thread(receiverProcessor, server_sd, client_sd));
   }
}

/**
 * Receiver thread for processing client communication.
 *
 * Client packet format - <packet header> <[init msg|report msg|done msg]>
 *
 * The packet header consists of the source node and destination node connection information
 *
 * If the message is init, the receiver checks the hop counter = 0 and reports its information
 * back to the source. if hop counter != 0, we decrement the hop count by 1 and
 * broadcast to all our neighbors except toward the source.
 *
 * If the message is report msg, then this is a packet from another node who is reporting
 * their information to the source. If the destination is this node, add it to the reported node.
 * If not, then pass it toward the destination.
 *
 * If the message is done msg, then the client is done with the process. We close it's socket
 * connection and pass it on to the source.
 *
 * @param sd The server socket descriptor.
 * @param cd The client socket descriptor..
 */
void receiverProcessor(int sd, int cd)
{
   bool not_done = true;

   // read from the client until we receive the done status
   while(not_done)
   {
      char *buffer = new char[1024];
      int valread = read(sd, buffer, 1024);

      // TODO - check to see if client done signal is received
      // TODO - parse message received from client to determine what to do

      cout << "Receiver - " << buffer << endl;

      // delete buffer memory before next read
      delete [] buffer;
   }

   // close the client's connection
   close(cd);
}
