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

#include "aatree.h"

static struct aatree_node* aatree_node_create(int level, void* value);
static void aatree_node_destroy(struct aatree_node* node, bool recursively);
static struct aatree_node* aatree_node_insert(struct aatree* self, struct aatree_node* node, void* value);
static struct aatree_node* aatree_node_remove(struct aatree* self, struct aatree_node* node, void* value);
static struct aatree_node* aatree_node_skew(struct aatree_node* node);
static struct aatree_node* aatree_node_split(struct aatree_node* node);
static struct aatree_node* aatree_node_decrease_level(struct aatree_node* node);
static struct aatree_node* aatree_node_successor(struct aatree_node* node);
static struct aatree_node* aatree_node_predecessor(struct aatree_node* node);

struct aatree* aatree_create(aatree_compare compare) {
	struct aatree* self = (struct aatree*)calloc(1, sizeof(struct aatree));
	if(!self)
		return NULL;

	self->compare = compare;
	return self;
}

void aatree_destroy(struct aatree* self) {
	aatree_node_destroy(self->root->left, true);
	aatree_node_destroy(self->root->right ,true);
	memset(self, 0, sizeof(struct aatree)), free(self);
}

int aatree_insert(struct aatree* self, void* value) {
	struct aatree_node* inserted = aatree_node_insert(self, self->root, value);
	if(!inserted)
		return 1;

	self->root = inserted;
	self->size += 1;
	return 0;
}

void* aatree_remove(struct aatree* self, void* value) {
	if(!self->size)
		return NULL;

	/* if(!aatree_find(self, value)) */
	/* 	return NULL; */

	if(self->size == 1 && self->compare(self->root, value) == 0) {
		void* value = self->root->value;
		aatree_node_destroy(self->root, false);
		self->root = NULL;
		self->size = 0;
		return value;
	}

	struct aatree_node* removed = aatree_node_remove(self, self->root, value);
	if(!removed)
		return NULL;

	self->root = removed;
	self->size -= 1;
	return NULL;
}

size_t aatree_size(struct aatree* self) {
	return self->size;
}

void* aatree_find(struct aatree* self, void* value) {
	if(!self->size)
		return NULL;

	// perform 2-way search for better performance
	// @see [A Note on Searching in a Binary Search Tree](http://user.it.uu.se/~arnea/ps/searchproc.pdf)
	struct aatree_node* node = self->root;
	struct aatree_node* candidate = NULL;
	while(node) {
		if(self->compare(node->value, value) > 0)
			node = node->left;
		else {
			candidate = node;
			node = node->right;
		}
	}
	if(candidate && self->compare(candidate->value, value) == 0)
		return candidate;
	else
		return NULL;
}

void* aatree_find_min(struct aatree* self) {
	if(!self->size)
		return NULL;

	struct aatree_node* node = self->root;
	while(node->left)
		node = node->left;
	return node->value;
}

void* aatree_find_max(struct aatree* self) {
	if(!self->size)
		return NULL;

	struct aatree_node* node = self->root;
	while(node->right)
		node = node->right;
	return node->value;
}

static struct aatree_node* aatree_node_create(int level, void* value) {
	struct aatree_node* node = (struct aatree_node*)calloc(1, sizeof(struct aatree_node));
	if(!node)
		return NULL;

	node->level = level;
	node->value = value;
	return node;
}

static void aatree_node_destroy(struct aatree_node* node, bool recursively) {
	if(!node)
		return;

	if(recursively) {
		aatree_node_destroy(node->left, true);
		aatree_node_destroy(node->right, true);
	}

	free(node);
}

static struct aatree_node* aatree_node_skew(struct aatree_node* node) {
	if(!node || !node->left)
		return node;

	if(node->level == node->left->level) {
		struct aatree_node* left = node->left;
		node->left = left->right;
		left->right = node;
		return left;
	}

	return node;
}

static struct aatree_node* aatree_node_split(struct aatree_node* node) {
	if(!node || !node->right || !node->right->right)
		return node;

	if(node->level == node->right->right->level) {
		struct aatree_node* right = node->right;
		node->right = right->left;
		right->left = node;
		right->level += 1;
		return right;
	}

	return node;
}

static struct aatree_node* aatree_node_insert(struct aatree* self, struct aatree_node* node, void* value) {
	if(!node)
		return aatree_node_create(1, value);

