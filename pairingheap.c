/*
    libtds: tiny data structures
    Copyright (C) 2017 junhee lee

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <string.h>

#include "pairingheap.h"

static struct pairingheap_node* pairingheap_node_create(void* value);
static void pairingheap_node_destroy(struct pairingheap_node* base, bool recursively);
static struct pairingheap_node* pairingheap_node_merge(struct pairingheap* self, struct pairingheap_node* lhs, struct pairingheap_node* rhs);

struct pairingheap* pairingheap_create(pairingheap_compare compare) {
	struct pairingheap* self = (struct pairingheap*)calloc(1, sizeof(struct pairingheap));
	if(!self)
		return NULL;

	self->compare = compare;
	return self;
}

void pairingheap_destroy(struct pairingheap* self) {
	pairingheap_node_destroy(self->root, true);
	memset(self, 0, sizeof(struct pairingheap)), free(self);
}

size_t pairingheap_size(struct pairingheap* self) {
	return self->size;
}

int pairingheap_push(struct pairingheap* self, void* value) {
	struct pairingheap_node* node = pairingheap_node_create(value);
	if(!node)
		return 1;

	self->root = pairingheap_node_merge(self, self->root, node);
	self->size += 1;
	return 0;
}

void* pairingheap_pop(struct pairingheap* self) {
	if(!self->size)
		return NULL;

	struct pairingheap_node* root = self->root;
	void* value = self->root->value;

	if(self->size == 1) {
		pairingheap_node_destroy(root, true);
		self->root = NULL;
		self->size = 0;
		return value;
	}

	struct pairingheap_node* pass1_default[1024];

	// first pass: left to right merging
	struct pairingheap_node** pass1 = pass1_default;
	size_t pass1_capacity = sizeof(pass1_default) / sizeof(pass1_default[0]);
	size_t pass1_size = 0;

	struct pairingheap_node* link = root->down;
	while(link) {
		struct pairingheap_node* left = link;
		link = link->right;

		struct pairingheap_node* right = link;
		if(!right) {
			struct pairingheap_node** lastnode;
			if(pass1_size)
				lastnode = &pass1[pass1_size - 1];
			else {
				lastnode = &pass1[0];
				*lastnode = NULL;
				pass1_size += 1;
			}

			*lastnode = pairingheap_node_merge(self, *lastnode, left);
			break;
		}
		link = link->right;

		struct pairingheap_node* merged = pairingheap_node_merge(self, left, right);
		pass1[pass1_size++] = merged;

		if(pass1_size == pass1_capacity) {
			struct pairingheap_node** tmp = pass1;

			pass1 = (struct pairingheap_node**)malloc(sizeof(struct pairingheap_node*) * (pass1_capacity * 2));
			if(!pass1) {
				pass1 = tmp;
				struct pairingheap_node** lastnode = &pass1[pass1_size - 1];
				while(link) {
					*lastnode = pairingheap_node_merge(self, *lastnode, link);
					link = link->right;
				}
				break;
			}
			memcpy(pass1, tmp, sizeof(struct pairingheap_node*) * pass1_size);
			pass1_capacity *= 2;

			if(tmp != pass1_default)
				free(tmp);
		}
	}

	pairingheap_node_destroy(self->root, false), self->root = NULL;

	// second pass: right to left merging
	root = pass1[--pass1_size];
	while(pass1_size)
		root = pairingheap_node_merge(self, root, pass1[--pass1_size]);

	self->root = root;
	self->size -= 1;

	if(pass1 != pass1_default)
		free(pass1);

	return value;
}

void* pairingheap_peek(struct pairingheap* self) {
	return self->root->value;
}

static struct pairingheap_node* pairingheap_node_create(void* value) {
	struct pairingheap_node* node = (struct pairingheap_node*)calloc(1, sizeof(struct pairingheap_node));
	if(!node)
		return NULL;

	node->value = value;
	return node;
}

static void pairingheap_node_destroy(struct pairingheap_node* base, bool recursively) {
	if(recursively) {
		struct pairingheap_node* link = base->down;
		while(link) {
			pairingheap_node_destroy(link, true);
			link = link->right;
		}
	}
	free(base);
}

static struct pairingheap_node* pairingheap_node_merge(struct pairingheap* self, struct pairingheap_node* lhs, struct pairingheap_node* rhs) {
	if(!lhs)
		return rhs;
	if(!rhs)
		return lhs;

	struct pairingheap_node* parent;
	struct pairingheap_node* child;

	if(self->compare(lhs->value, rhs->value) < 0)
		parent = lhs, child = rhs;
	else
		parent = rhs, child = lhs;

	child->right = parent->down;
	parent->down = child;

	return parent;
}

#ifndef NDEBUG
#include <stdio.h>
static void pairingheap_node_dump(struct pairingheap_node* node, int depth) {
	if(!node)
		return;

	for(int i = 0; i < depth; ++i)
		putchar(' ');

	size_t nchild = 0;
	if(node->down) {
		struct pairingheap_node* link = node->down;
		while(link) {
			nchild += 1;
			link = link->right;
		}
	}

	printf("value[%ld] nchild[%ld]\n", (intptr_t)node->value, nchild);
	struct pairingheap_node* link = node->down;
	while(link) {
		pairingheap_node_dump(link, depth + 2);
		link = link->right;
	}
}

static int int_compare(void* a_lhs, void* a_rhs) {
	return (intptr_t)a_lhs - (intptr_t)a_rhs;
}

#include <assert.h>
#include <locale.h>
#include <time.h>
static long timediff(struct timespec* start, struct timespec* end) {
	int64_t ndiff = end->tv_nsec - start->tv_nsec;
	int64_t sdiff = (end->tv_sec - start->tv_sec) * 1000 * 1000 * 1000;
	return (sdiff + ndiff) / 1000; // diff in microsec
}

int main(int argc, char** argv) {
	setlocale(LC_NUMERIC, "");

	puts("starting pairingheap test suites...");
	puts("===================================");
	struct pairingheap* queue = pairingheap_create(int_compare);

	intptr_t input_values[] = {1,3,9,4,10,11,2};
	int input_values_size = sizeof(input_values) / sizeof(input_values[0]);
	for(int i = 0; i < input_values_size; ++i)
		pairingheap_push(queue, (void*)input_values[i]);
	pairingheap_node_dump(queue->root, 0);

	puts("start poping...");
	for(int i = 0; i < input_values_size; ++i) {
		intptr_t value = (intptr_t)pairingheap_pop(queue);
		printf("\x1B[32mpoped value: %ld \x1B[0m   \n", value);
		pairingheap_node_dump(queue->root, 0);
	}

	srand(time(NULL));
	struct timespec insert_start, insert_end;
	int item_count = 10000000;

	while(1) {

	printf("pushing %'d random integer numbers...", item_count);
	clock_gettime(CLOCK_MONOTONIC, &insert_start);
	for(int i = 0; i < item_count; ++i) {
		/* intptr_t value = rand(); */
		intptr_t value = i;
		/* intptr_t value = item_count - i; */
		pairingheap_push(queue, (void*)value);
	}
	clock_gettime(CLOCK_MONOTONIC, &insert_end);
	printf("it took %'ldusec\n", timediff(&insert_start, &insert_end));
	/* pairingheap_node_dump(queue->root, 0); */

	printf("poping %'d random integer numbers...", item_count);
	clock_gettime(CLOCK_MONOTONIC, &insert_start);
	while(pairingheap_size(queue)) {
		pairingheap_pop(queue);
	}
	clock_gettime(CLOCK_MONOTONIC, &insert_end);
	printf("it took %'ldusec\n", timediff(&insert_start, &insert_end));

	assert(queue->size == 0);

	}

	return 0;
}
#endif
