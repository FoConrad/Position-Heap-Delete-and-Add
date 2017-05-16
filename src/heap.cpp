/**********************************************/
// fastBuildFastSearch:
//   heap.cpp:  The heap class implements the position heap
/**********************************************/
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include "heap.h"
#include "downNode.h"
#include "generic.h"
#include "mylist.h"
using std::cout;
using std::cin;
using std::endl;


/*********************************************************************
The directory "fastBuildNaiveSearch" gives an implementation of the position
heap that takes O(n) time to build it, and a simple implementation of the 
find operation that does not take O(m+k) time in the worst case.
The directory "naiveBuildFastSearch" gives a simple way to build the
heap so that find operations take O(m+k) time, but the build operation does not
take O(n) time.  The key to the O(m+k) bound for find operations is the
installation of "maximal-reach pointers."  

This file demonstrates how to modify the O(n) build operation so that
it also installs the maximal-reach pointers in O(n) time.  This gives
a implementation that takes O(n) time to build the position heap and O(m+k) 
time for find operations.

We can "name" a node by the sequence of edge labels from
the root to the node.  This is a string of letters from the text's alphabet.
See the naive implementation of the build operation for a definition
of the (primal) position heap.  The nodes of the dual position heap
are the same as the nodes of the primal, so each node has a parent in the 
primal heap and a parent in the dual.  Here's the definition of the
dual heap:  it is the tree where the name of each node is the reverse
of its name in the primal position heap.

Once the O(n) construction algorithm has completed, we find the 
maximal-reach pointers in a way that is similar to the way the O(n)
algorithm builds the heap.  We work from right to left in the text.

The O(n) algorithm construction algorithm climbs from the most recently 
added node in the heap, until it finds a node reachable in the dual on 
character c = text[arrayIndex], where arrayIndex is the position whose
node is about to be added.  This node is x.  The new node is added as 
a child of x.  

Similarly, the O(n) algorithm for installing the maximal-reach pointers
climbs from the node pointed to by the most recently added maximal-reach
pointer, until it finds a node x reachable in the dual on character
c = text[arrayIndex], where arrayIndex is the next node to be labeled
with a maximal-reach pointer.  We make this maximal-reach pointer point
to x.  The analysis of the time bound is identical to that of the linear-time
build algorithm.

Notice that you are always climbing in the position heap and looking
downward in the dual heap.  During construction and installation of
max-reach pointers, we store the position heap as an upwardly-directed tree 
(one parent pointer for each node) and the dual heap as a downwardly-directed 
tree (a list of children for each node).  When we are finished, we
delete the dual, convert the position heap from an upwardly directed tree
to a downwardly-directed heap.  Then we delete the upwardly directed tree.
We then create space for DFS discovery- and finishing-time labels,
and run DFS on the downwardly-directed tree to create these labels.
At each point in time, the space requirement is at most words per position
in the text: a left child and right sibling label, a maximal-reach label, 
and either a parent label or a discovery- and finishing-time label.

By contrast, the O(n) construction algorithm with naive find
operations, which requires only the left child, right sibling, and parent 
pointers during construction, for a total of three integers per position
of text, and discards the parent pointers after construction, leaving
a space requirement of two integers per position of text.  The tradeoff
is that the (pessimistic) worst-case bound for find operations is O(m^2+k)
instead of O(m+k).  The naive construction algorithm with naive find 
operations takes two integers per position of text:  the left child and 
right sibling.  The tradeoff is that the find operation takes O(m^2+k)
time and the construction takes O(nh(T)) time, where h(T) is the length
of the longest substring X of the text T that occurs at least |X| times in 
T.

***************************************************************/

