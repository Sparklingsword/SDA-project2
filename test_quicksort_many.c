#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* =========================
   A REMPLACER PAR TON CODE
   ========================= */
void quicksort(void **arr, size_t low, size_t high, int (*cmp)(void*, void*));
size_t pivotRand(size_t p, size_t q);
void swap(void **arr, int i, int j);

/* =========================
   OUTILS
   ========================= */

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

size_t pivotRand(size_t p, size_t q) 
{ 
    return p + (size_t) (rand()%(q-p+1)); 
}

void swap(void **arr, int i, int j)
{
    void *tmp = arr[i];
    arr[i] = arr[j];
    arr[j] = tmp;
}

int compareInt(void *a, void *b)
{
    int x = *(int*)a;
    int y = *(int*)b;

    if (x < y) return -1;
    if (x > y) return 1;
    return 0;
}

int isSorted(void **arr, int n, int (*cmp)(void*, void*))
{
    for (int i = 1; i < n; i++)
    {
        if (cmp(arr[i], arr[i - 1]) < 0)
            return 0;
    }
    return 1;
}

void printIntArrayRaw(int *arr, int n)
{
    printf("[");
    for (int i = 0; i < n; i++)
    {
        printf("%d", arr[i]);
        if (i < n - 1) printf(", ");
    }
    printf("]");
}

void printPtrArray(void **arr, int n)
{
    printf("[");
    for (int i = 0; i < n; i++)
    {
        printf("%d", *(int*)arr[i]);
        if (i < n - 1) printf(", ");
    }
    printf("]");
}

void freePtrArray(void **arr, int n)
{
    for (int i = 0; i < n; i++)
        free(arr[i]);
    free(arr);
}

void **makePtrArrayFromIntArray(const int *src, int n)
{
    void **arr = malloc((size_t)n * sizeof(void*));
    if (!arr) return NULL;

    for (int i = 0; i < n; i++)
    {
        int *x = malloc(sizeof(int));
        if (!x)
        {
            for (int j = 0; j < i; j++)
                free(arr[j]);
            free(arr);
            return NULL;
        }
        *x = src[i];
        arr[i] = x;
    }

    return arr;
}

int *makeRandomIntArray(int n, int minVal, int maxVal)
{
    int *arr = malloc((size_t)n * sizeof(int));
    if (!arr) return NULL;

    for (int i = 0; i < n; i++)
        arr[i] = minVal + rand() % (maxVal - minVal + 1);

    return arr;
}

int runOneTestFromIntArray(const int *src, int n, int testNumber, const char *label)
{
    void **arr = makePtrArrayFromIntArray(src, n);
    if (!arr)
    {
        printf("Erreur allocation sur le test %d (%s)\n", testNumber, label);
        return 0;
    }

    if (n > 0)
        quicksort(arr, 0, (size_t)n - 1, compareInt);

    if (!isSorted(arr, n, compareInt))
    {
        printf("\nECHEC test %d (%s)\n", testNumber, label);
        printf("Tableau d'origine : ");
        printIntArrayRaw((int*)src, n);
        printf("\n");

        printf("Tableau obtenu    : ");
        printPtrArray(arr, n);
        printf("\n");

        freePtrArray(arr, n);
        return 0;
    }

    freePtrArray(arr, n);
    return 1;
}

/* =========================
   20 CAS ECRITS A LA MAIN
   ========================= */
