/******************************************************************************
 *
 * $Id$
 *
 * Project:  Optimization Modeling of Fire Fighter Resources
 * Purpose:  Generic tree for dispatching
 * Author:   Kyle Shannon <kyle@pobox.com>
 *
 ******************************************************************************
 *
 * THIS SOFTWARE WAS DEVELOPED AT THE ROCKY MOUNTAIN RESEARCH STATION (RMRS)
 * MISSOULA FIRE SCIENCES LABORATORY BY EMPLOYEES OF THE FEDERAL GOVERNMENT 
 * IN THE COURSE OF THEIR OFFICIAL DUTIES. PURSUANT TO TITLE 17 SECTION 105 
 * OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO COPYRIGHT 
 * PROTECTION AND IS IN THE PUBLIC DOMAIN. RMRS MISSOULA FIRE SCIENCES 
 * LABORATORY ASSUMES NO RESPONSIBILITY WHATSOEVER FOR ITS USE BY OTHER 
 * PARTIES,  AND MAKES NO GUARANTEES, EXPRESSED OR IMPLIED, ABOUT ITS QUALITY, 
 * RELIABILITY, OR ANY OTHER CHARACTERISTIC.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#ifndef OMFFR_TREE_H_
#define OMFFR_TREE_H_

#include <stdlib.h>
#include <iostream>
#include <vector>

/**
 * \class Node
 * \brief Generic Node for a general tree
 *
 * Store parent, children and data type.  This class and the following are used
 * to represent the dispatch structure in IRS, but could be used elsewhere.
 */
template<class T>
class OmffrNode
{
//private:
public:
    OmffrNode<T>(T data);
    OmffrNode<T>(const OmffrNode<T>&rhs);
    OmffrNode<T>(OmffrNode<T> *parent);
    OmffrNode<T> *parent;
    std::vector<OmffrNode<T>*>children;
    T data;
};

/**
 * \brief Create a Node with some data.
 *
 * Initialize parent to NULL and let the tree handle that, create a vector to
 * store children
 */
template<class T>
OmffrNode<T>::OmffrNode(T data)
{
    this->data = data;
    parent = NULL;
    children = std::vector<OmffrNode<T>*>();
}

template<class T>
OmffrNode<T>::OmffrNode(const OmffrNode<T>&rhs)
{
    data = rhs.data;
    children = rhs.children;
}

/**
 * \class OmffrTree
 * \brief A general tree of templated Nodes
 */
template<class T>
class OmffrTree
{
public:
    OmffrTree();
    OmffrTree(T data);
    OmffrTree(const OmffrTree &rhs);
    OmffrTree<T> operator = (const OmffrTree<T> &rhs);
    ~OmffrTree();

    int Size();
    bool Empty();

    T Replace(OmffrNode<T> *n, T data);
    T Element(OmffrNode<T> *n);

    OmffrNode<T>* Root();
    OmffrNode<T>* Parent(OmffrNode<T> *n);
    std::vector<OmffrNode<T>*>Siblings(OmffrNode<T> *n);
    std::vector<OmffrNode<T>*>Preorder(OmffrNode<T> *n);
    std::vector<OmffrNode<T>*>Postorder(OmffrNode<T> *n);
    //std::vector<OmffrNode<T>*>Nodes();
    std::vector<OmffrNode<T>*>Children(OmffrNode<T> *n);
    std::vector<OmffrNode<T>*>GetExternalNodes(OmffrNode<T> *n, int level);

    bool External(OmffrNode<T> *n);
    bool Internal(OmffrNode<T> *n);
    bool IsRoot(OmffrNode<T> *n);
    bool IsOrphan(OmffrNode<T> *n);

    OmffrNode<T>* AddChild(OmffrNode<T> *n, T data);
    OmffrTree<T> Detach(OmffrNode<T> *n);
    int RemoveTree(OmffrNode<T> *n);

