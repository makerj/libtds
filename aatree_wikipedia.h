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

#ifndef __AATREE_H__
#define __AATREE_H__

/**
 * @file
 * Arne Andersson self-balancing binary search tree implementation
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef int (*aatree_compare)(void* lhs, void* rhs);

struct aatree_node {
	void* value;
	struct aatree_node* left;
	struct aatree_node* right;
	int level;
};

struct aatree {
	struct aatree_node* root;
	size_t size;
	aatree_compare compare;
};

#ifdef __cplusplus
extern "C" {
#endif

struct aatree* aatree_create(aatree_compare compare);
void aatree_destroy(struct aatree* self);

int aatree_insert(struct aatree* self, void* value);
void* aatree_remove(struct aatree* self, void* value);
size_t aatree_size(struct aatree* self);

void* aatree_find(struct aatree* self, void* value);
void* aatree_find_min(struct aatree* self);
void* aatree_find_max(struct aatree* self);

#ifdef __cplusplus
}
#endif

#endif
