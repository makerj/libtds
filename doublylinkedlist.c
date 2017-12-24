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

#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <assert.h>

#include "doublylinkedlist.h"

struct doublylinkedlist* doublylinkedlist_create() {
	return (struct doublylinkedlist*)calloc(1, sizeof(struct doublylinkedlist));
}

void doublylinkedlist_destroy(struct doublylinkedlist* self) {
	if(self->size) {
		struct doublylinkedlist_node* node = self->head;

		for(int i = 0; i < self->size; ++i) {
			struct doublylinkedlist_node* tmp = node;
			node = node->next;
			free(tmp);
		}
	}

	memset(self, 0, sizeof(struct doublylinkedlist)), free(self);
}

size_t doublylinkedlist_size(struct doublylinkedlist* self) {
	return self->size;
}

struct doublylinkedlist_node* doublylinkedlist_at(struct doublylinkedlist* self, size_t index) {
	if(index >= self->size)
		return NULL;

	struct doublylinkedlist_node* node = self->head;

	for(int i = 0; i < index; ++i) {
		node = node->next;
	}

	return node;
}

int doublylinkedlist_push_front(struct doublylinkedlist* self, void* value) {
	struct doublylinkedlist_node* node = (struct doublylinkedlist_node*)malloc(sizeof(struct doublylinkedlist_node));
	if(!node)
		return 1;

	if(!self->size) {
		node->value = value;
		node->prev = node;
		node->next = node;
	} else {
		struct doublylinkedlist_node* head = self->head;
		struct doublylinkedlist_node* tail = head->prev;

		node->value = value;
		node->prev = tail;
		node->next = head;

		tail->next = node;
		head->prev = node;
	}

	self->head = node;
	self->size += 1;
	return 0;
}

int doublylinkedlist_push_back(struct doublylinkedlist* self, void* value) {
	if(!self->size) {
		self->head = (struct doublylinkedlist_node*)malloc(sizeof(struct doublylinkedlist_node));
		if(!self->head)
			return 1;

		self->head->value = value;
		self->head->prev = self->head;
		self->head->next = self->head;
	} else {
		struct doublylinkedlist_node* node = (struct doublylinkedlist_node*)malloc(sizeof(struct doublylinkedlist_node));
		if(!node)
			return 2;

		struct doublylinkedlist_node* tail = self->head->prev;

		node->value = value;
		node->prev = tail;
		node->next = self->head;

		tail->next = node;
		self->head->prev = node;
	}

	self->size += 1;
	return 0;
}

int doublylinkedlist_push_at(struct doublylinkedlist* self, size_t index, void* value) {
	if(index > self->size)
		return 1;
	if(index == 0)
		return doublylinkedlist_push_front(self, value);
	if(index == self->size)
		return doublylinkedlist_push_back(self, value);

	struct doublylinkedlist_node* older = doublylinkedlist_at(self, index);
	if(!older)
		return 2;

	struct doublylinkedlist_node* newer = (struct doublylinkedlist_node*)calloc(1, sizeof(struct doublylinkedlist_node));
	if(!newer)
		return 3;

	struct doublylinkedlist_node* prev = older->prev;
	prev->next = newer;
	older->prev = newer;

	newer->prev = prev;
	newer->next = older;

	if(older == self->head)
		self->head = newer;

	self->size += 1;
	return 0;
}

struct doublylinkedlist_node* doublylinkedlist_pop_back(struct doublylinkedlist* self) {
	if(!self->size)
		return NULL;

	struct doublylinkedlist_node* tail = self->head->prev;
	tail->prev->next = self->head;
	self->head->prev = tail->prev;
	self->size -= 1;

	return tail;
}


struct doublylinkedlist_node* doublylinkedlist_pop_at(struct doublylinkedlist* self, size_t index) {
	struct doublylinkedlist_node* node = doublylinkedlist_at(self, index);
	if(!node)
		return NULL;

	struct doublylinkedlist_node* prev = node->prev;
	struct doublylinkedlist_node* next = node->next;
	prev->next = next;
	next->prev = prev;

	if(node == self->head) {
		struct doublylinkedlist_node* tail = self->head->prev;
		self->head = tail->next;
	}

	self->size -= 1;
	return node;
}

size_t doublylinkedlist_merge(struct doublylinkedlist* dst, struct doublylinkedlist* src) {
	struct doublylinkedlist_node* node = src->head;
	size_t merged_node_count = 0;

	for(size_t i = 0; i < src->size; ++i) {
		int error = doublylinkedlist_push_back(dst, node->value);
		if(error)
			break;

		merged_node_count += 1;
		node = node->next;
	}

	return merged_node_count;
}

/* int main(int argc, char** argv) { */
/* 	setlocale(LC_NUMERIC, ""); */
/* 	puts("starting doublylinkedlist test suite..."); */
/* 	puts("======================================"); */

/* 	puts("invoking doublylinkedlist_create()"); */
/* 	struct doublylinkedlist* list = doublylinkedlist_create(); */
/* 	assert(list != NULL); */

/* 	puts("invoking doublylinkedlist_push_back() 5 times"); */
/* 	doublylinkedlist_push_back(list, (void*)1); */
/* 	doublylinkedlist_push_back(list, (void*)2); */
/* 	doublylinkedlist_push_back(list, (void*)3); */
/* 	doublylinkedlist_push_back(list, (void*)4); */
/* 	doublylinkedlist_push_back(list, (void*)5); */