    int Height(OmffrNode<T> *n);
    int Depth(OmffrNode<T> *n);

    void Print(OmffrNode<T> *n);
private:
    int count;
    OmffrNode<T> *root;
};

/**
 * \brief Create a new, empty tree
 */
template<class T>
OmffrTree<T>::OmffrTree()
{
    root = NULL;
    count = 0;
}

/**
 * \brief Copy Constructor
 *
 * Create a complete copy of a tree.
 */
template<class T>
OmffrTree<T>::OmffrTree(const OmffrTree &rhs)
{
    root = new OmffrNode<T>(rhs.root);
}


/**
 * \brief Create a new tree with some data for the root node.
 *
 * \param root_data data to add as the root, generic
 */
template<class T>
OmffrTree<T>::OmffrTree(T root_data)
{
    root = new OmffrNode<T>(root_data);
    count = 1;
}

/**
 * \brief Destroy the tree and all of the nodes
 */
template<class T>
OmffrTree<T>::~OmffrTree()
{
    if(Size() > 0 && root != NULL)
    {
        std::vector<OmffrNode<T>*> nodes = Preorder(root);
        for(int i = 0;i < nodes.size();i++)
            delete nodes[i];
    }
}

/**
 * \brief Equal operator
 */
template<class T>
OmffrTree<T> OmffrTree<T>::operator=(const OmffrTree &rhs)
{
    return OmffrTree();
}


/**
 * \brief Remove a node and all of the children of that node
 *
 * This removes any 'sub-tree' of the tree from node n.  An iterable is
 * obtained of all children of n using Preorder() and they are freed from
 * memory.  The node is then removed from its parent's children list (using
 * Siblings())and finally removed from memory itself.
 *
 * \param n node to start removal at
 * \return size of tree after removal
 */
template<class T>
int OmffrTree<T>::RemoveTree(OmffrNode<T> *n)
{
    std::vector<OmffrNode<T>*> nodes = Preorder(n);
    for(int i = 0;i < nodes.size();i++)
        delete nodes[i];
    Parent(n)->children = Siblings(n);
    delete n;
    n = NULL;
    count -= nodes.size();
    return --count;
}

/**
 * \brief Get the number of nodes in the tree
 *
 * \return size of the tree (number of nodes)
 */
template<class T>
int OmffrTree<T>::Size()
{
    return count;
}

/**
 * \brief Test if the tree is empty
 *
 * \return true if there are no Nodes, false otherwise
 */
template<class T>
bool OmffrTree<T>::Empty()
{
    return (Size() == 0);
}

/**
 * \brief replace the element of a node with some data
 *
 * \param n node to act upon
 * \param data data to replace current data in node n
 * \return old element data
 */
template<class T>
T OmffrTree<T>::Replace(OmffrNode<T> *n, T data)
{
    T tmp = n->data;
    n->data = data;
    return tmp;
}

/**
 * \brief Fetch the data for a node
 *
 * \param n node to act upon
 * \return data at node n
 */
template<class T>
T OmffrTree<T>::Element(OmffrNode<T> *n)
{
    return n->data;
}

/**
 * \brief Get the root node for the tree.
 *
 * \return a pointer to the root node
 */
template<class T>
OmffrNode<T> * OmffrTree<T>::Root()
{
    return root;
}

/**
 * \brief Get the parent of a given node
 *
 * \param n node to act upon
 * \return a pointer to the parent of node n
 */
template<class T>
OmffrNode<T> * OmffrTree<T>::Parent(OmffrNode<T> *n)
{
    return n->parent;
}

/**
 * \brief Get a list of direct siblings for a node
 *
 * This returns a new vector without the node itself in the vector.  It is the
 * callers responsibility to destroy the node if necessary (as is the case in
 * RemoveTree() where the list of the node's parent's child vector needs to be
 * altered).  Siblings are considered to be nodes that have the same depth, and
 * have the same parent.
 *
 * \param n node for sibling list to be formed from
 * \return a vector of siblings.
 */
