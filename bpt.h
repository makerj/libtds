#ifndef __BPT_H__
#define __BPT_H__

/**
 * @file
 * B+ Tree implementation
 */

#include <stdbool.h>
#include <stdlib.h>

/**
 * bplus tree node
 */
struct bpt_node {
	struct bpt_node*	parent;		///< parent node
	bool				is_leaf;	///< indicates node is leaf or not
	int*				keys;		///< array of key
	int					num_keys;	///< number of key
	void**				pointers;	///< another node, or record on leaf node
};

/**
 * bplus tree
 */
struct bpt {
	struct bpt_node* root;	///< tree root
	size_t size;			///< number of element
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a new bplus tree
 *
 * @return newly created bplus tree
 */
struct bpt* bpt_create();

/**
 * Destroy a bplus tree
 *
 * @param self bplus tree
 */
void bpt_destroy(struct bpt* self);

/**
 * Put key and value into bplus tree
 *
 * @param self bplus tree
 * @param key key
 * @param value value
 *
 * @return key and value inserted or not
 */
bool bpt_put(struct bpt* self, void* key, void* value);

/**
 * Remove element using key
 *
 * @param self bplus tree
 * @param key key
 *
 * @return element is removed or not
 */
bool bpt_remove(struct bpt* self, void* key);

/**
 * Get element from bplus tree
 *
 * @param self bplus tree
 * @param key key
 *
 * @return element or NULL
 */
void* bpt_get(struct bpt* self, void* key);

/**
 * Get elements from bplus tree
 *
 * @param self bplus tree
 * @param key_start range start
 * @param key_end range end
 * @param values value holder
 * @param values_size value holder capacity
 *
 * @return number of matched elements
 */
size_t bpt_get_ranged(struct bpt* self, void* key_start, void* key_end, void** values, size_t values_size);

#ifdef __cplusplus
}
#endif

#endif
