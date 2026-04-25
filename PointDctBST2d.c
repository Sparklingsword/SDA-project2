#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include <time.h>

#include "PointDct.h"

typedef struct BNode2d_t BNode2d;

struct BNode2d_t
{
    Point *point;
    void *value;
    BNode2d *parent;
    BNode2d *left;
    BNode2d *right;
};

typedef struct BST2d_t
{
    BNode2d *root;
    size_t size;
}BST2d;

typedef struct pvpair_t
{
    Point *point;
    void *value;
    size_t id;
    double xy;
} PVpair;

struct PointDct_t
{
	BST2d *bst;
};

double now(void);

BST2d *bst2dNew(void);
BNode2d *bn2dNew(Point *point, void *value);
void bst2dFree(BST2d *bst, bool freePoint, bool freeValue);
void bst2dFreeRec(BNode2d *n, bool freePoint, bool freeValue);
size_t bst2dSize(BST2d *bst);
size_t bst2dHeight(BST2d *bst);
static size_t bst2dHeightRec(BNode2d *root);
double bst2dAverageNodeDepth(BST2d *bst);
size_t calcBst2dAverageNodeDepth(BNode2d *node, size_t *nbrNode, size_t Depth);


size_t medianOfThree(PVpair *array, size_t p, size_t q);
void swapPV(PVpair *array, size_t i, size_t j);
void quickSort(PVpair *array, size_t p, size_t q);


int compare(double x1, double x2);


static inline void markLeftIds(PVpair *array, bool *isLeft, size_t p, size_t m);
void tempName(PVpair *array, PVpair med, PVpair *temp, bool *isLeft, size_t p, size_t q);
BNode2d *buildOptBst2d(PVpair *arraySortedX ,PVpair *arraySortedY, PVpair *temp, BNode2d *parent,size_t p, size_t q, bool axis, bool *isLeft);



void *pdctExactSearch(PointDct *pd, Point *p);
void *pdctExactSearchRec(BNode2d *node, Point *p,bool axis);

static inline bool isInBall(double x, double y, double xc, double yc, double rr);
List *pdctBallSearch(PointDct *pd, Point *p, double r);
void pdctBallSearchRec(BNode2d *node,double xc,double yc, double r, double rr, bool axis, List *result);



double now()
{
    return (double)clock() / CLOCKS_PER_SEC;
}

/*
    2 Dimensional Bst structure and useful functions for the bst2d
*/

BST2d *bst2dNew()
{
    BST2d *bst = malloc(sizeof(BST2d));
    if (bst == NULL)
    {
        fprintf(stderr, "bestNew: allocation error");
        exit(1);
    }
    bst->root = NULL;
    bst->size = 0;
    return bst;
}


BNode2d *bn2dNew(Point *point, void *value) 
{
    BNode2d *n = malloc(sizeof(BNode2d));
    if (n == NULL)
    {
        fprintf(stderr, "bnNew2d: allocation error\n");
        exit(1);
    }
    n->left = NULL;
    n->right = NULL;
    n->parent = NULL;
    n->point = point;
    n->value = value;
    return n;
}


void bst2dFree(BST2d *bst, bool freePoint, bool freeValue)
{
    bst2dFreeRec(bst->root, freePoint, freeValue);
    free(bst);
}

void bst2dFreeRec(BNode2d *n, bool freePoint, bool freeValue)
{
    if (n == NULL)
        return;
    bst2dFreeRec(n->left, freePoint, freeValue);
    bst2dFreeRec(n->right, freePoint, freeValue);
    if (freePoint)
        free(n->point);
    if (freeValue)
        free(n->value);
    free(n);
}


size_t bst2dSize(BST2d *bst)
{
    return bst->size;
}


static size_t bst2dHeightRec(BNode2d *root)
{
    if (!root)
        return 0;

    size_t hleft = bst2dHeightRec(root->left);
    size_t hright = bst2dHeightRec(root->right);
    if (hleft > hright)
        return 1 + hleft;
    else
        return 1 + hright;
}

size_t bst2dHeight(BST2d *bst)
{
    return bst2dHeightRec(bst->root);
}


double bst2dAverageNodeDepth(BST2d *bst) //finito
{
    if(!bst || !bst->root)
    {
        return -1;
    }

    size_t nbrNode = 0;
    size_t Depth = 0;


    size_t sumDepth = calcBst2dAverageNodeDepth(bst->root, &nbrNode, Depth);

    double result = (double) sumDepth/nbrNode;
    return result;
}

