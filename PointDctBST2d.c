#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

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
} PVpair;

struct PointDct_t
{
	BST2d *bst;
};


void mergeSort(PVpair *array, int (*compare)(Point *, Point *), size_t p, size_t q);
int compareX(Point *p1, Point *p2);
int compareY(Point *p1, Point *p2);


BST2d *bst2dNew(void);
BNode2d *bn2dNew(Point *point, void *value);
void bst2dFree(BST2d *bst, bool freePoint, bool freeValue);
void bst2dFreeRec(BNode2d *n, bool freePoint, bool freeValue);
size_t bst2dSize(BST2d *bst);
size_t bst2dHeight(BST2d *bst);
static size_t bst2dHeightRec(BNode2d *root);
double bst2dAverageNodeDepth(BST2d *bst);
size_t calcBst2dAverageNodeDepth(BNode2d *node, size_t *nbrNode, size_t Depth);

BNode2d *buildOptBst2d(PVpair *arraySortedX ,PVpair *arraySortedY, BNode2d *parent,size_t p, size_t q, bool axis, bool *isLeft);
void *pdctExactSearch(PointDct *pd, Point *p);
void *pdctExactSearchRec(BNode2d *node, Point *p,bool axis);
List *pdctBallSearch(PointDct *pd, Point *p, double r);
BNode2d *bst2dSearchPointmin(BNode2d *node, Point *Point,bool axis);
void pdctBallSearchRec(BNode2d *node, Point *center, double r, bool axis, List *result);
//static BNode2d *successor2d(BNode2d *n);
//static BNode2d *bn2dMin(BNode2d *n);
int isInBall(Point *current,Point *ref,double r);

//void rebuildArray(PVpair *array,PVpair med,size_t *leftIds,size_t leftSize,size_t p,size_t q);
//void getLeftIds(PVpair *array, size_t p, size_t m, size_t *ids);
//bool isInLeft(size_t id, size_t *ids, size_t size);

void markLeftIds(PVpair *array, bool *isLeft, size_t p, size_t m);
void unmarkLeftIds(PVpair *array, bool *isLeft, size_t p, size_t m);
void tempName(PVpair *array, PVpair med, bool *isLeft, size_t p, size_t q);



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
PointDct *pdctCreate(List *lpoints, List *Lvalues)
{
    if(listSize(lpoints) != listSize(Lvalues)) return NULL;

    PointDct *pd = malloc(sizeof(PointDct));
	if(!pd) return NULL;

	LNode *pointNode = lpoints->head;
	LNode *valNode = Lvalues->head;
    PVpair *array = malloc(listSize(Lvalues)*sizeof(PVpair));
	
	for(size_t i = 0; pointNode != NULL; i++)
	{
        array[i].point = pointNode->value;
        array[i].value = valNode->value;
        array[i].id = i;

        pointNode = pointNode->next;
		valNode = valNode->next;

    }

    PVpair *arraySortedX = malloc(listSize(Lvalues)* sizeof(PVpair));

    memcpy(arraySortedX, array, listSize(Lvalues)* sizeof(PVpair));

    PVpair *arraySortedY = malloc(listSize(Lvalues)* sizeof(PVpair));

    memcpy(arraySortedY, array, listSize(Lvalues)* sizeof(PVpair));

    free(array);

    mergeSort(arraySortedX, compareX, 0, listSize(Lvalues)-1);
    mergeSort(arraySortedY, compareY, 0, listSize(Lvalues)-1);

    BST2d *bst = bst2dNew();
    size_t n = listSize(Lvalues);
    bool *isLeft = calloc(n, sizeof(bool));

    bst->root = buildOptBst2d(arraySortedX, arraySortedY, NULL, 0, n-1, true, isLeft);

    free(isLeft);

    free(arraySortedX); // On a plus besoin de array donc je peux non ? // Je viens de tester avec valgrind, go le laisser, on gagne 160000 bytes
    free(arraySortedY);

    bst->size = listSize(Lvalues);
    pd->bst = bst;
	return pd;

}

BNode2d *buildOptBst2d(PVpair *arraySortedX ,PVpair *arraySortedY, BNode2d *parent,size_t p, size_t q, bool axis, bool *isLeft)
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
        tempName(arraySortedY, arraySortedX[m], isLeft, p, q);
        unmarkLeftIds(arraySortedX, isLeft, p, m);
    }
    else
    {
        node = bn2dNew(arraySortedY[m].point, arraySortedY[m].value);

        markLeftIds(arraySortedY, isLeft, p, m);
        tempName(arraySortedX, arraySortedY[m], isLeft, p, q);
        unmarkLeftIds(arraySortedY, isLeft, p, m);
    }

    if(!node)
    {
        return NULL;
    }

    node->parent = parent;
    if(p < m) node->left = buildOptBst2d(arraySortedX, arraySortedY, node, p, m-1, !axis, isLeft);
    
    node->right = buildOptBst2d(arraySortedX, arraySortedY, node, m+1, q, !axis, isLeft);

    return node;
}

