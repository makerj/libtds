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
#include <assert.h>

#include "aatree.h"

static struct aatree_node* aatree_node_create(struct aatree* self, int level, void* value);
static void aatree_node_destroy(struct aatree* self, struct aatree_node* node, bool recursively);
static void aatree_node_insert(struct aatree* self, struct aatree_node** node, void* value);
static void aatree_node_remove(struct aatree* self, struct aatree_node** a_node, void* value);

struct aatree* aatree_create(aatree_compare compare) {
	struct aatree* self = (struct aatree*)calloc(1, sizeof(struct aatree));
	if(!self)
		return NULL;

	self->priv.bottom.left = &self->priv.bottom;
	self->priv.bottom.right = &self->priv.bottom;
	self->priv.deleted = &self->priv.bottom;

	self->compare = compare;
	return self;
}

void aatree_destroy(struct aatree* self) {
	aatree_node_destroy(self, self->root->left, true);
	aatree_node_destroy(self, self->root->right ,true);
	memset(self, 0, sizeof(struct aatree)), free(self);
}

int aatree_insert(struct aatree* self, void* value) {
	if(!self->size) {
		self->root = aatree_node_create(self, 1, value);
		if(!self->root)
			return 1;

		self->size += 1;
		return 0;
	}

	aatree_node_insert(self, &self->root, value);
	if(self->priv.error) {
		self->priv.error = 0;
		return 1;
	}

	self->size += 1;
	return 0;
}

int aatree_remove(struct aatree* self, void* value) {
	if(!self->size)
		return 1;

	aatree_node_remove(self, &self->root, value);
	if(self->priv.remove_performed) {
		self->priv.remove_performed = 0;
		self->priv.removed_value = NULL;
		self->size -= 1;
	}

	if(self->size)
		assert(self->root != &self->priv.bottom);

	return 0;
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
	while(node->left != &self->priv.bottom)
		node = node->left;
	return node->value;
}

void* aatree_find_max(struct aatree* self) {
	if(!self->size)
		return NULL;

	struct aatree_node* node = self->root;
	while(node->right != &self->priv.bottom)
		node = node->right;
	return node->value;
}

void aatree_node_iterate(struct aatree* self, struct aatree_node* node, aatree_iteration_callback callback, void* callback_context, bool foward) {
	if(node == &self->priv.bottom)
		return;

	if(foward) {
		aatree_node_iterate(self, node->left, callback, callback_context, true);
		callback(self, node->value, callback_context);
		aatree_node_iterate(self, node->right, callback, callback_context, true);
	} else {
		aatree_node_iterate(self, node->right, callback, callback_context, false);
		callback(self, node->value, callback_context);
		aatree_node_iterate(self, node->left, callback, callback_context, false);
	}
}

void aatree_iterate_foward(struct aatree* self, aatree_iteration_callback callback, void* callback_context) {
	if(self->size && callback)
		aatree_node_iterate(self, self->root, callback, callback_context, true);
}

void aatree_iterate_backward(struct aatree* self, aatree_iteration_callback callback, void* callback_context) {
	if(self->size && callback)
		aatree_node_iterate(self, self->root, callback, callback_context, false);
}

static struct aatree_node* aatree_node_create(struct aatree* self, int level, void* value) {
	struct aatree_node* node = (struct aatree_node*)calloc(1, sizeof(struct aatree_node));
	if(!node)
		return NULL;

	node->level = level;
	node->value = value;
	node->left = &self->priv.bottom;
	node->right = &self->priv.bottom;
	return node;
}

static void aatree_node_destroy(struct aatree* self, struct aatree_node* node, bool recursively) {
	if(node == &self->priv.bottom)
		return;

	if(recursively) {
		aatree_node_destroy(self, node->left, true);
		aatree_node_destroy(self, node->right, true);
	}

	free(node);
}

// skew (rotate right)
static void aatree_node_skew(struct aatree* self, struct aatree_node** a_node) {
	if((*a_node)->left->level != (*a_node)->level)
		return;

	struct aatree_node* temp;
	struct aatree_node* node = *a_node;
	temp = node;
	node = node->left;
	temp->left = node->right;
	node->right = temp;
	*a_node = node;
}

// split (rotate left)
static void aatree_node_split(struct aatree* self, struct aatree_node** a_node) {
	if((*a_node)->right->right->level != (*a_node)->level)
		return;

	struct aatree_node* temp;
	struct aatree_node* node = *a_node;
	temp = node;
	node = node->right;
	temp->right = node->left;
	node->left = temp;
	node->level += 1;
	*a_node = node;
}

static void aatree_node_insert(struct aatree* self, struct aatree_node** a_node, void* value) {
	if((*a_node) == &self->priv.bottom) {
		struct aatree_node* node = aatree_node_create(self, 1, value);
		if(!node) {
			self->priv.error = 1;
			return;
		}

		*a_node = node;
		return;
	}

	if(self->compare(value, (*a_node)->value) < 0)
		aatree_node_insert(self, &(*a_node)->left, value);
	else if(self->compare(value, (*a_node)->value) > 0)
		aatree_node_insert(self, &(*a_node)->right, value);
	else
		self->priv.error = 1; // key conflict

	if(!self->priv.error) {
		aatree_node_skew(self, a_node);
		aatree_node_split(self, a_node);
	}
}