size_t calcBst2dAverageNodeDepth(BNode2d *node, size_t *nbrNode, size_t Depth)
{
    (*nbrNode)++;
    size_t sumLeft = 0;
    size_t sumRight = 0;

    if(node->left)
    {
        sumLeft = calcBst2dAverageNodeDepth(node->left,nbrNode, Depth+1);
    }

    if(node->right)
    {
        sumRight = calcBst2dAverageNodeDepth(node->right, nbrNode, Depth+1);
    }

    return sumLeft + Depth + sumRight;
}

/*
    Comparison and sorting functions
*/

int compare(double x1, double x2)
{
    if (x1 <  x2) return -1;
    else if (x1 >  x2) return 1;
    else return 0;
}


void swapPV(PVpair *array, size_t i, size_t j)
{
    PVpair temp = array[i];
    array[i] = array[j];
    array[j] = temp;
}


size_t medianOfThree(PVpair *array, size_t p, size_t q)
{
    size_t m = p + (q - p) / 2;

    if (compare(array[p].xy, array[m].xy) > 0)
        swapPV(array,p,m);

    if (compare(array[p].xy, array[q].xy) > 0)
        swapPV(array,p,q);

    if (compare(array[m].xy, array[q].xy) > 0)
        swapPV(array,m,q);

    return m;
}


void quickSort(PVpair *array, size_t p, size_t q)
{
    if (p >= q) return;

    size_t pivot = medianOfThree(array, p, q);
    swapPV(array, pivot, q);

    int i = (int)p;
    int j = (int)q - 1;
    int k = (int)p;

    while (k <= j)
    {
        int comp = compare(array[k].xy, array[q].xy);

        if (comp < 0)
        {
            swapPV(array, (size_t)k, (size_t)i);
            i++;
            k++;
        }
        else if (comp > 0)
        {
            swapPV(array, (size_t)k, (size_t)j);
            j--;
        }
        else
        {
            k++;
        }
    }

    swapPV(array, (size_t)(j + 1), q);

    if (i - 1 >= (int)p)
    {
        quickSort(array, p, (size_t)(i - 1));
    }

    if (j + 2 <= (int)q)
    {
        quickSort(array, (size_t)(j + 2), q);
    }
}



/*
    Dictionnary functions
*/


static inline void markLeftIds(PVpair *array, bool *isLeft, size_t p, size_t m)
{
    for(size_t i = p; i < m; i++)
    {
        isLeft[array[i].id] = true;
    }
}

void tempName(PVpair *array, PVpair med, PVpair *temp, bool *isLeft, size_t p, size_t q) 
{
    

    size_t leftSize = (q - p)/2;
    size_t i = 0;
    size_t j = leftSize + 1;

    for(size_t k = p; k <= q; k++)
    {
        if(array[k].id == med.id)
        {
            temp[leftSize] = array[k];
        }
        else if(isLeft[array[k].id])
        {
            temp[i] = array[k];
            isLeft[array[k].id] = false;
            i++;
        }
        else
        {
            temp[j] = array[k];
            j++;
        }
    }

    for(size_t k = 0; k < (q-p+1); k++)
    {
        array[p+k] = temp[k];
    }


}

BNode2d *buildOptBst2d(PVpair *arraySortedX, PVpair *arraySortedY, PVpair *temp, BNode2d *parent,size_t p, size_t q, bool axis, bool *isLeft)
{
    if(p > q)
    {
        return NULL;
    }

    size_t m = p + (q - p)/2;
    BNode2d *node;

    if(axis)
    {
        node = bn2dNew(arraySortedX[m].point, arraySortedX[m].value);

        markLeftIds(arraySortedX, isLeft, p, m);
        tempName(arraySortedY, arraySortedX[m], temp, isLeft, p, q);
        
    }
    else
    {
        node = bn2dNew(arraySortedY[m].point, arraySortedY[m].value);

        markLeftIds(arraySortedY, isLeft, p, m);
        tempName(arraySortedX, arraySortedY[m], temp, isLeft, p, q);

    }

    if(!node)
    {
        return NULL;
    }

    node->parent = parent;
    if(p < m) node->left = buildOptBst2d(arraySortedX, arraySortedY, temp, node, p, m-1, !axis, isLeft);
    
    node->right = buildOptBst2d(arraySortedX, arraySortedY, temp, node, m+1, q, !axis, isLeft);

    return node;
}

