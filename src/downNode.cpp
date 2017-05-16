/***************************
 * downNode.cpp:  implements a downNode in a downwardly-directed rooted tree, that is,
 * a tree where each downNode has a list of its children, and where each downNode
 * can be labeled with a discovery and finishing time during a depth-first
 * search, as well as a "maximal reach pointer" to a descendant downNode.
 * See heap.cpp for more details.
 * **************************/
#include <iostream>
#include "downNode.h"
using std::cout;

downNode::downNode()
{
   child = -1;
   sibling = -1;
}

int downNode::getChild ()
{
   return child;
}

int downNode::getSibling ()
{
   return sibling;
}


void downNode::setChild (int c)
{
   child = c;
}

void downNode::setSibling (int s)
{
   sibling = s;
}

void downNode::print()
{
   cout << " child: " << getChild() << " sibling: " << getSibling();
}
