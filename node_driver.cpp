#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <thread>

#include "Node.h"

using namespace std;


/**
 * Sender thread
 * @param adj_lst This node's adjacency list
 */
void sender(void *adj_lst)
{
   cout << "Inside sender thread" << endl;
}

/**
 * Receiver thread
 * @param adj_lst This node's adjacency list
 */
void receiver(void *adj_lst)
{
   cout << "Inside reeciver thread" << endl;
}

/* DYNAMIC MEMORY:
 *
 * Node *this_node
 * map<int, Node*> adj_lst
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

   map<int, Node*> adj_lst; // adjacency list for this node
   stringstream ss;         // type converter
   const string NID_S = argv[1];      // node id str
   const string HOSTNAME_S = argv[2]; // node hostname str
   const string PORT_S = argv[3];     // node port str
   int NID;  // node id int
   int PORT; // node port int
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

   // start sender and receiver threads with this node's adj_lst
   threads.push_back(thread(sender, &adj_lst));
   threads.push_back(thread(receiver, &adj_lst));

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

   return 0;
}
