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

int main(int argc, char **argv)
{
   // validate command line arguments
   if(argc != 2)
   {
      cerr << "[-] Error : Invalid number of command line arguments." << endl;
      cerr << "[-] Usage : proj_1 <config_path>" << endl;
      return -1;
   }

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

   /*
    * Now since we have parsed the configuration file and obtained all the information
    * from it, we can fork and execvp the child node processes run with their
    * configuration parameters.
    */

   int num_procs = nodes_map.size();
   int pid[num_procs]; // child process id array
   int idx = 0; // child process id index

   // fork for each node in the map
   for(auto &kv : nodes_map)
   {
      Node *node = (Node *)(kv.second);

      // If the node is null, just continue
      if(!node)
      {
         continue;
      }

      ostringstream ss;
      string id_s; // node id string
      string port_s; // node port string

      // convert node id from int to string
      ss << node->nid;
      id_s = ss.str();

      // convert node port from int to string
      ss << node->port;
      port_s = ss.str();

      // print debugging info
      cout << "NODE: " << id_s << endl << "------------------" << endl;
      cout << id_s << endl;
      cout << node->hostname << endl;
      cout << port_s << endl;
      cout << "------------------" << endl;

      // fork a child process to execute the node driver with the given node's configuration
      if((pid[idx] = fork()) == 0)
      {
         execlp("./node","./node", id_s.c_str(), node->hostname.c_str(), port_s.c_str());
         cerr << "[-] Error : Failed executing child node process." << endl;
         nodes_map.clear();
         return -1;
      }

      // failed forking the child process
      if(pid[idx] < 0)
      {
         cerr << "[-] Error : Failed forking child process." << endl;
         nodes_map.clear();
         return -1;
      }

      // increment process id index
      idx++;
   }

   // reset idx
   idx = 0;

   // wait for each child process to return before continuing on with the parent process
   for(int i = 0; i < num_procs; i++)
   {
      if(pid[i] > 0)
      {
         int status; // child process return status

         waitpid(pid[i], &status, 0);

         if(status > 0)
         {
            cerr << "[-] Error : Child node process sent exit status error." << endl;
         }
      }
      else
      {
         cerr << "[-] Error : There was no process to wait on." << endl;
      }
   }

   // finish nodes_map cleanup
   nodes_map.clear();

   return 0;
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