/****************************************/
// position heap constructor.  Builds the position heap
//  for the text pointed to by 'str'
/****************************************/
heap::heap(char *str)
{
    textLength = strlen (str);    // length of text

    // upwardly-directed rooted tree for holding the (primal) position
    // heap during construction ...
    parent = new int [textLength]; 

    // downwardly-directed rooted tree for holding the dual heap during
    //   construction, and also the primal heap when it's been constructed
    //   and is ready for use ...
    downArray = new downNode[textLength];

    // array of maximal-reach pointers; maxReach[i] tells the node
    //   pointed to by node i
    maxReach = new int[textLength];

    // Private version of text.  If you want to keep storage cost down to two 
    //  integers per character of text, you should use the text pointed to 
    //  by *str, rather than keeping a private copy of the text.  
    text = new char[textLength];   
    if (! downArray || !maxReach || !text) 
         {cout << "Memory allocation failure in heap constructor\n"; exit(1);}
    
    char *p1 = str;  char *p2 = text + textLength - 1;
    while (*p1 != '\0') // reverse the indexing order to be from
        *p2-- = *p1++;   //    right to left in private copy of 'text'

    build();                       // build the position heap for the string
}

// position heap destructor ...
heap::~heap()
{
    delete []downArray; 
    delete []parent;
    delete []text;
}

/*******************************************/
// build:  Build the position heap.  Assume text has been reversed in its 
// array so that the indices are in ascending order from right to left.
/*******************************************/
void heap::build ()
{
    int pathNode, child;  // current node on path up, potential parent of 
                          //   new node
    int prevPathNode;  // child of pathNode on way up
    for (int arrayIndex = 1; arrayIndex < textLength; arrayIndex++)
    {
        if ((arrayIndex % 100000) == 0) 
               cout << "Text position: " << arrayIndex << '\n';
        char *textptr = text + arrayIndex; // Next character on indexing path
        
        if (childOnLetter(ROOT, 0, *textptr) == NOCHILD)
        {
            
            parent[arrayIndex] = ROOT;
            insertChild(arrayIndex, ROOT);
            pathNode = arrayIndex;
        }
        else
        {
            char c = text[arrayIndex];

            // Starting at the most recently added node, climb in the primal 
            // position heap until you find a child on the new letter c in 
            // the dual heap.  That child is the longest prefix of 
            // text[arrayIndex..0] that is already a node of the primal 
            // heap.  The new node must therefore be added as a child of 
            // it in the primal heap.  In the dual heap, the new node must 
            // be added to the highest node you saw that doesn't have a 
            // child on letter 'c'.

            // climb ...
            do
            {
                prevPathNode = pathNode;
                pathNode = parent[pathNode];
                child = childOnLetter(pathNode, 0, c);
            } while (child == NOCHILD);  
           
            // add new node to primal heap
            parent[arrayIndex] = child;

            // add new node to dual heap
            insertChild(arrayIndex, prevPathNode);

            // record new node in preparation for next iteration
            pathNode = arrayIndex;
            /*-------------------*/
        }
    }
    installMaxReaches();

    // Turn heap from an upwardly directed tree in parent array to a downwardly
    //  directed tree in downArray, discarding the dual heap ...
    downArray[0].setSibling(NOCHILD);
    downArray[0].setChild(NOCHILD);
    for (int arrayIndex = 1; arrayIndex < textLength; arrayIndex++)
    {
        downArray[arrayIndex].setSibling(NOCHILD);
        downArray[arrayIndex].setChild(NOCHILD);
        insertChild(arrayIndex, parent[arrayIndex]);
    }

    delete [] parent;   // needed only for building and installing max reaches

    // now that the parent array has been deleted, allocate space for
    //   discovery/finishing times
    discoveryTime = new int[textLength]; 
    finishingTime = new int[textLength];
    if (!discoveryTime || !finishingTime)
       {cout << "Memory allocation failure in build\n"; exit(1);}
    setDiscoveryFinishing();
}

/**************************************/
// insertChild:  insert 'child' as a child of 'parent' 
/**************************************/
void heap::insertChild(int child, int parent)
{
    downArray[child].setSibling(downArray[parent].getChild());
    downArray[parent].setChild(child);
}

