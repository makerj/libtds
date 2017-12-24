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

#ifndef __DOUBLYLINKEDLIST_H__
#define __DOUBLYLINKEDLIST_H__

#include <stdint.h>
#include <stdlib.h>

/**
 * Node for Doubly-linked circular list
 */
struct doublylinkedlist_node {
	void* value;
	struct doublylinkedlist_node* prev;
	struct doublylinkedlist_node* next;
};

/**
 * Doubly-linked circular list
 */
struct doublylinkedlist {
	struct doublylinkedlist_node* head;
	size_t size;
};

#ifdef __cplusplus
extern "C" {
#endif

struct doublylinkedlist* doublylinkedlist_create();
void doublylinkedlist_destroy(struct doublylinkedlist* self);
size_t doublylinkedlist_size(struct doublylinkedlist* self);
struct doublylinkedlist_node* doublylinkedlist_at(struct doublylinkedlist* self, size_t index);

int doublylinkedlist_push_front(struct doublylinkedlist* self, void* value);
int doublylinkedlist_push_back(struct doublylinkedlist* self, void* value);
int doublylinkedlist_push_at(struct doublylinkedlist* self, size_t index, void* value);

struct doublylinkedlist_node* doublylinkedlist_pop_back(struct doublylinkedlist* self);
struct doublylinkedlist_node* doublylinkedlist_pop_at(struct doublylinkedlist* self, size_t index);

size_t doublylinkedlist_merge(struct doublylinkedlist* dst, struct doublylinkedlist* src);

#ifdef __cplusplus
}
#endif

#endif
