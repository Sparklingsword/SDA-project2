
#include <stdlib.h>
#include <stdint.h>
#include "BST.h"
#include "PointDct.h"

static uint64_t interleave8(uint8_t m, uint8_t n);

/* ------------------------------------------------------------------------- *
 * Returns the morton code (or z-code) associated to the (x,y) coordinates
 * in argument. The x and y values are expected to be uint32_t integer between
 * 0 and UINT32_MAX.
 * 
 * PARAMETERS
 * x           the x coordinate
 * y           the y coordinate
 *
 * RETURN
 * z           the morton code
 *
 * ------------------------------------------------------------------------- */

static uint64_t zEncode(uint32_t x, uint32_t y) {
	uint64_t out = 0;
	for (uint8_t byte = 0; byte < 4; ++byte) {
		out |= interleave8((x >> (byte * 8)) & 0xFF, (y >> (byte * 8)) & 0xFF) << (byte * 16);
	}
	return out;
}

static uint64_t interleave8(uint8_t m, uint8_t n) {
	return (
		((m * 0x0101010101010101ULL & 0x8040201008040201ULL) * 0x0102040810204081ULL >> 49) & 0x5555
	) | (
		((n * 0x0101010101010101ULL & 0x8040201008040201ULL) * 0x0102040810204081ULL >> 48) & 0xAAAA
	);
}

typedef struct minmax_t
{
    double minX, maxX;
    double minY, maxY;
} MinMax;

struct PointDct_t
{
	BST *bst;
	MinMax minmax;
};

typedef struct pvpair_t
{
    Point *point;
    void *value;
} PVpair;

int comparisonZ(void* a, void* b);
MinMax getMinMax(List *lpoints);
uint32_t normalizeUint32(double x, double min, double max);
int isInBall(Point *current, Point *ref, double r);

int comparisonZ(void* a, void* b) 
{
	uint64_t *z1 = (uint64_t*)a;
	uint64_t *z2 = (uint64_t*)b;

	if(*z1 < *z2)
		return -1;

	else if(*z1 == *z2)
		return 0;

	else return 1;
}

MinMax getMinMax(List *lpoints)
{
	Point *first = (Point *) lpoints->head->value;
	double minX = ptGetx(first);
	double minY = ptGety(first);
	double maxX = ptGetx(first);
	double maxY = ptGety(first);
	for(LNode *n = lpoints->head; n; n = n->next)
	{
		Point *currentPoint = (Point *) n->value;

		if(ptGetx(currentPoint) > maxX) maxX = ptGetx(currentPoint);
		if(ptGety(currentPoint) > maxY) maxY = ptGety(currentPoint);

		if(ptGetx(currentPoint) < minX) minX = ptGetx(currentPoint);
		if(ptGety(currentPoint) < minY) minY = ptGety(currentPoint);
	}

	MinMax minmax;

	minmax.minX = minX;
	minmax.maxX = maxX;

	minmax.minY = minY;
	minmax.maxY = maxY;

	return minmax;
}

uint32_t normalizeUint32(double x, double min, double max) 
{
	if(max == min)
		return 0;

	return (x-min)/(max-min)*UINT32_MAX;
}

PointDct *pdctCreate(List *lpoints, List *Lvalues)
{
	PointDct *pd = malloc(sizeof(PointDct));
	if(!pd)
		return NULL;

	MinMax minmax = getMinMax(lpoints);

	List *lz = listNew();
	List *pairList = listNew();

	if(listSize(lpoints) != listSize(Lvalues))
		return NULL;

	LNode *pointNode = lpoints->head;
	LNode *valNode = Lvalues->head;
	
	while(pointNode)
	{
		PVpair *pair = malloc(sizeof(PVpair));
		
		uint64_t *z = malloc(sizeof(uint64_t));

		pair->point = (Point *)pointNode->value;
		pair->value = valNode->value;

		uint32_t normalizedX = normalizeUint32(ptGetx(pair->point), minmax.minX, minmax.maxX);
		uint32_t normalizedY = normalizeUint32(ptGety(pair->point), minmax.minY, minmax.maxY);

		*z = zEncode(normalizedX, normalizedY);

		listInsertLast(lz, z);
		listInsertLast(pairList, pair);
		
		pointNode = pointNode->next;
		valNode = valNode->next;
	}

	BST *bst = bstOptimalBuild(comparisonZ,lz,pairList);

	listFree(lz, false);
	listFree(pairList, false);

	pd->bst = bst;

	pd->minmax = minmax;

	return pd;
}

void pdctFree(PointDct *pd)
{
	bstFree(pd->bst, true, true);
	free(pd);
}

size_t pdctSize(PointDct *pd)
{
	return bstSize(pd->bst);
}

size_t pdctAverageNodeDepth(PointDct *pd)
{
	return bstAverageNodeDepth(pd->bst);
}

size_t pdctHeight(PointDct *pd)
{
	return bstHeight(pd->bst);
}

void *pdctExactSearch(PointDct *pd, Point *p)
{
	MinMax minmax = pd->minmax;

	uint32_t normalizedX = normalizeUint32(ptGetx(p), minmax.minX, minmax.maxX);
	uint32_t normalizedY = normalizeUint32(ptGety(p), minmax.minY, minmax.maxY);

	uint64_t z = zEncode(normalizedX, normalizedY);
	List *l = bstRangeSearch(pd->bst, &z, &z);

	for(LNode *n = l->head; n; n = n->next)
	{
		PVpair *pair = (PVpair*) n->value;

		if(ptCompare(pair->point, p) == 0) 
		{
			listFree(l, false);
			return pair->value;
		}
	}

	listFree(l, false);
	return NULL;
}

List *pdctBallSearch(PointDct *pd, Point *p, double r)
{
	MinMax minmax = pd->minmax;

	double x1 = ptGetx(p) - r;
	double y1 = ptGety(p) - r;

	if(x1 < minmax.minX)
		x1 = minmax.minX;

	if(y1 < minmax.minY)
		y1 = minmax.minY;

	uint32_t normalizedX1 = normalizeUint32(x1, minmax.minX, minmax.maxX);
	uint32_t normalizedY1 = normalizeUint32(y1, minmax.minY, minmax.maxY);

	uint64_t z1 = zEncode(normalizedX1,normalizedY1);

	double x2 = ptGetx(p) + r;
	double y2 = ptGety(p) + r;

	if(x2 > minmax.maxX)
		x2 = minmax.maxX;

	if(y2 > minmax.maxY)
		y2 = minmax.maxY;

	uint32_t normalizedX2 = normalizeUint32(x2, minmax.minX, minmax.maxX);
	uint32_t normalizedY2 = normalizeUint32(y2, minmax.minY, minmax.maxY);

	uint64_t z2 = zEncode(normalizedX2, normalizedY2);

	List *l = bstRangeSearch(pd->bst, &z1, &z2);

	List *result = listNew();

	for(LNode *n = l->head; n; n = n->next)
	{
		PVpair *pair = (PVpair*) n->value;

		if(isInBall(pair->point, p, r) ==  1)
			listInsertLast(result, pair->value);
	}

	listFree(l, false);

	return result;
}

int isInBall(Point *current,Point *ref,double r)
{
	double x = ptGetx(current);
	double y = ptGety(current);

	double xc = ptGetx(ref);
	double yc = ptGety(ref);

	if((x-xc)*(x-xc) + (y-yc)*(y-yc) <= r*r)
		return 1;

	return 0;
}