template<class T>
std::vector<OmffrNode<T>*> OmffrTree<T>::Siblings(OmffrNode<T> *n)
{
    std::vector<OmffrNode<T>*> nodes = Children(Parent(n));
    int i;
    while(i < nodes.size())
    {
        if(nodes[i] == n)
            break;
        i++;
    }
    nodes.erase(nodes.begin() + i);
    i = 0;
    return nodes;
}

/**
 * \brief Convenience for checking if a node is orphaned.
 *
 * If a node has no parent, and it isn't the root node, it maybe an orphan,
 * meaning there is no way to get to it.  These should be destroyed using
 * RemoveTree() or by hand.  I don't know when this would/should/could be used,
 * but in a pinch, if a nod loses it's parent, we leak memory.
 *
 * \param node to test orphanage residence
 * \return TRUE if the node is not null, has no parent, and is *not* the root
 */
template<class T>
bool OmffrTree<T>::IsOrphan(OmffrNode<T> *n)
{
    return ((n != NULL) && !IsRoot(n) && (Parent(n) == NULL));
}

/**
 * \brief Get an iterable of *all* of the nodes in a tree
 *
 * Use a preorder traversal to get all of the nodes
 *
 * \todo add node n to vector???
 *
 * \param n node to start traversal at
 * \return a vector of all nodes under
 */
template<class T>
std::vector<OmffrNode<T>*> OmffrTree<T>::Preorder(OmffrNode<T> *n)
{
    std::vector<OmffrNode<T>*> nodes;
    std::vector<OmffrNode<T>*> add_nodes;
    nodes.push_back(n);
    for(int i = 0;i < n->children.size();i++)
    {
        add_nodes = Preorder(n->children[i]);
        nodes.insert(nodes.end(), add_nodes.begin(), add_nodes.end());
    }
    return nodes;
}

/**
 * \brief Get an iterable of *all* of the nodes in a tree
 *
 * Use a postorder traversal to get all of the nodes
 *
 * \todo add node n to vector???
 *
 * \param n node to start traversal at
 * \return a vector of all nodes under
 */
template<class T>
std::vector<OmffrNode<T>*> OmffrTree<T>::Postorder(OmffrNode<T> *n)
{
    std::vector<OmffrNode<T>*> nodes;
    std::vector<OmffrNode<T>*> add_nodes;
    for(int i = 0;i < n->children.size();i++)
    {
        add_nodes = Postorder(n->children[i]);
        nodes.insert(nodes.end(), add_nodes.begin(), add_nodes.end());
    }
    nodes.push_back(n);
    return nodes;
}

/**
 * \brief Test if a node is the root of the tree
 *
 * This test probably doesn't need the second check, but it doesn't hurt
 * anything
 *
 * \param n node to test
 * \return true if the node is the root, false otherwise
 */
template<class T>
bool OmffrTree<T>::IsRoot(OmffrNode<T> *n)
{
    return (root == n && parent(n) == NULL);
}

/**
 * \brief Add a child to a node
 *
 * Create a new node and set the element, then add it to the list of children
 * for node n.  If NULL is passed for n, then set it as the root of the tree
 * only if the root has not been set.  If the tree has a root, don't do
 * anything and return NULL.
 *
 * \param n node to add a child to
 * \param element data for the new node
 * \return pointer to the new node
 */
template<class T>
OmffrNode<T>* OmffrTree<T>::AddChild(OmffrNode<T> *n, T element)
{
    if(n == NULL && root != NULL)
        return n;

    OmffrNode<T> *new_node = new OmffrNode<T>(element);
    if(root == NULL)
        root = new_node;
    else
    {
        new_node->parent = n;
        n->children.push_back(new_node);
    }
    count++;
    return new_node;
}

