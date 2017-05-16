/*************************
  heap.h:  see heap.cpp
 ************************/
#include <math.h>

// Objects to represent the nodes of the position heap's tree.
class downNode;  
class mylist;
const int ROOT = 0;
const int NOCHILD = -1;  
class heap
{
    public:
        heap (char *str);
        ~heap();
        void preorderPrint();
        mylist *search(char *pattern, int patternLength);
    private:
        int *parent;          // upwardly-directed tree for storing primal 
                              //   position heap during construction
                              //   (set to NULL once constructed)
	downNode *downArray;  // array of nodes of downwardly directed tree
        int *maxReach;        // maximal-reach pointers 
        int *discoveryTime;  // DFS discovery times of tree nodes
        int *finishingTime;  // DFS finishing times of tree nodes
	char *text;           // text string that the heap is constructed from
	int textLength;       // number of characters in the text
        int getTextLength();
        void build();
        mylist *genCandidates(char *pattern, int patternLength, int &pathEndDepth);
        mylist *pruneCandidates(char *pattern, int patternLength, 
                                mylist *candidates, int &offset);
        int indexIntoTrie(char *pattern, int patternLength, int &endDepth);
        void appendSubtreeOccurrences(int node, mylist *Occurrences);
        void installMaxReaches();
        void setDiscoveryFinishing();
        void setDFAux (int index, int depth, int &i);
        bool isDescendant(int node1, int node2);
        mylist *pathOccurrences(char *pattern, int patternLength, 
                               int pathEndNode);
        int childOnLetter(int node, int depth, char c);  
        void insertChild (int child, int parent); 
        void preorderAux (int index, int depth);
};