/* 	puts("invoking doublylinkedlist_size()"); */
/* 	assert(5 == doublylinkedlist_size(list)); */

/* 	puts("invoking doublylinkedlist_at() 5 times"); */
/* 	for(int i = 0; i < 5; ++i) { */
/* 		assert(i + 1 == (intptr_t)doublylinkedlist_at(list, i)->value); */
/* 	} */

/* 	puts("invoking doublylinkedlist_pop_back() 5 times"); */
/* 	assert(5 == (intptr_t)doublylinkedlist_pop_back(list)->value); */
/* 	assert(4 == (intptr_t)doublylinkedlist_pop_back(list)->value); */
/* 	assert(3 == (intptr_t)doublylinkedlist_pop_back(list)->value); */
/* 	assert(2 == (intptr_t)doublylinkedlist_pop_back(list)->value); */
/* 	assert(1 == (intptr_t)doublylinkedlist_pop_back(list)->value); */

/* 	puts("invoking doublylinkedlist_size()"); */
/* 	assert(0 == doublylinkedlist_size(list)); */

/* 	puts("invoking doublylinkedlist_push_back() 5 times"); */
/* 	doublylinkedlist_push_back(list, (void*)1); */
/* 	doublylinkedlist_push_back(list, (void*)2); */
/* 	doublylinkedlist_push_back(list, (void*)3); */
/* 	doublylinkedlist_push_back(list, (void*)4); */
/* 	doublylinkedlist_push_back(list, (void*)5); */

/* 	puts("invoking doublylinkedlist_size()"); */
/* 	assert(5 == doublylinkedlist_size(list)); */

/* 	puts("invoking doublylinkedlist_pop_at(0) 5 times"); */
/* 	assert(1 == (intptr_t)doublylinkedlist_pop_at(list, 0)->value); */
/* 	assert(2 == (intptr_t)doublylinkedlist_pop_at(list, 0)->value); */
/* 	assert(3 == (intptr_t)doublylinkedlist_pop_at(list, 0)->value); */
/* 	assert(4 == (intptr_t)doublylinkedlist_pop_at(list, 0)->value); */
/* 	assert(5 == (intptr_t)doublylinkedlist_pop_at(list, 0)->value); */

/* 	puts("invoking doublylinkedlist_size()"); */
/* 	assert(0 == doublylinkedlist_size(list)); */

/* 	puts("invoking doublylinkedlist_push_back() 5 times"); */
/* 	doublylinkedlist_push_back(list, (void*)1); */
/* 	doublylinkedlist_push_back(list, (void*)2); */
/* 	doublylinkedlist_push_back(list, (void*)3); */
/* 	doublylinkedlist_push_back(list, (void*)4); */
/* 	doublylinkedlist_push_back(list, (void*)5); */

/* 	puts("invoking doublylinkedlist_size()"); */
/* 	assert(5 == doublylinkedlist_size(list)); */

/* 	puts("invoking doublylinkedlist_pop_at() 5 times"); */
/* 	assert(4 == (intptr_t)doublylinkedlist_pop_at(list, 3)->value); */
/* 	assert(1 == (intptr_t)doublylinkedlist_pop_at(list, 0)->value); */
/* 	assert(3 == (intptr_t)doublylinkedlist_pop_at(list, 1)->value); */
/* 	assert(2 == (intptr_t)doublylinkedlist_pop_at(list, 0)->value); */
/* 	assert(5 == (intptr_t)doublylinkedlist_pop_at(list, 0)->value); */

/* 	puts("invoking doublylinkedlist_size()"); */
/* 	assert(0 == doublylinkedlist_size(list)); */

/* 	puts("invoking doublylinkedlist_push_front() 5 times"); */
/* 	doublylinkedlist_push_front(list, (void*)1); */
/* 	doublylinkedlist_push_front(list, (void*)2); */
/* 	doublylinkedlist_push_front(list, (void*)3); */
/* 	doublylinkedlist_push_front(list, (void*)4); */
/* 	doublylinkedlist_push_front(list, (void*)5); */

/* 	puts("invoking doublylinkedlist_size()"); */
/* 	assert(5 == doublylinkedlist_size(list)); */

/* 	puts("invoking doublylinkedlist_pop_at(0) 5 times"); */
/* 	assert(5 == (intptr_t)doublylinkedlist_pop_at(list, 0)->value); */
/* 	assert(4 == (intptr_t)doublylinkedlist_pop_at(list, 0)->value); */
/* 	assert(3 == (intptr_t)doublylinkedlist_pop_at(list, 0)->value); */
/* 	assert(2 == (intptr_t)doublylinkedlist_pop_at(list, 0)->value); */
/* 	assert(1 == (intptr_t)doublylinkedlist_pop_at(list, 0)->value); */

/* 	puts("invoking doublylinkedlist_size()"); */
/* 	assert(0 == doublylinkedlist_size(list)); */

/* 	puts("invoking doublylinkedlist_destroy()"); */
/* 	doublylinkedlist_destroy(list); */

/* 	return 0; */
/* } */