void markLeftIds(PVpair *array, bool *isLeft, size_t p, size_t m)
{
    for(size_t i = p; i < m; i++)
    {
        isLeft[array[i].id] = true;
    }
}

void unmarkLeftIds(PVpair *array, bool *isLeft, size_t p, size_t m)
{
    for(size_t i = p; i < m; i++)
    {
        isLeft[array[i].id] = false;
    }
}

void tempName(PVpair *array, PVpair med, bool *isLeft, size_t p, size_t q) 
{
    PVpair *temp = malloc((q-p+1)*sizeof(PVpair));
    if(!temp) return;

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

    free(temp);
}

/*
PointDct *pdctCreate(List *lpoints, List *Lvalues)
{
    if(listSize(lpoints) != listSize(Lvalues)) return NULL;

    PointDct *pd = malloc(sizeof(PointDct));
	if(!pd) return NULL;

	LNode *pointNode = lpoints->head;
	LNode *valNode = Lvalues->head;
    PVpair *array = malloc(listSize(Lvalues)*sizeof(PVpair));
	
	for(size_t i = 0; pointNode != NULL; i++)
	{
        array[i].point = pointNode->value;
        array[i].value = valNode->value;
        array[i].id = i;

        pointNode = pointNode->next;
		valNode = valNode->next;

    }

    PVpair *arraySortedX = malloc(listSize(Lvalues)* sizeof(PVpair));

    memcpy(arraySortedX, array, listSize(Lvalues)* sizeof(PVpair));

    PVpair *arraySortedY = malloc(listSize(Lvalues)* sizeof(PVpair));

    memcpy(arraySortedY, array, listSize(Lvalues)* sizeof(PVpair));

    free(array);

    mergeSort(arraySortedX, compareX, 0, listSize(Lvalues)-1);
    mergeSort(arraySortedY, compareY, 0, listSize(Lvalues)-1);

    BST2d *bst = bst2dNew();
    bst->root = buildOptBst2d(arraySortedX,arraySortedY,NULL, 0, listSize(Lvalues)-1,true);

    free(arraySortedX); // On a plus besoin de array donc je peux non ? // Je viens de tester avec valgrind, go le laisser, on gagne 160000 bytes
    free(arraySortedY);

    bst->size = listSize(Lvalues);
    pd->bst = bst;
	return pd;

}

BNode2d *buildOptBst2d(PVpair *arraySortedX ,PVpair *arraySortedY,BNode2d *parent, size_t p, size_t q,bool axis)
{
    if(p>q)
    {
        return NULL;
    }

    size_t m = p + (q - p)/2;
    BNode2d *node;

    if(axis)
    {
        node = bn2dNew(arraySortedX[m].point,arraySortedX[m].value);
        size_t *leftIds = malloc((m-p) * sizeof(size_t));
        getLeftIds(arraySortedX, p, m, leftIds);

        rebuildArray(arraySortedY, arraySortedX[m], leftIds, m-p, p, q);

        free(leftIds);
    } 

    else 
    {
        node = bn2dNew(arraySortedY[m].point,arraySortedY[m].value);
        size_t *leftIds = malloc((m-p) * sizeof(size_t));
        getLeftIds(arraySortedY, p, m, leftIds);

        rebuildArray(arraySortedX, arraySortedY[m], leftIds, m-p, p, q);

        free(leftIds);
    }


    if(!node)
    {
        return NULL;
    }

    node->parent = parent;
    

    if(p < m)
    {
        node->left = buildOptBst2d(arraySortedX,arraySortedY,node,p,m-1,!axis);
    }

    node->right = buildOptBst2d(arraySortedX,arraySortedY,node,m+1,q,!axis);

    return node;
}

void getLeftIds(PVpair *array, size_t p, size_t m, size_t *ids)
{
    size_t k = 0;
    for(size_t i = p; i < m; i++)
    {
        ids[k++] = array[i].id;
    }
}

bool isInLeft(size_t id, size_t *ids, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        if(ids[i] == id) return true;
    }
    return false;
}

void rebuildArray(PVpair *array,PVpair med,size_t *leftIds,size_t leftSize,size_t p,size_t q)
{
    size_t n = q - p + 1;
    PVpair *temp = malloc(n * sizeof(PVpair));
    if(!temp) return;

    size_t i = 0;
    size_t j = leftSize + 1; // après pivot

    for(size_t k = p; k <= q; k++)
    {
        if(array[k].id == med.id)
        {
            temp[leftSize] = array[k]; // pivot au milieu
        }
        else if(isInLeft(array[k].id, leftIds, leftSize))
        {
            temp[i++] = array[k]; // gauche
        }
        else
        {
            temp[j++] = array[k]; // droite
        }
    }

    // copie dans array
    for(size_t k = 0; k < n; k++)
    {
        array[p + k] = temp[k];
    }

    free(temp);
}

*/