/**
 * \brief Test to check if the node is external
 *
 * External is defined as a node having no children.
 *
 * \param n node to test
 * \return true if the node has no children, false otherwise
 */
template<class T>
bool OmffrTree<T>::External(OmffrNode<T> *n)
{
    return (n->children.size() == 0);
}

/**
 * \brief Test to check if the node is internal
 *
 * Internal is defined as a node having one or more children.
 *
 * \param n node to test
 * \return true if the node has children, false otherwise
 */
template<class T>
bool OmffrTree<T>::Internal(OmffrNode<T> *n)
{
    return !External(n);
}

/**
 * \brief Get the depth of the node
 *
 * The depth is defined as the number of ancestors a node has.  The root has a
 * depth of 0, the children of root have a depth of 1, etc.
 *
 * \param n node to check
 * \return depth of node n
 */
template<class T>
int OmffrTree<T>::Depth(OmffrNode<T> *n)
{
    OmffrNode<T>* p = Parent(n);
    if(p == NULL)
        return 0;
    else
        return 1 + Depth(p);
}

/**
 * \brief Get a list of children for a given node
 *
 * \param n node to act upon
 * \return a list of node n's children
 */
template<class T>
std::vector<OmffrNode<T>*> OmffrTree<T>::Children(OmffrNode<T> *n)
{
    return n->children;
}

template<class T>
int OmffrTree<T>::Height(OmffrNode<T> *n)
{
    if(External(n))
        return 0;
    int h = 0;
    std::vector<OmffrNode<T>*> c = children(n);
    for(int i = 0;i < c.size();i++)
    {
        h = MAX(h, height(c[i]));
    }
    return 1 + h;
}

/**
 * \brief Fetch all external nodes for a given depth.
 *
 * Convenience function for accessing the Dispatch locations.  This allows us
 * to find all locations that are external and at a certain depth (in our case,
 * local dispatch locations will always be at level n=3, but it is left generic
 * just in case we find other uses.  In the convenience case, the node provided
 * will always be the root (can be NULL as well here), and the depth will be
 * absolute.  It is possible to use a sub-node of root, and the level is then
 * relative to the sub-node.  A negative level value will return all external
 * nodes in the children of n.
 *
 * \param n node to start traversal, NULL applies to root
 * \param level depth *relative* to the node provided, negative value retrieves
 *        all external nodes
 * \return vector of nodes that meet the search criteria
 */
template<class T>
std::vector<OmffrNode<T>*> OmffrTree<T>::GetExternalNodes(OmffrNode<T> *n,
                                                          int level)
{
    std::vector<OmffrNode<T>*> nodes;
    if(n == NULL)
        n = root;
    std::vector<OmffrNode<T>*> c = Preorder(n);
    for(int i = 0;i < c.size(); i++ )
    {
        if(level >= 0)
        {
            if(External(c[i]) && (Depth(c[i]) - Depth(n)) == level)
                nodes.push_back(c[i]);
        }
        else if(External(c[i]))
        {
            nodes.push_back(c[i]);
        }
    }
    return nodes;
}
/**
 * \brief Detach a subtree and create a new tree that represents the subtree
 *
 * XXX: Needs = operator implemented
 * \param n node to detach, it becomes the root of the new tree
 * \return a tree with n as the root
 */
template<class T>
OmffrTree<T> OmffrTree<T>::Detach(OmffrNode<T> *n)
{
    return OmffrTree();
}

/**
 * \brief Print the tree to stdout
 * \todo: overload << operator?
 * \param n node to start printing from
 * \return void
 */
template<class T>
void OmffrTree<T>::Print(OmffrNode<T> *n)
{
    for(int i = 0;i < Depth(n);i++)
        std::cout << "\t";
    std::cout << Element(n) << std::endl;
    for(int i = 0;i < n->children.size();i++)
    {
        Print(n->children[i]);
    }
}

#endif /* OMFFR_TREE_H_ */

