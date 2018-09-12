#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <vector>
#include <string>

using namespace std;

class Node
{
private:


public:
    int nid;
    int port;
   string hostname;
   vector<Node *> adj_lst;
   
   Node(const int, const string, const int);
   ~Node();

   void insertAdjNode(Node *);
   void printAdjNodes() const;
   void toString() const;
};

#endif
