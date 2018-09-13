#include "Node.h"

/**
 * Constructor
 */
Node::Node(const int nid, const string hostname, const int port)
{
   this->nid = nid;
   this->hostname = hostname;
   this->port = port;
   this->adj_lst.clear();
}

Node::~Node()
{

   // Cleanup adjacency list
   for(auto &node : this->adj_lst)
   {
      if(node != nullptr)
      {
         delete node;
      }
   }

   // reset variables
   this->nid = -1;
   this->hostname = "";
   this->port = -1;
   this->adj_lst.clear();
}

void Node::toString() const
{
   cout << "NODE : ID = " << nid << " HOSTNAME = " << hostname << " PORT = " << port << endl;
}

void Node::insertAdjNode(Node *node)
{
   adj_lst.push_back(node);
}

void Node::printAdjNodes() const
{
   cout << "-------------------------------\nNODE " << nid << " ADJACENCY LIST" << endl;
   for(auto &node : adj_lst)
   {
      node->toString();
   }
   cout << "-------------------------------" << endl;
}