/**************************************
installMaxReaches:  install the maximal reach pointer on each node
of the position heap.  The procedure must be run after the position
heap for the text has been constructed.  For a node corresponding at 
position i, find the maximal prefix of T[i, i-1, ... , 0] that is a path in
the heap.  Make the node's maximal reach pointer point to that node.
**************************************/
void heap::installMaxReaches()
{
    int pathNode, child;  // current node on path up, potential parent of 
                          //   new node
    int prevPathNode;  // child of pathNode on way up
    int depth;    // dummy parameter for indexIntoTrie

    pathNode = indexIntoTrie (text, 1, depth);
    maxReach[ROOT] = pathNode;
    for (int arrayIndex = 1; arrayIndex < textLength; arrayIndex++)
    {
        if ((arrayIndex % 100000) == 0) 
               cout << "Text position: " << arrayIndex << '\n';
        
       char c = text[arrayIndex];

       // Starting at the most recently added node, climb in the primal 
       // position heap until you find a child on the new letter c in 
       // the dual heap.  That child is the longest prefix of 
       // text[arrayIndex..0] that is already a node of the primal 
       // heap.  The new node must therefore be added as a child of 
       // it in the primal heap.  In the dual heap, the new node must 
       // be added to the highest node you saw that doesn't have a 
       // child on letter 'c'.

       // climb ...
       child = childOnLetter(pathNode, 0, c);
       while (child == NOCHILD)
       {
           prevPathNode = pathNode;
           pathNode = parent[pathNode];
           child = childOnLetter(pathNode, 0, c);
       }
           
       pathNode = child;
       maxReach[arrayIndex] = pathNode;
    }

}