/*
PointDct *pdctCreate(List *lpoints, List *Lvalues)
{
    if(listSize(lpoints) != listSize(Lvalues)) return NULL;

    PointDct *pd = malloc(sizeof(PointDct));
	if(!pd) return NULL;

	LNode *pointNode = lpoints->head;
	LNode *valNode = Lvalues->head;
    PVpair *array = malloc(listSize(Lvalues)*sizeof(PVpair));
	
	for(int i = 0;pointNode;i++)
	{
        array[i].point = pointNode->value;
        array[i].value = valNode->value;

        pointNode = pointNode->next;
		valNode = valNode->next;

    }

    BST2d *bst = bst2dNew(ptCompare);

    bst->root = buildOptBst2d(array, NULL, 0, listSize(lpoints) - 1, true);
    free(array); // On a plus besoin de array donc je peux non ? // Je viens de tester avec valgrind, go le laisser, on gagne 160000 bytes
    bst->size = listSize(Lvalues);

    pd->bst = bst;
	return pd;
}

BNode2d *buildOptBst2d(PVpair *array ,BNode2d *parent, size_t p, size_t q,bool axis) 
{
    if(p>q)
    {
        return NULL;
    }

    if(axis)
    {
        mergeSort(array, compareX, p, q);
        axis = false;
    } 

    else 
    {
        mergeSort(array, compareY, p, q);
        axis = true;
    }

    size_t m = p + (q - p)/2;

    BNode2d *node = bn2dNew(array[m].point,array[m].value);

    if(!node)
    {
        return NULL;
    }

    node->parent = parent;
    

    if(p < m)
    {
        node->left = buildOptBst2d(array,node,p,m-1,axis);
    }

    node->right = buildOptBst2d(array,node,m+1,q,axis);

    return node;
}
*/
void mergeSort(PVpair *array, int (*compare)(Point *, Point *),size_t p, size_t q)
{
   

    if(p < q)
        {
            PVpair *temp = malloc((q+1)*sizeof(PVpair));
            size_t m = p + (q - p)/2;
            mergeSort(array, compare, p, m);
            mergeSort(array, compare, m + 1 , q);

            int comp;
            size_t i = p;
            size_t j = m + 1; 

            for(size_t k = p; k <= q; k++)
            {
                if(i == m + 1)
                {
                    temp[k] = array[j];
                    j++;
                    continue;
                }

                if(j == q+1)
                {
                    temp[k] = array[i];
                    i++;
                    continue;
                }

                comp = compare(array[i].point,array[j].point);

                if(comp <= 0) 
                {
                    temp[k] = array[i];
                    i++;
                }

                if(comp > 0)
                {
                    temp[k] = array[j];
                    j++;
                }
            }

            for (size_t k = p; k <= q; k++)
            {
                array[k] = temp[k];
            }
            free(temp);
        }

    return;
}

int compareX(Point *p1, Point *p2)
{
    if(ptGetx(p1)< ptGetx(p2)) return -1;
    else return 1;
}

