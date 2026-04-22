/* ========================================================================= *
 * BST definition
 * ========================================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

#include "BST.h"
#include "List.h"

/* Opaque Structure */

typedef struct BNode_t BNode;

struct BNode_t
{
    BNode *parent;
    BNode *left;
    BNode *right;
    void *key;
    void *value;
};

struct BST_t
{
    BNode *root;
    size_t size;
    int (*compfn)(void *, void *);
};

typedef struct Pair_t
{
    long sum;
    size_t n;
} Pair;

typedef struct kvpair_t
{
    void *key;
    void *value;
} KVpair;

/* Prototypes of static functions */

static void bstFreeRec(BNode *n, bool freeKey, bool freeValue);
static BNode *bnNew(void *key, void *value);
static size_t bstHeightRec(BNode *root);
static BNode *bnMin(BNode *n);
static BNode *successor(BNode *n);

size_t calcBstAverageNodeDepth(BNode *node, size_t *nbrNode, size_t Depth);
BNode *bstRangeSearchKeymin(BNode *node, void *keymin, int (*compfn)(void *, void *));
void mergeSortKV(KVpair *pairList,KVpair *temp, int comparison_fn_t(void *, void *),size_t p, size_t q);
BNode *buildOptBst(KVpair *pairList,BNode *parent, size_t p, size_t q);


BNode *bnNew(void *key, void *value)
{
    BNode *n = malloc(sizeof(BNode));
    if (n == NULL)
    {
        fprintf(stderr, "bnNew: allocation error\n");
        exit(1);
    }
    n->left = NULL;
    n->right = NULL;
    n->parent = NULL; // modif annonce prof
    n->key = key;
    n->value = value;
    return n;
}

BST *bstNew(int comparison_fn_t(void *, void *))
{
    BST *bst = malloc(sizeof(BST));
    if (bst == NULL)
    {
        fprintf(stderr, "bestNew: allocation error");
        exit(1);
    }
    bst->root = NULL;
    bst->size = 0;
    bst->compfn = comparison_fn_t;
    return bst;
}

void bstFree(BST *bst, bool freeKey, bool freeValue)
{
    bstFreeRec(bst->root, freeKey, freeValue);
    free(bst);
}

void bstFreeRec(BNode *n, bool freeKey, bool freeValue)
{
    if (n == NULL)
        return;
    bstFreeRec(n->left, freeKey, freeValue);
    bstFreeRec(n->right, freeKey, freeValue);
    if (freeKey)
        free(n->key);
    if (freeValue)
        free(n->value);
    free(n);
}

size_t bstSize(BST *bst)
{
    return bst->size; 
}

static size_t bstHeightRec(BNode *root)
{
    if (!root)
        return 0;

    size_t hleft = bstHeightRec(root->left);
    size_t hright = bstHeightRec(root->right);
    if (hleft > hright)
        return 1 + hleft;
    else
        return 1 + hright;
}

size_t bstHeight(BST *bst)
{
    return bstHeightRec(bst->root) -1 ; //modif prof
}

bool bstInsert(BST *bst, void *key, void *value)
{
    if (bst->root == NULL)
    {
        bst->root = bnNew(key, value);
        if (bst->root == NULL)
        {
            return false;
        }
        bst->size++;
        return true;
    }
    BNode *prev = NULL;
    BNode *n = bst->root;
    while (n != NULL)
    {
        prev = n;
        int cmp = bst->compfn(key, n->key);
        if (cmp <= 0)
        {
            n = n->left;
        }
        else if (cmp > 0)
        {
            n = n->right;
        }
    }
    BNode *new = bnNew(key, value);
    if (new == NULL)
    {
        return false;
    }
    new->parent = prev;
    if (bst->compfn(key, prev->key) <= 0)
    {
        prev->left = new;
    }
    else
    {
        prev->right = new;
    }
    bst->size++;
    return true;
}

void *bstSearch(BST *bst, void *key)
{
    BNode *n = bst->root;
    while (n != NULL)
    {
        int cmp = bst->compfn(key, n->key);
        if (cmp < 0)
        {
            n = n->left;
        }
        else if (cmp > 0)
        {
            n = n->right;
        }
        else
        {
            return n->value;
        }
    }
    return NULL;
}

static BNode *bnMin(BNode *n)
{
    while (n->left != NULL)
        n = n->left;
    return n;
}

static BNode *successor(BNode *n)
{
    if (n->right != NULL)
        return bnMin(n->right);
    BNode *y = n->parent;
    BNode *x = n;
    while (y != NULL && x == y->right)
    {
        x = y;
        y = y->parent;
    }
    return y;
}

// ----------------------------------------------------------------------------------
// The functions below have to be implemented

