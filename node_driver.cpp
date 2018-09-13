#include <iostream>

using namespace std;

void sender(void *);
void receiver(void *);

int main(int argc, char **argv)
{
   if(argc != 4)
   {
      cerr << "[-] Error : Invalid number of arguments for node process." << endl;
      return -1;
   }

   const string NID = argv[1];
   const string HOSTNAME = argv[2];
   const string PORT = argv[3];

   cout << "Hello from node (" << NID << " " << HOSTNAME << " " << PORT << ")" << endl;

   return 0;
}

void sender(void *args)
{

}

void receiver(void *args)
{

}
