#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void bubble_sort(int *list, int n) {
  for (int c = 0 ; c < (n - 1); c++) {
    for (int d = 0 ; d < n - c - 1; d++) {
      if (list[d] > list[d+1]) {
        int t = list[d];
        list[d] = list[d+1];
        list[d+1] = t;
      }
    }
  }
}

void selection_sort(int *list, int n) {
  for (int i = 0; i < (n-1); i++) {
    int min = i;
    for (int j = (i+1); j < n; j++) {
      if(list[j] < list[min]) 
        min = j;
    }
    if (i != min) {
      int aux = list[i];
      list[i] = list[min];
      list[min] = aux;
    }
  }
}

void insertion_sort(int *list, int n) {
  for (int i = 0; i < n; i++) {
    int current = list[i];
    int j = i - 1;
    while ((j >= 0) && (current < list[j])) {
      list[j + 1] = list[j];
      j = j - 1;
    }
    
    list[j + 1] = current;
  }
}

typedef struct Node {
  struct Node *next;
  int value;
} Node;

typedef struct List {
  Node *head;
} List;

List* sub_list(List *l, int elements) {
  if (!l)
    return NULL;

  List *newList = (List*) malloc(sizeof(List));
  Node *prev = NULL;
  Node *n = l->head;
  for (int i=0; i < elements; i++) {
    if (!n) break;
    Node *newNode = (Node*) malloc(sizeof(Node));
    newNode->value = n->value;
    if (prev)
      prev->next = newNode;
    else
      newList->head = newNode;
    prev = newNode;
    n = n->next;
  }
  return newList;
}

struct Pair {
    int i;
    int j;
};

struct Pair closest_point(struct Pair *pairs, int size) {
  int min = INFINITY;
  struct Pair closest;
  for (int i=0; i < size; i++) {
    struct Pair p = pairs[i];
    int dist = sqrt(pow(p.i, 2) + pow(p.j, 2));
    if (dist < min) {
      min = dist;
      closest = p;
     }
  }
  return closest;
}

int closest_pairs(struct Pair *pairs, int size) {
  int min = INFINITY;
  struct Pair closest;
  for (int i=0; i < size; i++) {
    struct Pair p1 = pairs[i];
    for (int j=i; j < size; j++) {
      struct Pair p2 = pairs[j];
      int dist = sqrt(pow(p1.i - p2.i, 2) + pow(p1.j - p2.j, 2));
      if (dist < min) {
        min = dist;
      }
    }
  }
  return min;
}

int multiply(int *a, int *b, int size) {
  int result = 0;
  for (int i=0; i < size; i++) {
      result += a[i] * b[i];
  }
  return result;
}

/* Initialize a single-source shortest-paths computation. */
void initialize_single_source(double d[], int n) {
  for (int i = 1; i < n; ++i) {
    d[i] = 1000000000.0;
  }
  d[0] = 0.0;
}

double *bellman_ford(double **w, int n) {
  double *d = (double*) malloc(sizeof(double)*n);
  initialize_single_source(d, n);

  for (int i = 0; i < n-1; ++i) {
    for (int u = 0; u < n; ++u) {
      for (int v = 0; v < n; ++v) {
      	if (d[v] > d[u] + w[u][v]) {
          d[v] = d[u] + w[u][v];
        }
      }
    }
  }

  for (int u = 0; u < n; ++u) {
    for (int v = 0; v < n; ++v) {
      if (d[v] > d[u] + w[u][v])
        return NULL;
    }    
  }

  return d;
}

int **matrix_mul(int **matA, int **matB, int n){
  int i, j, k, sum;
  int **result = (int**) malloc(n * sizeof(int*));
  for (i = 0; i < n; i++)
    result[i] = (int*) malloc(n * sizeof(int));

  for (i=0; i < n; i++) {
    for (j=0; j < n; j++) {
      sum = 0;
      for (k=0; k < n; k++) {
        sum += matA[i][k] * matB[k][j];
      }
      result[i][j] = sum;
    }
  }
  return result;
}

int **sum(int **matA, int **matB, int n) {
  if (!matA || !matB) return NULL;
  int **result = (int**) malloc(n * sizeof(int*));
  for (int i=0; i < n; i++) {
    result[i] = (int*) malloc(n * sizeof(int));
    for(int j=0; j < n; j++)
      result[i][j] = matA[i][j] + matB[i][j];
  }
  return result;
}