BST *bstOptimalBuild(int comparison_fn_t(void *, void *), List *lkeys, List *lvalues) //finito
{
    if(!lkeys || !lvalues)
    {
        return NULL;
    }

    LNode *keyNode = lkeys->head;
    LNode *valNode = lvalues->head;

    size_t nbrKeys = listSize(lkeys);
    size_t nbrValues = listSize(lvalues);

    

    if(nbrKeys != nbrValues)
    {
        return NULL;
    }

    KVpair *pairList = malloc(nbrValues*sizeof(KVpair));

    if(!pairList)
    {
        return NULL;
    }

    for(size_t i = 0; i < nbrKeys; i++)
    {
        pairList[i].value = valNode->value;
        pairList[i].key = keyNode->value;

        valNode = valNode->next;
        keyNode = keyNode->next;
    }

    KVpair *temp = malloc(nbrValues * sizeof(KVpair));

    if(!temp)
    {
        return NULL;
    }

    mergeSortKV(pairList, temp, comparison_fn_t, 0, nbrValues-1);

    free(temp);

    BST *bst = bstNew(comparison_fn_t);

    bst->root = buildOptBst(pairList,NULL,0,nbrValues-1);

    return bst;
}

void mergeSortKV(KVpair *pairList,KVpair *temp, int comparison_fn_t(void *, void *),size_t p, size_t q)
{
    if(p < q)
        {
            size_t m = p + (q - p)/2;
            mergeSortKV(pairList, temp, comparison_fn_t, p, m);
            mergeSortKV(pairList, temp, comparison_fn_t, m + 1 , q);

            int comp;
            size_t i = p;
            size_t j = m + 1; 

            for(size_t k = p; k <= q; k++)
            {
                if(i == m + 1)
                {
                    temp[k] = pairList[j];
                    j++;
                    continue;
                }

                if(j == q+1)
                {
                    temp[k] = pairList[i];
                    i++;
                    continue;
                }

                comp = comparison_fn_t(pairList[i].key,pairList[j].key);

                if(comp <= 0) 
                {
                    temp[k] = pairList[i];
                    i++;
                }

                if(comp > 0)
                {
                    temp[k] = pairList[j];
                    j++;
                }
            }

            for (size_t k = p; k <= q; k++)
            {
                pairList[k] = temp[k];
            }
        }

    return;
}

BNode *buildOptBst(KVpair *pairList,BNode *parent, size_t p, size_t q) 
{
    if(p>q)
    {
        return NULL;
    }

    size_t m = p + (q - p)/2;

    BNode *node = bnNew(pairList[m].key,pairList[m].value);
    if(!node)
    {
        return NULL;
    }

    node->parent = parent;

    if(p < m)
    {
        node->left = buildOptBst(pairList,node,p,m-1);
    }

    node->right = buildOptBst(pairList,node,m+1,q);

    return node;
}

List *bstRangeSearch(BST *bst, void *keymin, void *keymax) //finito
{
    List *list = listNew();
    
    if(!bst || !bst->root)
    {
        return list;
    }

    BNode *node = bstRangeSearchKeymin(bst->root, keymin, bst->compfn);

    if(!node)
    {
        return list;
    }

    for(;node && bst->compfn(node->key,keymax) <= 0; node = successor(node))
    {
        listInsertLast(list,node->value);
    }
    
    return list;
}

BNode *bstRangeSearchKeymin(BNode *node, void *keymin, int (*compfn)(void *, void *))
{
    if(!node)
    {
        return NULL;
    }

    if(compfn(node->key,keymin) < 0)    
    {
        return bstRangeSearchKeymin(node->right,keymin,compfn);
    }

    else
    {
        BNode *test = bstRangeSearchKeymin(node->left,keymin,compfn);
        if(test)
        {
            return test;
        }
        return node;
    }
}

double bstAverageNodeDepth(BST *bst) //finito
{
    if(!bst || !bst->root)
    {
        return -1;
    }

    size_t nbrNode = 0;
    size_t Depth = 0;


    size_t sumDepth = calcBstAverageNodeDepth(bst->root, &nbrNode, Depth);

    double result = (double) sumDepth/nbrNode;
    return result;
}

size_t calcBstAverageNodeDepth(BNode *node, size_t *nbrNode, size_t Depth)
{
    (*nbrNode)++;
    size_t sumLeft = 0;
    size_t sumRight = 0;

    if(node->left)
    {
        sumLeft = calcBstAverageNodeDepth(node->left,nbrNode, Depth+1);
    }

    if(node->right)
    {
        sumRight = calcBstAverageNodeDepth(node->right, nbrNode, Depth+1);
    }

    return sumLeft + Depth + sumRight;
}