int compareY(Point *p1, Point *p2)
{
    if(ptGety(p1)< ptGety(p2)) return -1;
    else return 1;
}
void pdctFree(PointDct *pd)
{
	bst2dFree(pd->bst,false,false);
	free(pd);
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

size_t pdctSize(PointDct *pd)
{
	return bst2dSize(pd->bst);
}

size_t bst2dSize(BST2d *bst)
{
    return bst->size;
}


size_t pdctHeight(PointDct *pd)
{
	return bst2dHeight(pd->bst);
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

size_t pdctAverageNodeDepth(PointDct *pd)
{
	return bst2dAverageNodeDepth(pd->bst);
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

void *pdctExactSearch(PointDct *pd, Point *p)
{
    return pdctExactSearchRec(pd->bst->root,p,true);
}
void *pdctExactSearchRec(BNode2d *node, Point *p,bool axis)
{
    if(!node) return NULL;
    if(ptCompare(node->point,p) == 0) return node->value;

    int comp;
    if(axis)
    {
        comp = compareX(p,node->point);
        axis = false;
    }
    else
    {
        comp = compareY(p,node->point);
        axis = true;
    }

    if(comp <= 0) return pdctExactSearchRec(node->left, p, axis);
    else return pdctExactSearchRec(node->right, p, axis);
}
/*

Pas bon

List *pdctBallSearch(PointDct *pd, Point *p, double r)
{
    Point *x1 = ptNewFromXY(ptGetx(p) - r,ptGety(p) - r);
    if(x1 == NULL)
            fprintf(stderr, "x1 pointe vers NULL\n");
    Point *x2 = ptNewFromXY(ptGetx(p) + r,ptGety(p) + r);
    if(x2 == NULL)
            fprintf(stderr, "x2 pointe vers NULL\n");

    BNode2d *current = bst2dSearchPointmin(pd->bst->root, x1, true);
    if(current == NULL)
        fprintf(stderr, "current pointe vers NULL\n");

    List *result = listNew();

    while(ptCompare(x2,current->point)>0)
    {
        if(isInBall(current->point,p,r))
        {
            listInsertLast(result,current->value);
        }
        current = successor2d(current);
        if(current == NULL)
            fprintf(stderr, "current après while dans pointe vers NULL\n");
    }
    return result;
}
*/

// Fonction interne pour explorer larbre
void pdctBallSearchRec(BNode2d *node, Point *ref, double r, bool axis, List *result)
{
    if (node == NULL)
        return;

    if (isInBall(node->point, ref, r))
        listInsertLast(result, node->value);

    double dist;

    if(axis)
        dist = ptGetx(ref) - ptGetx(node->point);
    else
        dist = ptGety(ref) - ptGety(node->point);

    if (dist >= -r) 
        pdctBallSearchRec(node->left, ref, r, !axis, result);
    
    if (dist <= r) 
        pdctBallSearchRec(node->right, ref, r, !axis, result);
}

List *pdctBallSearch(PointDct *pd, Point *p, double r)
{
    List *result = listNew();
    if(!result)
        return NULL;

    if(pd->bst->root == NULL)
        return NULL;

    pdctBallSearchRec(pd->bst->root, p, r, true, result);

    return result;
}

BNode2d *bst2dSearchPointmin(BNode2d *node, Point *p, bool axis)
{
    int comp;
    if(!node)
    {
        fprintf(stderr, "node pointe vers NULL\n");
        return NULL;
    }

    if(axis)
    {
        comp = compareX(p,node->point);
        axis = false;
    }
    else
    {
        comp = compareY(p,node->point);
        axis = true;
    }

    if(comp < 0)    
    {
        return bst2dSearchPointmin(node->right,p,axis);
    }

    else
    {
        BNode2d *test = bst2dSearchPointmin(node->left,p,axis);
        if(test)
        {
            return test;
        }
        else fprintf(stderr, "test pointe vers NULL\n");
        return node;
    }
}

/* sert à rien du coup
static BNode2d *successor2d(BNode2d *n)
{

    if(n == NULL)
        fprintf(stderr, "n pointe ver NULL\n");

    if(n->left == NULL)
        fprintf(stderr, "n->left pointe ver NULL\n");

    if(n->right == NULL)
        fprintf(stderr, "n->right pointe ver NULL\n");

    if (n->right != NULL)
        return bn2dMin(n->right);
    BNode2d *y = n->parent;
    if(y == NULL)
        fprintf(stderr, "y pointe ver NULL\n");
    BNode2d *x = n;
    if(x == NULL)
        fprintf(stderr, "x pointe ver NULL\n");
    while (y != NULL && x == y->right)
    {
        x = y;
        if(x == NULL)
            fprintf(stderr, "x dans while pointe ver NULL\n");
        y = y->parent;
        if(x == NULL)
            fprintf(stderr, "y dans while pointe ver NULL\n");
    }
    return y;
}

static BNode2d *bn2dMin(BNode2d *n)
{
    while (n->left != NULL)
        n = n->left;
    return n;
}*/

int isInBall(Point *current,Point *ref,double r)
{
	double x = ptGetx(current);
	double y = ptGety(current);

	double xc = ptGetx(ref);
	double yc = ptGety(ref);

	if( (x-xc)*(x-xc) + (y-yc)*(y-yc) <= r*r ) return 1;

	return 0;
}