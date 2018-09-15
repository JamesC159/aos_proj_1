#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <thread>
#include <fstream>
#include <regex>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "Node.h"

using namespace std;

void sender(Node *);
void receiver(Node *);
void receiverProcessor(int, int);
string trim(const string &);
string trim_l(const string &);
string trim_r(const string &);
bool openConfig(ifstream &, const char *);
void closeConfig(ifstream &);
bool isValid(const string);


/* DYNAMIC MEMORY:
 *
 * nodes_map
 * buffer - in receiver thread
 */

int main(int argc, char **argv)
{

   // validate command line arguments
   if(argc != 3)
   {
      cerr << "[-] Error : Invalid number of arguments for node process." << endl;
      cerr << "[-] Usage : ./node <config path> <id>" << endl;
      return -1;
   }

   const char *PATH = argv[1];   // config file path
   const string NID_S = argv[2]; // node id str
   ifstream conf_file;           // config file
   int num_nodes = 0;            // number of nodes in the network
   int vld_ctr = 0;              // valid line counter
   int NID;                      // node id int
   string line = "";             // line from config file
   map< int, Node *> nodes_map;  // map of node ids to node pointers
   vector< thread > threads;     // list of threads
   stringstream ss;              // type converter

   // convert the string nid to int
   ss << NID_S;
   ss >> NID;
   ss.str("");

   // open the config file
   if(!openConfig(conf_file, PATH))
   {
      cerr << "[-] Error : Could not open the configuration file successfully." << endl;
      return -1;
   }

   // read the config file line by line
   while(getline(conf_file, line))
   {
      // trim the line of leading and trailing whitespace
      trim(line);

      // If the line is empty or is a comment, it is not valid
      if(!isValid(line))
      {
         continue;
      }

      // load the line tokenizer
      stringstream ss(line);

      /*
       * parse each section of the config file
       *
       * line == 0 --> num_nodes
       * 1 <= line <= num_nodes --> list of nodes
       * num_nodes < line <= (2 * num_nodes) --> each node's adjacency list
       * line > (2 * num_nodes) --> end of file, ignore
       */

      if(vld_ctr == 0)
      {
         ss >> num_nodes;
      }
      else if(vld_ctr >= 1 && vld_ctr <= num_nodes)
      {
         int id = 0;
         int port = 0;
         string hostname = "";

         // get node's id, hostname, and port
         ss >> id >> hostname >> port;

         // create the new node
         Node *node = new Node(id, hostname, port);

         // insert the node in the map
         nodes_map[id] = node;
      }
      else if(vld_ctr > num_nodes && vld_ctr <= (2 * num_nodes))
      {
         int id = 0;

         // get the node's id
         ss >> id;

         // look for it in the node map
         auto key = nodes_map.find(id);

         // if the node exists, create the adjacency list for it
         if(key != nodes_map.end())
         {
            // grab the current node
            Node *node = (Node *)(key->second);

            int adj_id = 0;

            // read the adj node id's
            while(ss >> adj_id)
            {
               // look for it in the map
               auto adj_key = nodes_map.find(adj_id);

               // if found, insert the node into the adj list
               if(adj_key != nodes_map.end())
               {
                  Node *adj_node = (Node *)(adj_key->second);
                  node->insertAdjNode(adj_node);
               }
            }
         }
      }
      else
      {
         // ignore the end of the file.
         break;
      }

      // increment valid line ctr for next interation
      vld_ctr++;
   }

   // close the conf file since we are done with it.
   closeConfig(conf_file);

   // print the nodes for debug
   for(auto &kv : nodes_map)
   {
      Node *node = (Node *)(kv.second);
      node->toString();
   }

   // print the nodes adj_lst for debug
   for(auto &kv : nodes_map)
   {
      Node *node = (Node *)(kv.second);
      node->printAdjNodes();
   }

   /*
    * Now that we are done with the config file, we can start sender and receiver threads.
    */

   // look for this node in the map
   Node *this_node;
   auto key = nodes_map.find(NID);

   if(key == nodes_map.end())
   {
      // couldn't find my own node in the map, quit
      nodes_map.clear();
      cerr << "[-] Error : Couldn't find own node in the node map." << endl;
      return -1;
   }

   // retreive this node
   this_node = (Node *)(key->second);

   // start sender and receiver threads with this node
   threads.push_back(thread(sender, this_node));
   threads.push_back(thread(receiver, this_node));

   // join all threads before exiting main thread
   for(int i = 0; i < threads.size(); i++)
   {
      threads[i].join();
   }

   // TODO : print list of k hop neighbors
   

   return 0;
}


