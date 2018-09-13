#include <iostream>

using namespace std;

void sender(void *);
void receiver(void *);

int main(int argc, char **argv)
{
   if(argc != 4)
   {
      cerr << "[-] Error : Invalid number of arguments for node process." << endl;
      cerr << "[-] Usage : node <id> <hostname> <port>" << endl;
      return -1;
   }

   const string NID = argv[1];
   const string HOSTNAME = argv[2];
   const string PORT = argv[3];

   cout << "Hello from node (" << NID << " " << HOSTNAME << " " << PORT << ")" << endl;

   return 0;
}

/**
 * Sender thread for the node
 * @param args
 */
void sender(void *args)
{

}

/**
 * Receiver thread for the node
 * @param args
 */
void receiver(void *args)
{

}