/**************************************
search:  find positions of all occurrences of pattern in text.  The
procedure returns them as a set of positions of the text, where
the positions are numbered in descending order from left to right,
with position 0 being the rightmost.  The positions are stored in
a 'mylist' structure, which the procedure allocates, and which must
be deleted by the user when it is no longer needed, to avoid a memory
leak.

Here's how it works.

For notational convenience, we consider a node of the position heap to 
be synonymous with the corresponding position in the text.  When we say that
one position is an "ancestor" of another.  A position is an "occurrence" of 
a pattern string if it is the leftmost position of a substring of the text 
that is equal to the pattern string.  

Similarly, we will sometimes treat a node as synonymous with the string 
of edge labels from the root to the node.  This allows us to talk about 
whether a string "occurs" in the heap, whether one string is an 
"ancestor" of another, for example.

Indexing into the heap on a string that does not occur in the heap
"falls off the tree," that is, after indexing on a prefix of the string,
we find that we cannot continue, because the next letter of the string is not
the label of any edge to a child.

Recall that the naive search algorithm indexes as far as possible into
the text on the pattern string.  Let z be the last node on this path.
For each of the at-most m ancestors of z, it takes O(m) worst-case time 
to determine whether the pattern occurs there, for a total of O(m^2) time.  

If the pattern doesn't fall off the tree, then the occurrences of the 
pattern are z and its descendants, as well as those ancestors of z that are 
occurrences of the pattern.  It takes O(k) time to report the descendants.  
For each ancestor, it takes O(m) time to check the position to see if it is 
an occurrence of the pattern.  Since there are at most m-1 ancestors of z, 
the checking of ancestors takes O(m^2) time.  This gives the O(m^2+k) time 
bound for the naive search algorithm.

If the pattern does fall off the tree, then none of the proper descendants
of z is an occurrence of the pattern, since the path to them doesn't
fall off the tree at z.  The step of reporting the proper descendants of z is
omitted, giving an O(m^2) bound, and yielding O(m) occurrences of the
pattern.

The obstacle to getting an O(m+k) bound is therefore checking which ancestors
of z are occurrences of the pattern string.  

The O(m+k) approach implemented here works as follows.  

The maximal-reach pointer is defined as follows.  Compute
the position heap for the text.  Then, for each node x, find the position
i corresponding to x.  Let T[i,i-1, ..., 0] denote the suffix of the text 
starting at position i.  Index as far as possible into the
heap on this suffix, that is, find the maximum prefix of T[i,i-1,...,0]
that is the sequence of edge labels of a downward path from the root in the 
heap.  Let y be the node at the end of this path. Let x's maximal-reach pointer 
point to y.

The maximal-reach pointers and the discovery/finishing times provide a means 
of testing whether an ancestor position is an occurrence of the pattern in 
O(1) time, *provided the pattern doesn't fall off the tree.* This brings the 
time down to O(m+k) if the pattern doesn't fall off the tree.  The test works 
as follows: It is easy to see that if i is an ancestor of z, then i is an 
occurrence of the pattern if and only if the maximal-reach pointer points to 
a (not necessarily proper) descendant w of z.  This is the case if and only 
if the DFS discovery time of z is earlier than that of w and the finishing is 
later than that of w.

Henceforth, assume that the pattern does fall off the tree.  We reduce the
problem to queries on a series of patterns that do not fall off the tree.

Let XYcZ be an arbitrary string.  Let us say that this occurrence of Y is 
"maximal" in the string if Y is a node of the heap, but Yc is not.  
The key observation is that if the occurrence of Y is maximal, then
*only ancestors of Y that are also maximal occurrences of Y in the text 
are candidates to be occurrences of Yc in the text.*  Occurrences of Y
that are descendants of Y are followed by some other character than c.
Abusing terminology somewhat, let us call these "Y's candidate positions"; 
implicit in this terminology is a particular instance of Y in XYcZ that 
is understood.  

We seek all places where Y is preceded by X and followed by cZ; adding
|X| to these positions gives all positions where the pattern occurs.  This 
happens only at candidate positions, but not at all candidate
positions.  Since all candidate positions are ancestors of Y, Y has at most 
|Y| candidate positions.

Recall that the text positions are numbered in descending order from left to
right.  We divide the pattern into the concatenation X_1X_2, ..., X_j, where
for all i < j, X_i is maximal in the pattern.  We find the at-most 
|X_1| candidate positions for X_1.  For each such candidate h, we find 
whether h - |X_1| is a candidate for X_2.  Then if h passes this test, 
we find whether h - |X_1X_2| is a candidate for X_3, etc.  The final step 
is to find whether h - |X_1X_2...X_{j-1}| is an arbitrary occurrence of 
|X_j|, not just a candidate, since X_j is not maximal in the pattern.  
Then h is an occurrence of the pattern if and only if it passes all of these 
tests.

There are at most |X_1| candidates for X_1, and it takes O(|X_1|) time to find
a list of them, using the maximal-reach pointers, as described above for the
case where the pattern does not fall off the tree.  For candidate h, it takes 
O(1) time to find whether h - |X_1| is a candidate for X_2, also using the
maximal-reach pointers in this way.  If not, h is pruned from the list.
Pruning the list therefore takes O(|X_1|) time.  The resulting list has 
at most |X_2| candidates.  For each element of this list, it takes O(1) time
to find whether h - |X_1X_2| is a candidate for X_3, and prune it
from the list if it is not.  This pruning of the list takes O(|X_2|) time, 
etc.  At the last step, we have at most |X_{j-1}| candidates left in the
list.  It takes O(1) for each remaining candidate h, it takes O(1)
time to find whether h - |X_1X_2...X_{j-1}| is an occurrence of X_j;
this happens if and only if h is a descendant of X_j or an ancestor of X_j 
that is an occurrence of X_j.

The total time is therefore O(|X_1X_2...X_j|) which is O(m), yielding
all O(m) occurrences of the pattern in the case where the pattern falls
off the tree.
**************************************/