PointDct *pdctCreate(List *lpoints, List *Lvalues)
{
    printf("\n");
    double t0, t1;

    size_t size = listSize(Lvalues);
    
    if(listSize(lpoints) != size) return NULL;

    PointDct *pd = malloc(sizeof(PointDct));
	if(!pd) return NULL;

	LNode *pointNode = lpoints->head;
	LNode *valNode = Lvalues->head;

    PVpair *arraySortedX = malloc(size* sizeof(PVpair));
    PVpair *arraySortedY = malloc(size* sizeof(PVpair));
	
    t0 = now();

	for(size_t i = 0; pointNode != NULL; i++)
	{
        arraySortedX[i].point = pointNode->value;
        arraySortedX[i].value = valNode->value;
        arraySortedX[i].id = i;
        arraySortedX[i].xy = ptGetx(pointNode->value);

        arraySortedY[i].point = pointNode->value;
        arraySortedY[i].value = valNode->value;
        arraySortedY[i].id = i;
        arraySortedY[i].xy = ptGety(pointNode->value);

        pointNode = pointNode->next;
		valNode = valNode->next;
    }

    t1 = now();
    printf("Création arrays: %f s\n", t1 - t0);


    t0 = now();
    quickSort(arraySortedX, 0, size-1);
    t1 = now();
    printf("Tri X: %f s\n", t1 - t0);

    
    t0 = now();
    quickSort(arraySortedY, 0, size-1);
    t1 = now();
    printf("Tri Y: %f s\n", t1 - t0);


    BST2d *bst = bst2dNew();

    bool *isLeft = calloc(size, sizeof(bool));
    if(!isLeft) return NULL;

    PVpair *temp = malloc(size * sizeof(PVpair));
    if(!temp) return  NULL;


    t0 = now();
    bst->root = buildOptBst2d(arraySortedX, arraySortedY, temp, NULL, 0, size-1, true, isLeft);
    t1 = now();
    printf("Build: %f s\n", t1 - t0);


    free(arraySortedX); 
    free(arraySortedY);
    free(temp);
    free(isLeft);

    bst->size = size;
    pd->bst = bst;
	return pd;
}


void pdctFree(PointDct *pd)
{
	bst2dFree(pd->bst,false,false);
	free(pd);
}


size_t pdctSize(PointDct *pd)
{
	return bst2dSize(pd->bst);
}


size_t pdctHeight(PointDct *pd)
{
	return bst2dHeight(pd->bst) -1;
}


size_t pdctAverageNodeDepth(PointDct *pd)
{
	return bst2dAverageNodeDepth(pd->bst);
}

/*
    Function used to answer queries with the dictionnary
*/

void *pdctExactSearch(PointDct *pd, Point *p)
{
    return pdctExactSearchRec(pd->bst->root,p,true);
}

void *pdctExactSearchRec(BNode2d *node, Point *p,bool axis)
{
    if(!node) return NULL;
    Point *point = node->point;
    if(ptCompare(point,p) == 0) return node->value;

    int comp;
    if(axis)
    {
        comp = compare(ptGetx(p),ptGetx(point));
        axis = false;
    }
    else
    {
        comp = compare(ptGety(p),ptGety(point));
        axis = true;
    }

    if(comp < 0) return pdctExactSearchRec(node->left, p, axis);
    else return pdctExactSearchRec(node->right, p, axis);
}


static inline bool isInBall(double x, double y, double xc, double yc, double rr)
{
    if((x-xc)*(x-xc) + (y-yc)*(y-yc) <= rr) return true;
    else return false;

}

// Fonction interne pour explorer larbre
void pdctBallSearchRec(BNode2d *node,double xc,double yc, double r,double rr, bool axis, List *result)
{
    if (node == NULL) return;

    double x = ptGetx(node->point);
    double y = ptGety(node->point);

    if(isInBall(x, y, xc, yc, rr))
        listInsertLast(result, node->value);

    double dist;
        
    if(axis)
        dist = xc - x;
    else
        dist = yc - y;

    if (dist < r) 
        pdctBallSearchRec(node->left, xc, yc, r,rr, !axis, result);
    
    if (dist >= -r) 
        pdctBallSearchRec(node->right, xc, yc, r,rr, !axis, result);
}

List *pdctBallSearch(PointDct *pd, Point *p, double r)
{
    List *result = listNew();
    if(!result)
        return NULL;

    if(pd->bst->root == NULL)
        return result;

    double xc = ptGetx(p);
	double yc = ptGety(p);

    pdctBallSearchRec(pd->bst->root, xc, yc, r,r*r, true, result);

    return result;
}