static void aatree_node_remove(struct aatree* self, struct aatree_node** a_node, void* value) {
	if(*a_node == &self->priv.bottom)
		return;

	// search down the tree and set pointers last and deleted
	struct aatree_node* node = *a_node;
	self->priv.last = node;

	if(self->compare(value, node->value) < 0) {
		aatree_node_remove(self, &node->left, value);
	} else {
		self->priv.deleted = node;
		aatree_node_remove(self, &node->right, value);
	}

	if(node == self->priv.last && self->priv.deleted != &self->priv.bottom
			&& self->compare(value, self->priv.deleted->value) == 0) {
		// at the bottom of the tree we remove the element if it is present
		self->priv.remove_performed = 1;
		self->priv.removed_value = self->priv.deleted->value;
		self->priv.deleted->value = node->value;
		self->priv.deleted = &self->priv.bottom;
		*a_node = node->right;
		aatree_node_destroy(self, self->priv.last, false);
	} else if(node->left->level < node->level - 1 || node->right->level < node->level -1) {
		// on the way back, we rebalance
		node->level -= 1;
		if(node->right->level > node->level)
			node->right->level = node->level;

		aatree_node_skew(self, a_node), node = *a_node;
		aatree_node_skew(self, &node->right);
		aatree_node_skew(self, &node->right->right);

		aatree_node_split(self, a_node), node = *a_node;
		aatree_node_split(self, &node->right);
	}
}

#ifndef NDBUG
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
int aatree_node_dump(struct aatree* self, struct aatree_node* base, int indent) {
	if(indent == 0) {
		printf("tree information\n");
		printf("size[%zu] bottom[%p]\n", self->size, &self->priv.bottom);
		printf("================\n");
		if(!self->size)
			return 0;
	}

	if(!base || base == &self->priv.bottom)
		return 0;

	int nchild = 0;
	if(base->left != &self->priv.bottom)
		nchild += 1;
	if(base->right != &self->priv.bottom)
		nchild += 1;

	printf("\x1B[32m@%p \x1B[0m", base);
	for(int i = 0; i < indent; ++i)
		putchar(' ');

	printf("value[%ld] level[%d] left[%p] right[%p]\n",
			(intptr_t)base->value, base->level, base->left, base->right);

	nchild += aatree_node_dump(self, base->left, indent + 2);
	nchild += aatree_node_dump(self, base->right, indent + 2);

	if(indent == 0)
		return nchild + 1;

	return nchild;
}

void print_value(struct aatree* self, void* value, void* context) {
	printf("%ld\n", (intptr_t)value);
}

int int_compare(void* a_lhs, void* a_rhs) {
	return (intptr_t)a_lhs - (intptr_t)a_rhs;
}

int main(int argc, char** argv) {
	struct aatree* tree = aatree_create(int_compare);
	for(intptr_t i = 10; i < 16; ++i) {
		printf("inserting %ld...\n", i);
		aatree_insert(tree, (void*)i);
		assert(aatree_node_dump(tree, tree->root, 0) == tree->size), puts("");
		/* sleep(1); */
	}
	aatree_node_dump(tree, tree->root, 0), puts("");

	printf("min: %ld\n", (intptr_t)aatree_find_min(tree));
	printf("max: %ld\n", (intptr_t)aatree_find_max(tree));

	puts("foward iteration");
	aatree_iterate_foward(tree, print_value, NULL);
	puts("backward iteration");
	aatree_iterate_backward(tree, print_value, NULL);
	exit(0);

	for(intptr_t i = 10; i < 16; ++i) {
		printf("removing %ld...\n", i);
		aatree_remove(tree, (void*)i);
		assert(aatree_node_dump(tree, tree->root, 0) == tree->size), puts("");
		/* sleep(1); */
	}
	aatree_node_dump(tree, tree->root, 0), puts("");

	for(intptr_t i = 10; i < 16; ++i)
		aatree_insert(tree, (void*)i);
	aatree_node_dump(tree, tree->root, 0), puts("");

	aatree_remove(tree, (void*)15);
	aatree_insert(tree, (void*)15);
	aatree_remove(tree, (void*)13);
	aatree_insert(tree, (void*)20);
	aatree_insert(tree, (void*)1);
	aatree_insert(tree, (void*)2);
	aatree_node_dump(tree, tree->root, 0), puts("");

	aatree_destroy(tree);
	tree = aatree_create(int_compare);

	srand(time(NULL));
	while(1) {
		intptr_t value = rand() % 300 + 1;
		if(rand() % 9 < 2) {
			printf("inserting value[%ld]\n", value);
			printf("====================\n");
			aatree_insert(tree, (void*)value);
		} else {
			printf("removing value[%ld]\n", value);
			printf("====================\n");
			aatree_remove(tree, (void*)value);
		}
		int nchild = aatree_node_dump(tree, tree->root, 0);
		printf("tree->size[%d] nchild[%d]\n", tree->size, nchild);
		assert(nchild == tree->size);
		/* usleep(120 * 1000); */
		/* int x = system("clear"); */
	}

	return 0;
}
#endif