mylist *heap::search(char *pattern, int patternLength)
{
    //  observe convention of making indices descend from left to right
    reverse (pattern, patternLength); 

    int pathEndDepth; // end of indexing path for X_1
    // Get the positions of X_1 if it does not fall off the tree; otherwise
    //  get its candidate positions ...
    mylist *candidates = genCandidates (pattern, patternLength, pathEndDepth);
    bool fellOffTree = (pathEndDepth < patternLength);
    
    // If X_1 fell off the tree, we are done ...
    if (!fellOffTree) 
        candidates->compact();
    else 
    {
        // Cycle through X_2, X_3, ... X_j, pruning candidates as
        //  described above ...
        int offset = pathEndDepth;    
        while (offset < patternLength && candidates->size() > 0)
            candidates = pruneCandidates(pattern, patternLength-offset, 
                                          candidates, offset);
    }
   
    // un-reverse the user's pattern string to leave it in its original state
    reverse (pattern, patternLength); 
    return candidates;
}

/**************************************
genCandidates:  (See heap::search for terminology.)  Return the set of 
positions of the pattern string if it doesn't fall off the tree; find 
the maximal prefix X1 and its candidate positions otherwise.  Indexing 
on X_1 leads to a node, 'pathEndNode'.  Set 'pathEndDepth' to record the 
depth of 'pathEndNode', since this is |X_1|, which the caller needs to know.

If the pattern does not fall off the tree, then the set of positions of the 
pattern are the descendants of 'pathEndNode' and the ancestors whose 
maximal-reach pointers point to a (not necessarily proper) descendant 
of 'pathEndNode'.  As we have seen in the naive search algorithm,
every occurrence of the pattern is either an ancestor or a descendant
of 'pathEndNode', so that finishes the search.

Otherwise, X_1 is maximal in the pattern string.  The candidates of X_1 are
those ancestors of 'pathEndNode' that are occurrences of X_1.

The procedure allocates the 'mylist' structure for returning the list
of positions; this must be deleted by the caller to avoid a memory leak.  
It also sets the parameter 'pathEndDepth' to be the depth of 'pathEndNode', 
since this is also |X_1|, and the caller needs to know |X_1|.  
**************************************/
mylist *heap::genCandidates(char *pattern, int patternLength, int &pathEndDepth)
{

   // index as far as possible on 'pattern' ...
   int pathEndNode = indexIntoTrie(pattern, patternLength, pathEndDepth);

   // Find all *proper* ancestors of pathEndNode that are occurrences of X_1
   mylist *candidates = pathOccurrences(pattern, patternLength, pathEndNode);

   // If didn't fall off tree during indexing, append all *not necessarily
   //  proper* descendants of pathEndNode
   if (pathEndDepth == patternLength) 
       appendSubtreeOccurrences(pathEndNode, candidates);

   // pathEndNode is a non-proper descendant of itself that is an occurrence of
   //  X_1, so it must be reported as a candidate, along with those reported 
   //  by pathOccurrences
   else candidates->add(pathEndNode);  
                                      
   return candidates;
}

/**************************************
pruneCandidates:  (See heap::search comments for terminology.)  The pattern
string is X_1X_2...X_j.  The 'suffix' parameter is the string 
X_iX_{i+1}...X_j for some i > 1.  The 'candidates' parameter is all 
positions of X_1X_2...X_{i-1} in the text, minus a set of positions that 
are known not to be be followed by X_iX_{i+1}...X_j.  Since the pattern 
is X_1X_2...X_j, 'candidates' contains all positions where the pattern 
occurs, and possibly some where it does not.  The 'offset' parameter is 
|X_1X_2...X_{i-1}|.  Find X_i by indexing as far as possible into the trie 
on 'suffix'.  For each h in candidates, if i < j, throw h out of 'candidates' 
if h-'offset' is not a candidate for X_i, and if i=j, throw h out if 
h-'offset' is not an occurrence of X_j.  In either of these cases, h cannot 
be the position of an occurrence of the pattern.  

Therefore, if i < j, the final pruned candidate list is all occurrences 
X_1X_2...X_i, minus a set of positions that are known not to be followed by
by X_{i+1}X_{i+2}...X_j.  This differs from the candidate list at the beginning
of the call, where the candidate list was all occurrences of X_1X_2...X_{i-1}
that are known not to be followed by X_iX_{i+1}...X_j.  Therefore, the pruned 
candidate list contains all occurrences of the pattern, and possibly some 
others.   If i = j, the pruned candidate list is all occurrences of the 
pattern, X_1X_2...X_j.  

The procedure changes the 'offset' from |X_1X_2...X_{i-1}| to |X_1X_2...X_i| 
in preparation for the next iteration of 'pruneCandidates', where i will be
one step closer to j.
**************************************/

