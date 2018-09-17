#ifndef MESSAGE_H
#define MESSAGE_H

#include <iostream>
#include <vector>

class Message
{
	public:
		std::string kind;
		std::vector<int> path;
		std::vector<int> visited;
		Message();
		Message(std::string kind);
		friend std::ostream &operator<<(std::ostream &os, Message const &m); 
		//int hop_number; // You don't need the hop number because the size of path tells you the hop number
};

#endif // MESSAGE_H
