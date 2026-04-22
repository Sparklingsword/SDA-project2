#include <stdio.h>
#include <stdlib.h>
#include <time.h>


void swap(void **arr, int i, int j);

/* -------- TON QUICKSORT -------- */
void quicksort(void **arr, size_t low, size_t high, int (*cmp)(void*, void*));

/* -------- COMPARATEUR -------- */
int compareInt(void *a, void *b)
{
    int x = *(int*)a;
    int y = *(int*)b;

    if (x < y) return -1;
    if (x > y) return 1;
    return 0;
}

/* -------- PRINT -------- */
void printArray(void **arr, int n)
{
    for (int i = 0; i < n; i++)
    {
        printf("%d ", *(int*)arr[i]);
    }
    printf("\n");
}

/* -------- CHECK -------- */
int isSorted(void **arr, int n, int (*cmp)(void*, void*))
{
    for (int i = 1; i < n; i++)
    {
        if (cmp(arr[i], arr[i-1]) < 0)
            return 0;
    }
    return 1;
}

/* -------- MAIN -------- */
int main(int argc, char *argv[])
{
    int n = 20;

    if (argc > 1)
        n = atoi(argv[1]);

    void **arr = malloc(n * sizeof(void*));
    if (!arr) return 1;

    srand(time(NULL));

    /* allocation des ints */
    for (int i = 0; i < n; i++)
    {
        int *x = malloc(sizeof(int));
        *x = rand() % 100;
        arr[i] = x;
    }

    printf("Avant tri:\n");
    printArray(arr, n);

    clock_t start = clock();

    quicksort(arr, 0, n - 1, compareInt);

    clock_t end = clock();

    printf("Après tri:\n");
    printArray(arr, n);

    printf("Tri correct ? %s\n",
           isSorted(arr, n, compareInt) ? "OUI" : "NON");

    printf("Temps: %f s\n",
           (double)(end - start) / CLOCKS_PER_SEC);

    /* free mémoire */
    for (int i = 0; i < n; i++)
        free(arr[i]);
    free(arr);

    return 0;
}

size_t pivotRand(size_t p, size_t q)
{
	return p + (size_t) (rand()%(q-p+1));
}

void quicksort(void **arr, size_t low, size_t high, int (*cmp)(void*, void*))
{

    if(low >= high)return;
    size_t pivot = pivotRand(low,high);
    swap(arr,pivot,high);

    int i = low;
    int j = high-1;
    int k = low;
    while(k <= j)
    {
        int comp = cmp(arr[k],arr[high]);
        if(comp < 0)
        {
            swap(arr,k,i);
            i++;
            k++;
        }

        else if (comp > 0)
        {
            swap(arr,k,j);
            j--;
        }
        else
        {
            k++;
        }
    }

    swap(arr,j+1,high);
    if(i-1>=0)
    {
        quicksort(arr,low, i-1,  cmp);
    }

    if(j+2 <= (int) high)
    {
      quicksort(arr,j+2,  high,cmp);  
    } 

}

void swap(void **arr, int i, int j)
{
    void *tmp = arr[i];
    arr[i] = arr[j];
    arr[j] = tmp;
}