mylist *heap::pruneCandidates(char *suffix, int suffixLength, 
                              mylist *candidates, int &offset)
{
    int pathEndDepth;  // depth of end node of indexing path

    // index as far as possible into the heap on 'suffix' to find which
    // of its prefixes is X_i.  Set 'pathEndDepth=|X_i|
    int pathEndNode = indexIntoTrie(suffix, suffixLength, pathEndDepth);

    //  fellOffTree is true if we have found that i != j ...
    bool fellOffTree = (pathEndDepth < suffixLength);

    // Prepare a list to hold the positions where PX occurs
    mylist *newCandidates = new mylist();

    // If X_i-'pathEndDepth' is the empty string, i != j, the first letter 
    //  of 'suffix' does not occur in the text, so neither does the pattern.  We
    //  only need to return a nonempty set of candidates if this doesn't happen
    if (pathEndDepth > 0)
    {
        // for each h in 'candidates', keep h if it passes the test ...
        for (int index = 0; index < candidates->size(); index++)
        {
            int h = candidates->getElement(index);
            int offsetNode = h - offset;
  
            // if we haven't run off the righthand end of the text ...
            if (offsetNode >= 0)  
            {
                if   (
                      // h-'offset' is an ancestor of X_i that is an occurrence
                      //   of X_i
                      (isDescendant (pathEndNode, offsetNode) 
                         &&(isDescendant(maxReach[offsetNode], pathEndNode)))

                      //OR i=j and h-'offset' is a descendant of X_j, hence
                      //  an occurrence of it that isn't an ancestor ...
                      || (!fellOffTree 
                         && isDescendant (offsetNode, pathEndNode)))

                   // THEN keep h ...
                   newCandidates->add(candidates->getElement(index));
            }
        }
        // update 'offset' from |X_1X_2...X_{i-1}| to |X_1X_2...X_i| ...
        offset += pathEndDepth;  
    }
    delete candidates;
    return newCandidates;
}

/**************************************
indexIntoTrie:  Find the maximal prefix Q of 'pattern' that is the sequence
of edge labels on a path from the root in the position heap.  The returned
value is the last node on the indexing path, and the parameter 'endDepth'
tells the depth of this node, which is also the length of Q.

Recall that the indices of 'pattern' decrease from right to left, so
it is implemented with a pointer its *rightmost* character.
**************************************/
int heap::indexIntoTrie(char *pattern, int patternLength, 
                        int &endDepth)
{
    int pathNode;      // current node on the indexing path
    int child = ROOT;  // child of 'pathNode', except at beginning, when
                       //  it is the root and 'pathNode' is undefined
    int depth = 0;     // current depth
    if (patternLength == 0) return ROOT;
    else
    {

        //  Get a pointer to the leftmost position of 'pattern' 
        char *patPtr = pattern + patternLength - 1;
        do
        {
            pathNode = child;
            // get child of pathNode reachable on next letter of 'pattern'
            child = childOnLetter(pathNode, depth++, *patPtr--);
        } while (child != NOCHILD && depth < patternLength);

        // If we fell off the tree when trying to find 'child', 'pathNode' 
        //   is the last node of the indexing path
        if (child == NOCHILD) 
        {
           endDepth = depth - 1;
           return pathNode;
        }
        else // we reached the end of 'pattern, so 'child' is the end of path
        {
           endDepth = depth;
           return child;
        }
    }
}
/****************************
// childOnLetter:  Find the child reachable from 'node' on character c; 
//  'nodeDepth' is the depth of 'node'
******************************/
int heap::childOnLetter(int node, int nodeDepth, char c)
{
   int child = downArray[node].getChild();
   while (child != NOCHILD && text[child - nodeDepth] != c)
      child = downArray[child].getSibling();
   return child;
}