/**
 * Sender thread
 * @param n This node
 */
void sender(Node *this_node)
{
   // TODO - sender needs to setup socket connections with all of its neighbors
   cout << "Inside sender thread - Node " << this_node->nid << endl;


    
    
    // using 1 sender threads for all neighbor
    // it's possible to use 1 sender thread for each neighbor, which will spawn more threads.
    int countOf1Hop = this_node->adj_lst.size();
    int client_sd[countOf1Hop]; // sender socket desc, sender is client in the client-server model

    struct sockaddr_in client_addr; // sender address struct
    struct hostent *receiverHost;
    Node * neighborNode;


    for (int i = 0; i < countOf1Hop; i++)
    {
        // get the neighbor node #i (on adj list) for this node
        neighborNode = (this_node->adj_lst[i]);

        // create the sender socket for each direct neighbor
        if((client_sd[i] = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        {
            cerr << "[-] Error : Failed creating sender socket in node " << this_node->nid << endl;
            return;
        }

        // get host info for neighbor #i
        receiverHost = gethostbyname(neighborNode->hostname);

        if ( (receiverHost == 0) || (receiverHost == NULL) ) {
            cerr << "[-] Error : Failed getting host info for node " << neighborNode->nid << " in node " << this_node->nid << endl;
            return;
        }
        
        // setup the address structure
        client_addr.sin_family = AF_INET;
        client_addr.sin_addr.s_addr = ((struct in_addr *)(receiverHost->h_addr))->s_addr; 
        client_addr.sin_port = htons(neighborNode->port);  // get the port of direct neighbor #i

        // connect to neighbor #i through given port
        if ( connect(client_sd[i], (struct sockaddr *) &client_addr, sizeof(client_addr) ) == -1 )
        {
            cerr << "[-] Error : Failed connecting to receiver socket of node " << neighborNode->nid << " from node " << this_node->nid << endl;
            return;
        }

    }

    while (true)

    {

        
    }
   


   
}


/**
 * Receiver thread
 * Client packet format - <packet header> <[init msg|report msg|done msg]>
 * @param n This node.
 */
void receiver(Node *this_node)
{
   int server_sd; // receiver socket desc
   int client_sd; // client socket desc
   struct sockaddr_in server_addr; // receiver address struct
   int addr_len;
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
   
   addr_len = sizeof(server_addr)
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

/**
 * Opens the configuration file.
 * @param conf The configuration file to open.
 * @param path The path to the configuration file.
 */
bool openConfig(ifstream &conf, const char *path)
{
   conf.open(path);
   if(!conf.is_open())
   {
      cerr << "[-] Error : Failed opening the configuration file." << endl;
      return false;
   }
   return true;
}

/**
 * Closes the configuration file.
 * @param conf The configuration file to close.
 */
void closeConfig(ifstream &conf)
{
   if(conf.is_open())
   {
      conf.close();
   }
}

/**
 * Trims leading whitespace from a string
 * @param str The string to trim leading whitespace from.
 * @return The substring containing no leading whitespace.
 */
string trim_l(const string &str)
{
   const string pattern = " \f\n\r\t\v"; // pattern to match whitespace
   return str.substr(str.find_first_not_of(pattern));
}

/**
 * Trims trailing whitespace from a string
 * @param str The string to trim trailing whitespace from.
 * @return The substring containing no trailing whitespace.
 */
string trim_r(const string &str)
{
   const string pattern = " \f\n\r\t\v"; // pattern to match white space
   return str.substr(0,str.find_last_not_of(pattern) + 1);
}

/**
 * Trims leading and trailing whitespace from a string
 * @param str The string to trim leading and trailing whitespace from.
 * @return The substring containing no leading and trailing whitespace.
 */
string trim(const string &str)
{
   return (!str.empty() ? trim_l(trim_r(str)) : "");
}

/**
 * Checks to make sure a line is valid in the configuration file.
 * @param str The line from the configuration file.
 * @return True if it is not empty or is a comment, false otherwise.
 */
bool isValid(const string str)
{
   regex cmt_rgx("^([^#]).*");  // regex to filter out lines that start with a comment
   return (!str.empty() ? regex_match(str, cmt_rgx) : false);
}
