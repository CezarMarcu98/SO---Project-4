#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "queueAndList.h"


extern int errno;


TLista AlocCelula(void *e)
{
	TLista aux = (TLista)malloc(sizeof(TCelula));

	if (aux) {
		aux->info = e;
		aux->urm = NULL;
	}
	return aux;
}

void DistrugeL(TLista *aL)
{
	TLista aux;

	while (*aL) {
		aux = *aL;
		*aL = (*aL)->urm;
		free(aux);
	}
}

void Insert(TLista *aL, void *data, size_t data_size)
{
	TLista aux = (TLista)calloc(1, sizeof(TCelula));

	aux->info = data;

	aux->urm = (*aL);
	(*aL) = aux;
}

void *firsList(TLista list)
{
	return list->info;
}

void popList(TLista *al)
{
	TLista temp = *al;
	(*al) = (*al)->urm;
	free(temp);
}

void *take_Thr(TLista *al)
{
	TLista aux = (*al);
	void *temp = aux->info;

	(*al) = (*al)->urm;
	aux->urm = NULL;
	free(aux);
	return temp;
}

void *headQ(priorityQueue *q)
{
	if (q->head)
		return q->head->data;
	else
		return NULL;
}

void insertQ(priorityQueue *q, void *data, int priority)
{
	qCell *cell = calloc(1, sizeof(qCell));

	cell->data  = data;
	cell->priority = priority;

	if (q->head == NULL)
		q->head = cell;
	else if (q->head->priority < cell->priority) {
		cell->next = q->head;
		q->head = cell;
	} else {
		qCell *aux = q->head;

		while (aux->next != NULL &&
		       aux->next->priority >= priority)
			aux = aux->next;
		cell->next = aux->next;
		aux->next = cell;
	}
	q->size++;
}

void popQ(priorityQueue *q)
{
	qCell *temp = q->head;

	q->head = q->head->next;
	free(temp);
	q->size--;
}
