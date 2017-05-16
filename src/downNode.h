/******************************
 * downNode.h:  see downNode.cpp
 * ****************************/
class downNode
{
   private:
     int child;
     int sibling;
	
   public:
     downNode();
     void setChild (int c);
     void setSibling (int s);
     int getChild ();
     int getSibling ();
     void print();
};