int runManualTests(void)
{
    int ok = 1;
    int testNumber = 1;

    {
        int t[] = {};
        ok &= runOneTestFromIntArray(t, 0, testNumber++, "vide");
    }
    {
        int t[] = {42};
        ok &= runOneTestFromIntArray(t, 1, testNumber++, "un element");
    }
    {
        int t[] = {1, 2};
        ok &= runOneTestFromIntArray(t, 2, testNumber++, "deja trie 2");
    }
    {
        int t[] = {2, 1};
        ok &= runOneTestFromIntArray(t, 2, testNumber++, "inverse 2");
    }
    {
        int t[] = {5, 5};
        ok &= runOneTestFromIntArray(t, 2, testNumber++, "deux egaux");
    }
    {
        int t[] = {1, 2, 3, 4, 5};
        ok &= runOneTestFromIntArray(t, 5, testNumber++, "deja trie 5");
    }
    {
        int t[] = {5, 4, 3, 2, 1};
        ok &= runOneTestFromIntArray(t, 5, testNumber++, "inverse 5");
    }
    {
        int t[] = {7, 7, 7, 7, 7};
        ok &= runOneTestFromIntArray(t, 5, testNumber++, "tous egaux");
    }
    {
        int t[] = {3, 1, 2, 3, 2, 1, 3};
        ok &= runOneTestFromIntArray(t, 7, testNumber++, "beaucoup doublons");
    }
    {
        int t[] = {0, -1, 5, -3, 2, -8, 7};
        ok &= runOneTestFromIntArray(t, 7, testNumber++, "avec negatifs");
    }
    {
        int t[] = {10, 0, 10, 0, 10, 0, 10, 0};
        ok &= runOneTestFromIntArray(t, 8, testNumber++, "alternance doublons");
    }
    {
        int t[] = {9, 1, 8, 2, 7, 3, 6, 4, 5};
        ok &= runOneTestFromIntArray(t, 9, testNumber++, "zigzag");
    }
    {
        int t[] = {100, 99, 98, 1, 2, 3, 4};
        ok &= runOneTestFromIntArray(t, 7, testNumber++, "gros puis petits");
    }
    {
        int t[] = {1, 100, 2, 99, 3, 98, 4, 97};
        ok &= runOneTestFromIntArray(t, 8, testNumber++, "alternance extremes");
    }
    {
        int t[] = {4, 4, 2, 2, 3, 3, 1, 1};
        ok &= runOneTestFromIntArray(t, 8, testNumber++, "paires doublons");
    }
    {
        int t[] = {6, 5, 5, 5, 4, 4, 3, 2, 1};
        ok &= runOneTestFromIntArray(t, 9, testNumber++, "descendant avec doublons");
    }
    {
        int t[] = {-5, -10, -3, -4, -1, -7};
        ok &= runOneTestFromIntArray(t, 6, testNumber++, "negatifs seulement");
    }
    {
        int t[] = {214, -214, 0, 999, -999, 5, -5};
        ok &= runOneTestFromIntArray(t, 7, testNumber++, "signes melanges");
    }
    {
        int t[] = {8, 3, 8, 3, 8, 3, 8, 3, 8};
        ok &= runOneTestFromIntArray(t, 9, testNumber++, "deux valeurs");
    }
    {
        int t[] = {12, 11, 13, 5, 6, 7};
        ok &= runOneTestFromIntArray(t, 6, testNumber++, "classique");
    }

    return ok;
}

/* =========================
   980 CAS ALEATOIRES
   ========================= */
int runRandomTests(void)
{
    int baseNumber = 21;

    for (int t = 0; t < 980; t++)
    {
        int n = 10000 + rand() % 101; /* tailles entre 0 et 100 */

        int mode = rand() % 5;
        int minVal, maxVal;

        if (mode == 0)
        {
            minVal = 0;
            maxVal = 10;      /* beaucoup de doublons */
        }
        else if (mode == 1)
        {
            minVal = -20;
            maxVal = 20;      /* petits entiers signés */
        }
        else if (mode == 2)
        {
            minVal = -1000;
            maxVal = 1000;    /* plus large */
        }
        else if (mode == 3)
        {
            minVal = 0;
            maxVal = 2;       /* énorme quantité de doublons */
        }
        else
        {
            minVal = -5;
            maxVal = 5;       /* très concentré */
        }

        int *src = makeRandomIntArray(n, minVal, maxVal);
        if (!src)
        {
            printf("Erreur allocation random test %d\n", baseNumber + t);
            return 0;
        }

        int ok = runOneTestFromIntArray(src, n, baseNumber + t, "random");
        free(src);

        if (!ok)
            return 0;
    }

    return 1;
}

int main(void)
{
    srand((unsigned)time(NULL));

    int repeats = 10;
    clock_t start, end;

    printf("Lancement des 20 tests manuels...\n");
    start = clock();

    for (int r = 0; r < repeats; r++)
    {
        if (!runManualTests())
        {
            printf("\nEchec dans les tests manuels.\n");
            return 1;
        }
    }

    end = clock();

    printf("20/20 tests manuels reussis (%d repetitions).\n", repeats);
    printf("Temps tests manuels : %.6f s\n",
           (double)(end - start) / CLOCKS_PER_SEC);

    printf("\nLancement des 980 tests aleatoires...\n");
    start = clock();

    for (int r = 0; r < repeats; r++)
    {
        if (!runRandomTests())
        {
            printf("\nEchec dans les tests aleatoires.\n");
            return 1;
        }
    }

    end = clock();

    printf("980/980 tests aleatoires reussis (%d repetitions).\n", repeats);
    printf("Temps tests aleatoires : %.6f s\n",
           (double)(end - start) / CLOCKS_PER_SEC);

    printf("\nTous les tests ont reussi.\n");

    return 0;
}