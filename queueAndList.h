#include <stdio.h>

#ifndef QUEUE_H__
#define QUEUE_H__

typedef struct celula {
	void *info;
	struct celula *urm;
} TCelula, *TLista;

typedef struct Node {
	int priority;
	void *data;
	struct Node *next;
} qCell;

typedef struct Queue {
	size_t size;
	qCell *head;
} priorityQueue;

TLista AlocCelula(void *e);
void DistrugeL(TLista *aL);
void Insert(TLista *aL, void *data, size_t data_size);
void *take_Thr(TLista *al);
void *firsList(TLista list);
void popList(TLista *al);

void *headQ(priorityQueue *q);
void insertQ(priorityQueue *q, void *data, int priority);
void popQ(priorityQueue *q);

#endif // QUEUE_H__
