#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <thread>
#include <fstream>
#include <regex>
#include <queue>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/* POSIX semaphore and pthread library*/
#include <semaphore.h>
#include <pthread.h>

#include "Node.h"
#include "nodeMsg.h"
#include "parser.h"

using namespace std;

void sender(Node *);
void receiver(Node *);
void receiverProcessor(int, int);

/* DYNAMIC MEMORY:
 *
 * nodes_map
 * buffer - in receiver thread
 */

/* threads' shared memory */
sem_t incomingQueue; // semaphore to access incoming queue
sem_t outgoingQueue; // semaphore to access outgoing queue

std::queue<nodeMsg> incomingMsgQueue; // queue for incoming msg (received)
std::queue<nodeMsg> outgoingMsgQueue; // queue for outgoing msg (to be sent)
int numOfNodes; // number of Nodes in the network, n.
bool terminable; // bool variable to signal receiver & sender's termination

int main(int argc, char **argv)
{
   // validate command line arguments
   if(argc != 3)
   {
      cerr << "[-] Error : Invalid number of arguments for node process." << endl;
      cerr << "[-] Usage : ./node <config path> <id>" << endl;
      return -1;
   }

   const string NID_S = argv[2]; // node id str
   int vld_ctr = 0;              // valid line counter
   int NID = std::stoi(argv[2]);                      // node id int
   vector< thread > threads;     // list of threads

   Parser p1(argv[1]);
   p1.Parse_Config();

   auto nodes_map = p1.node_map;
   int num_nodes = p1.num_nodes;

//   print the nodes adj_lst for debug
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

   // retrieve this node
   this_node = (Node *)(key->second);

/* prepare for mullti threads' operation */
numOfNodes = num_nodes; // number of nodes in the network

sem_init(&incomingQueue, 0, 1);
sem_init(&outgoingQueue, 0, 1);
// for the semaphores
// 2nd parameter = 0 for threads, != 0 for forked processes, here we use threads, so = 0
// 3rd parameter = semaphore init value = number of desired concurrent users of resource

int hopCountMap[num_nodes]; // array to store hop count for each of n nodes in network
bool  completionMap[num_nodes]; // array to track status of each of n nodes
for (int i=0; i< num_nodes; i++)
{
    hopCountMap[i] = num_nodes;
    /* for n nodes, max hop = n-1, set hop count to n to signify infinite hop/undiscovered nodes */
    completionMap[i] = false; // initiate completion status for all nodes = false
}
hopCountMap[(this_node->nid)] = 0; // set my hop count from me = 0...

terminable = false; // initiate var terminable
int numOfDiscoveredNodes = 1; // discovered myself
int numOfCompletedNodes = 0; // no one is done yet
   
   // start sender and receiver threads with this node, should start receiver before sender - as receiver acting as "server"
   threads.push_back(thread(receiver, this_node));
   threads.push_back(thread(sender, this_node));

// main thread will now be acting as master processor for receiver and sender threads

nodeMsg msgInProcess; // to hold msg from the queue to be processed. Basically, a place holder...
/* first msg, the discovery msg */
msgInProcess.msgCode = 0;
msgInProcess.oriSender = this_node->nid;
msgInProcess.msgHop = 1;
msgInProcess.curSender = this_node->nid;
sem_wait(&outgoingQueue); // call the wait on semaphore for outgoing queue
outgoingMsgQueue.push(msgInProcess); // push the msg to the outgoing queue
sem_post(&outgoingQueue); // signal semaphore for outgoing queue

bool hasIncomingQueue;
sem_wait(&incomingQueue);
hasIncomingQueue = !incomingMsgQueue.empty();
sem_post(&incomingQueue);

/* incoming -> processing -> outgoing */
while ( (numOfCompletedNodes < num_nodes) || (hasIncomingQueue) )
{
    // if numOfCompletedNodes = num_nodes, but incoming msg queue is not empty, still need to finish its work backlog
    if(hasIncomingQueue)
    // only do the processing if incoming queue has something
    {
        sem_wait(&incomingQueue); // wait on the semaphore 
        msgInProcess = incomingMsgQueue.front(); // get the first msg - one in front of the queue.
        incomingMsgQueue.pop(); // pop the front of the queue.
        sem_post(&incomingQueue); // signal the semaphore
        switch (msgInProcess.msgCode)
        {
            case 0:
            // discovery msg
                if (msgInProcess.msgHop < hopCountMap[msgInProcess.oriSender])
                {
                    hopCountMap[msgInProcess.oriSender] = msgInProcess.msgHop;
                    msgInProcess.msgHop++; // increment the hop
                    sem_wait(&outgoingQueue); // call the wait on semaphore for outgoing queue
                    outgoingMsgQueue.push(msgInProcess); // push the msg to the outgoing queue
                    sem_post(&outgoingQueue); // signal semaphore for outgoing queue 

                }
                /* the tweak
                * if the msgHop >= hopCountMap[msgInProcess.oriSender]
                * it means the node that is the oriSender of that msg has already been discovered previously by me,
                * which also means I should have forwarded a similar msg from that oriSender before, which got to me through another middle node,
                * thus, there is no need to forward another
                * this would reduce the total number of msg in the network
                */

            break;
            case 1:
                // completion/termination signal msg
                if (completionMap[msgInProcess.oriSender] == false)
                {
                    completionMap[msgInProcess.oriSender] = true; // set the status of that nodes to completed
                    numOfCompletedNodes++; // increment number of completed nodes
                    sem_wait(&outgoingQueue); // call the wait on semaphore for outgoing queue
                    outgoingMsgQueue.push(msgInProcess); // push the msg to the outgoing queue
                    sem_post(&outgoingQueue); // signal semaphore for outgoing queue 
                }
            break;
            default:
            break;
        }
        

        // checking the number of discovered nodes
        if ( numOfDiscoveredNodes < num_nodes )
        {
            // reset numOfDiscoveredNodes to 0 then recount;
            numOfDiscoveredNodes = 0;
            for (int i = 0; i< num_nodes; i++)
            {
                if ( (hopCountMap[i] < num_nodes) && (hopCountMap[i] >= 0) )
                {
                    numOfDiscoveredNodes++;
                }
            }

            // if all nodes have just been discovered, push the completion signal msg to outgoing queue
            if (numOfDiscoveredNodes == num_nodes)
            {
                // assemble a completion signal msg
                msgInProcess.msgCode = 1;
                msgInProcess.oriSender = (this_node->nid);
                msgInProcess.msgHop = num_nodes;
                msgInProcess.curSender = (this_node->nid);
                sem_wait(&outgoingQueue); // call the wait on semaphore for outgoing queue
                outgoingMsgQueue.push(msgInProcess); // push the msg to the outgoing queue
                sem_post(&outgoingQueue); // signal semaphore for outgoing queue 

                // change my completion status on my completion map to true
                completionMap[(this_node->nid)] = true;
                numOfCompletedNodes++; // increase numOfCompletedNodes by 1, me.
            }
        }
    }
    sem_wait(&incomingQueue);
    hasIncomingQueue = !incomingMsgQueue.empty(); // recheck if queue is empty yet
    sem_post(&incomingQueue);
}

// set var terminable = true to signal sender and receiver threads that they can terminate
terminable = true;



    // find max hop count after discovery
   int maxHop = 1;
   for(int i = 0; i < num_nodes; i++)
   {
       if (hopCountMap[i] > maxHop)
       {
           maxHop = hopCountMap[i];
       }
   }

   // print out the nodes per hop count from current node
   // simple print procedure, not optimal time complexity, O(n*maxHop), worst case: O(n(n-1)) = O (n^2)
   // there should be better ways to be figured out
    cout << "Number of nodes in network: " << num_nodes << endl;
    cout << "Current node: " << this_node->nid << endl;
   for (int i = 1; i <= maxHop; i++ )
   {   
       cout << i << "-hop nodes: " << endl;
       for (int j = 0; j < num_nodes; j++ )
        {
            if (hopCountMap[j] == i)
            {
                cout << "\t" << j << endl;
            }
        }
        cout << endl;
   }
   cout << "End .!." << endl;


   // join all threads before exiting main thread
   for(int i = 0; i < threads.size(); i++)
   {
      threads[i].join();
   }


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
    // it's possible to use 1 sender thread for each neighbor, which will spawn more threads. Also, it's somewhat harder to control the queue.

    int countOf1Hop = this_node->adj_lst.size();
    int client_sd[countOf1Hop]; // sender socket desc, sender is client in the client-server model

    struct sockaddr_in client_addr[countOf1Hop]; // sender address struct
    struct hostent *receiverHost[countOf1Hop]; // receiver host struct
    Node * neighborNode;


    for (int i = 0; i < countOf1Hop; i++)
    {
        // get the neighbor node #i (on adj list) for this node
        neighborNode = (this_node->adj_lst[i]);

        // create the sender socket for each direct neighbor
        if( ( client_sd[i] = socket(AF_INET, SOCK_STREAM, 0) ) == 0 )
        {
            cerr << "[-] Error : Failed creating sender socket in node " << this_node->nid << endl;
            return;
        }

        // get host info for neighbor #i
        receiverHost[i] = gethostbyname((neighborNode->hostname).c_str());

        if ( (receiverHost[i] == 0) || (receiverHost[i] == NULL) )
        {
            cerr << "[-] Error : Failed getting host info for node " << neighborNode->nid << " in node " << this_node->nid << endl;
            return;
        }
        
        // setup the address structure
        client_addr[i].sin_family = AF_INET;
        client_addr[i].sin_addr.s_addr = ((struct in_addr *)(receiverHost[i]->h_addr))->s_addr; 
        client_addr[i].sin_port = htons(neighborNode->port);  // get the port of direct neighbor #i

        // connect to neighbor #i through given port, retry until success
        while (connect( client_sd[i], (struct sockaddr *) &client_addr[i], sizeof(client_addr[i]) ) == -1)
        {
        
        // initial design, only 1 connect attempt >> failed >> bad!  --Khoa
        /*if ( connect( client_sd[i], (struct sockaddr *) &client_addr[i], sizeof(client_addr[i]) ) == -1 )
        {
            cerr << "[-] Error : Failed connecting to receiver socket of node " << neighborNode->nid << " from node " << this_node->nid << endl;
            return;
        }*/

        }
        cout << "Successfully connected to receiver socket of node " << neighborNode->nid << endl;

    }

    int prevSender; // to recognize previous sender so as not to rebound msg
    nodeMsg outgoingMsg;

    bool hasOutgoingQueue;
    sem_wait(&outgoingQueue);
    hasOutgoingQueue = !(outgoingMsgQueue.empty());
    sem_post(&outgoingQueue);

    while ( (!terminable) || (hasOutgoingQueue) )
    // while terminable is not set, or outgoing queue is not empty, do sending out msg from the queue
    {
        //
        if(hasOutgoingQueue)
        {
            sem_wait(&outgoingQueue);
            outgoingMsg = outgoingMsgQueue.front(); 
            outgoingMsgQueue.pop();            
            sem_post(&outgoingQueue);

            prevSender = outgoingMsg.curSender;
            outgoingMsg.curSender = this_node->nid;
            for (int i= 0; i< countOf1Hop; i++)
            {
                neighborNode = (this_node->adj_lst[i]);
                if ( (neighborNode->nid) != prevSender )
                // if 1 sent to 2, then 2 is not rebounding it back to 1
                {
                    if ( write(client_sd[i], &outgoingMsg, sizeof(outgoingMsg) ) == -1)
                    {
                        cerr << "Error on write call" << endl;
                        return;
                    }
                }
            }
        }
        sem_wait(&outgoingQueue);
        hasOutgoingQueue = !(outgoingMsgQueue.empty()); // recheck if queue is empty
        sem_post(&outgoingQueue);
    }
   
    for (int i = 0; i < countOf1Hop; i++)
    {
        close(client_sd[i]);
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
   
   addr_len = sizeof(server_addr);
   // bind the socket to the port
   bind(server_sd, (struct sockaddr *)&server_addr, sizeof(server_addr));
   
    // listen for incoming connections
    if(listen(server_sd, this_node->adj_lst.size()) < 0)
    {
        cerr << "[-] Error : Failed listening on receiver socket in node " << this_node->nid << endl;
        return;
    }


    nodeMsg incomingMsg; // place holder for incoming msg

    while(!terminable)
    // while terminable status has not been set
    {
        // accept the client connection
        if((client_sd = accept(server_sd, (struct sockaddr *)&server_addr, (socklen_t*)&addr_len)) < 0)
        {
            cerr << "[-] Error : Failed accepting client connection in receiver in node " << this_node->nid << endl;
            return;
        }

        while( read(client_sd, &incomingMsg, sizeof(incomingMsg))  > 0 )
        {
            sem_wait(&incomingQueue); // wait on the semaphore for incoming queue
            incomingMsgQueue.push(incomingMsg); // push the msg onto incoming queue
            sem_post(&incomingQueue); // signal the semaphore
        }

        /* I took this out, as we are using main thread as processor --Khoa */
      // start a new receiver processor thread for this client
      //threads.push_back(thread(receiverProcessor, server_sd, client_sd));
    }
    
    close(client_sd);
    close(server_sd);
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
/*void receiverProcessor(int sd, int cd)
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
}*/
