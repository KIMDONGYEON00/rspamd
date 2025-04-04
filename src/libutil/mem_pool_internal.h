/*
 * Copyright 2025 Vsevolod Stakhov
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RSPAMD_MEM_POOL_INTERNAL_H
#define RSPAMD_MEM_POOL_INTERNAL_H

/*
 * Internal memory pool stuff
 */

#define align_ptr(p, a) \
	((uint8_t *) ((uintptr_t) (p) + ((-(intptr_t) (p)) & ((a) -1))))

enum rspamd_mempool_chain_type {
	RSPAMD_MEMPOOL_NORMAL = 0,
	RSPAMD_MEMPOOL_SHARED,
	RSPAMD_MEMPOOL_MAX
};
#define ENTRY_LEN 128
#define ENTRY_NELTS 64

struct entry_elt {
	uint32_t fragmentation;
	uint32_t leftover;
};

struct rspamd_mempool_entry_point {
	char src[ENTRY_LEN];
	uint32_t cur_suggestion;
	uint32_t cur_elts;
	uint32_t cur_vars;
	struct entry_elt elts[ENTRY_NELTS];
};

/**
 * Destructors list item structure
 */
struct _pool_destructors {
	rspamd_mempool_destruct_t func; /**< pointer to destructor					*/
	void *data;                     /**< data to free							*/
	const char *function;           /**< function from which this destructor was added */
	const char *loc;                /**< line number                            */
	struct _pool_destructors *next;
};


struct rspamd_mempool_variable {
	gpointer data;
	rspamd_mempool_destruct_t dtor;
};

KHASH_INIT(rspamd_mempool_vars_hash,
		   const char *, struct rspamd_mempool_variable, 1,
		   kh_str_hash_func, kh_str_hash_equal);

struct rspamd_mempool_specific {
	struct _pool_chain *pools[RSPAMD_MEMPOOL_MAX];
	struct _pool_destructors *dtors_head, *dtors_tail;
	GPtrArray *trash_stack;
	khash_t(rspamd_mempool_vars_hash) * variables;
	struct rspamd_mempool_entry_point *entry;
	gsize elt_len; /**< size of an element						*/
	gsize used_memory;
	unsigned int wasted_memory;
	int flags;
};

/**
 * Pool page structure
 */
struct _pool_chain {
	uint8_t *begin;   /**< begin of pool chain block              */
	uint8_t *pos;     /**< current start of free space in block   */
	gsize slice_size; /**< length of block                        */
	struct _pool_chain *next;
};


#endif