/**************************************
pathOccurrences:  Report all proper ancestors of pathEndNode whose maximal
reach pointers point to (not necessarily proper) descendants of 'pathEndNode'.

The method allocates the list of positions; this must be deleted
when it is no longer needed.
**************************************/
mylist *heap::pathOccurrences(char *pattern, int patternLength, int pathEndNode)
{
    int pathNode, child;  // parent and child on indexing path
    int depth;            // depth of pathNode
    mylist *Occurrences = new mylist(); 
    if (! Occurrences) {cout << "Memory allocation failure in pathOccurrences\n"; exit(1);}
    
    child = depth = 0;
    // start at "left" end of pattern (right-to-left indexing)
    char *patPtr = pattern + patternLength - 1;
                                           
    do
    {
        pathNode = child;
        if (isDescendant (maxReach[pathNode], pathEndNode))
            Occurrences->add(pathNode);
        child = childOnLetter(pathNode, depth++, *patPtr--);
    } while (child != pathEndNode);
    return Occurrences;
}

/*****************************
isDescendant:  tell whether node1 is a (not necessary proper) descendant of 
node2
******************************/
bool heap::isDescendant(int node1, int node2)
{
    return discoveryTime[node1] >= discoveryTime[node2] 
        && finishingTime[node1] <= finishingTime[node2];
               
}

/****************************
 *  If you didn't fall off the tree while indexing in on the pattern
 *  string, then all positions corresponding to descendants of the
 *  last node on the indexing path are also occurrences of the pattern.
 *  Append them to the list of places where the pattern string occurs.
*****************************/
void heap::appendSubtreeOccurrences(int node, mylist *Occurrences)
{
    Occurrences->add(node);   // append root of subtree
    for (int child = downArray[node].getChild();  //recursively append
            child != NOCHILD;                          //  descendants
            child = downArray[child].getSibling())
       appendSubtreeOccurrences (child, Occurrences);
}

/*************************
discoveryFinishing:  label all nodes of the heap with their Depth-First Search
discovery and finishing times.
**************************/
void heap::setDiscoveryFinishing()
{
    int i = 0;
    setDFAux(0,0,i);
}

void heap::setDFAux (int index, int depth, int &i)
{
    if (index == -1) return;
    else
    {
       discoveryTime[index] = i++;
       for (int child = downArray[index].getChild(); 
                child != NOCHILD; 
                child = downArray[child].getSibling())
            setDFAux(child, depth+1, i);
       finishingTime[index] = i++;
    }
}

/***********************
// preorderPrint:  Display the shape of the heap tree using indented preorder 
*************************/
void heap::preorderPrint()
{
    preorderAux(0,0);
}

void heap::preorderAux (int index, int depth)
{
    if (index == -1) return;
    else
    {
       for (int i = 0; i < depth; i++)
          cout << ' ';
       cout << "Node " << index << "  Depth " << depth;
       cout << " max reach: " << maxReach[index];
       cout << " discovery: " << discoveryTime[index];
       cout << " finish: " << finishingTime[index];
       cout << "  Children: ";
       for (int child = downArray[index].getChild(); 
                child != NOCHILD; 
                child = downArray[child].getSibling())
             cout << '(' << text[child-depth] << ',' << child << ')';
       cout << '\n';
       for (int child = downArray[index].getChild(); 
                child != NOCHILD; 
                child = downArray[child].getSibling())
            preorderAux(child, depth+1);
    }
}



int heap::getTextLength()
{
    return textLength;
}

