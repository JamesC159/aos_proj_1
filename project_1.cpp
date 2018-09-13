#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <regex>
#include <map>
#include <vector>
#include <thread>

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#include "Node.h"

using namespace std;

string trim(const string &);
string trim_l(const string &);
string trim_r(const string &);
bool openConfig(ifstream &, const char *);
void closeConfig(ifstream &);
bool isValid(const string);

void nodeThread(void *a)
{
}

int main(int argc, char **argv)
{
   // validate command line arguments
   if(argc != 2)
   {
      cerr << "[-] Error : Invalid number of command line arguments." << endl;
      cerr << "[-] Usage : proj_1 <config_path>" << endl;
      return -1;
   }

   const char *NODE_BIN = "./node";
   const char *PATH = argv[1];   // config file path
   ifstream conf_file;           // config file
   int num_nodes = 0;            // number of nodes in the network
   int vld_ctr = 0;              // valid line counter
   string line = "";             // line from config file
   map< int, Node *> nodes_map; // map of node ids to node pointers
   vector< thread > node_threads; // list of threads running node processes

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

   // Now we want to loop for num_nodes and execvp("./node", args) for each node given its args list of char *
   for(auto &kv : nodes_map)
   {
      Node *node = (Node *)(kv.second);

      // If the node is null, just continue
      if(!node)
      {
         continue;
      }

      ostringstream ss;
      string id_s;
      string port_s;

      // convert node id from int to string
      ss << node->nid;
      id_s = ss.str();

      // convert node port from int to string
      ss << node->port;
      port_s = ss.str();

      // execvp args list
      const char *args_lst[] = {
         id_s.c_str(),
         node->hostname.c_str(),
         port_s.c_str()
      };

      // print for debugging
      cout << "NODE: " << id_s << endl << "------------------" << endl;
      for(const char *arg : args_lst)
      {
         cout << arg << endl;
      }
      cout << "------------------" << endl;

      // create new thread, start it and pass it the argument list and binary name
      node_threads.push_back(thread(nodeThread, args_lst));
   }

   // wait for node threads to join
   for(int i = 0; i < node_threads.size(); i++)
   {
      node_threads[i].join();
   }

   // clear node map memory
   if(!nodes_map.empty())
   {
      for(auto &kv : nodes_map)
      {
         Node *node = nullptr;
         if(kv.second)
         {
            delete kv.second;
         }
      }
   }

   // finish nodes_map cleanup
   nodes_map.clear();

   return 0;
}

bool openConfig(ifstream &conf, const char *path)
{
   conf.open(path);
   if(!conf.is_open())
   {
      cerr << "[-} Error : Failed opening the configuration file." << endl;
      return false;
   }
   return true;
}

void closeConfig(ifstream &conf)
{
   if(conf.is_open())
   {
      conf.close();
   }
}

string trim_l(const string &str)
{
   const string pattern = " \f\n\r\t\v";
   return str.substr(str.find_first_not_of(pattern));
}

string trim_r(const string &str)
{
   const string pattern = " \f\n\r\t\v";
   return str.substr(0,str.find_last_not_of(pattern) + 1);
}

string trim(const string &str)
{
   return (!str.empty() ? trim_l(trim_r(str)) : "");
}

bool isValid(const string str)
{
   regex cmt_rgx("^([^#]).*");  // regex to filter out lines that start with a comment
   return (!str.empty() ? regex_match(str, cmt_rgx) : false);
}