	if(self->compare(value, node->value) < 0)
		node->left = aatree_node_insert(self, node->left, value);
	else if(self->compare(value, node->value) > 0)
		node->right = aatree_node_insert(self, node->right, value);

	node = aatree_node_skew(node);
	node = aatree_node_split(node);
	return node;
}

static struct aatree_node* aatree_node_remove(struct aatree* self, struct aatree_node* node, void* value) {
	if(!node)
		return NULL;
	else if(self->compare(value, node->value) > 0)
		node->right = aatree_node_remove(self, node->right, value);
	else if(self->compare(value, node->value) < 0)
		node->left = aatree_node_remove(self, node->left, value);
	else {
		if(!node->left && !node->right) {
			aatree_node_destroy(node, false);
			return NULL;
		}

		if(!node->left) {
			struct aatree_node* successor = aatree_node_successor(node);
			node->right = aatree_node_remove(self, node->right, successor->value);
			node->value = successor->value;
		} else {
			struct aatree_node* predecessor = aatree_node_predecessor(node);
			node->left = aatree_node_remove(self, node->left, predecessor->value);
			node->value = predecessor->value;
		}
	}

	node = aatree_node_decrease_level(node);

	node = aatree_node_skew(node);
	node->right = aatree_node_skew(node->right);
	if(node->right)
		node->right->right = aatree_node_skew(node->right->right);

	node = aatree_node_split(node);
	node->right = aatree_node_split(node->right);

	return node;
}

static struct aatree_node* aatree_node_decrease_level(struct aatree_node* node) {
	if(!node || !node->left || !node->right)
		return node;

	int should_be = node->left->level < node->right->level ? : node->right->level;

	if(should_be < node->level) {
		node->level = should_be;
		if(should_be < node->right->level)
			node->right->level = should_be;
	}

	return node;
}

static struct aatree_node* aatree_node_successor(struct aatree_node* node) {
	struct aatree_node* successor = node->right;
	while(successor->left)
		successor = successor->left;
	return successor;
}

static struct aatree_node* aatree_node_predecessor(struct aatree_node* node) {
	struct aatree_node* predecessor = node->left;
	while(predecessor->right)
		predecessor = predecessor->right;
	return predecessor;
}

#ifndef NDBUG
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
void aatree_node_dump(struct aatree_node* base, int indent) {
	if(!base)
		return;

	printf("\x1B[32m@%p \x1B[0m", base);
	for(int i = 0; i < indent; ++i)
		putchar(' ');

	printf("value[%ld] level[%d] left[%p] right[%p]\n", (intptr_t)base->value, base->level, base->left, base->right);
	aatree_node_dump(base->left, indent + 2);
	aatree_node_dump(base->right, indent + 2);
}

int int_compare(void* a_lhs, void* a_rhs) {
	return (intptr_t)a_lhs - (intptr_t)a_rhs;
}

int main(int argc, char** argv) {
	struct aatree* tree = aatree_create(int_compare);
	for(intptr_t i = 10; i < 16; ++i)
		aatree_insert(tree, (void*)i);
	aatree_node_dump(tree->root, 0), puts("");

	printf("min: %ld\n", (intptr_t)aatree_find_min(tree));
	printf("max: %ld\n", (intptr_t)aatree_find_max(tree));

	for(intptr_t i = 10; i < 16; ++i)
		aatree_remove(tree, (void*)i);

	for(intptr_t i = 10; i < 16; ++i)
		aatree_insert(tree, (void*)i);
	aatree_node_dump(tree->root, 0), puts("");

	aatree_remove(tree, (void*)15);
	aatree_insert(tree, (void*)15);
	aatree_remove(tree, (void*)13);
	aatree_insert(tree, (void*)20);
	aatree_insert(tree, (void*)1);
	aatree_insert(tree, (void*)2);
	aatree_node_dump(tree->root, 0), puts("");

	aatree_destroy(tree);
	tree = aatree_create(int_compare);

	srand(time(NULL));
	while(1) {
		intptr_t value = rand() % 300;
		if(rand() % 9 < 1) {
			printf("inserting value[%ld]\n", value);
			printf("====================\n");
			aatree_insert(tree, (void*)value);
		} else {
			printf("removing value[%ld]\n", value);
			printf("====================\n");
			aatree_remove(tree, (void*)value);
		}
		aatree_node_dump(tree->root, 0);
		usleep(150 * 1000);
		int x = system("clear");
	}

	return 0;
}
#endif
