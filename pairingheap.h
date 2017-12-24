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

#ifndef __PAIRINGHEAP_H__
#define __PAIRINGHEAP_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef int (*pairingheap_compare)(void* lhs, void* rhs);

/**
 * Node for Pairing Queue
 */
struct pairingheap_node {
	void* value;
	struct pairingheap_node* down;
	struct pairingheap_node* right;
};

/**
 * Pairing Queue
 */
struct pairingheap {
	struct pairingheap_node* root;
	size_t size;
	pairingheap_compare compare;
};

#ifdef __cplusplus
extern "C" {
#endif

struct pairingheap* pairingheap_create(pairingheap_compare compare);
void pairingheap_destroy(struct pairingheap* self);
size_t pairingheap_size(struct pairingheap* self);

int pairingheap_push(struct pairingheap* self, void* value);
void* pairingheap_pop(struct pairingheap* self);
void* pairingheap_peek(struct pairingheap* self);

#ifdef __cplusplus
}
#endif

#endif
