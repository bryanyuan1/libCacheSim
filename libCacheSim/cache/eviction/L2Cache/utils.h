#pragma once

#include "../../include/libCacheSim/evictionAlgo/L2Cache.h"
#include "L2CacheInternal.h"


// static inline bucket_t *next_nonempty_bucket(cache_t *cache, int curr_idx) {
//   L2Cache_params_t *params = cache->eviction_params;
//   curr_idx = (curr_idx + 1) % MAX_N_BUCKET;
//   while (params->buckets[curr_idx].n_seg < params->n_merge) {
//     curr_idx = (curr_idx + 1) % MAX_N_BUCKET;
//   }
//   return &params->buckets[curr_idx];
// }


static inline int count_hash_chain_len(cache_obj_t *cache_obj) {
  int n = 0;
  while (cache_obj) {
    n += 1;
    cache_obj = cache_obj->hash_next;
  }
  return n;
}

static inline int cmp_double(const void *p1, const void *p2) {
  if (*(double *) p1 < *(double *) p2) return -1;
  else
    return 1;
}

static int find_argmin_float(const float *y, int n_elem) {
  float min_val = (float) INT64_MAX;
  int min_val_pos = -1;
  for (int i = 0; i < n_elem; i++) {
    if (y[i] < min_val) {
      min_val = y[i];
      min_val_pos = i;
    }
  }
  DEBUG_ASSERT(min_val_pos != -1);
  return min_val_pos;
}

static int find_argmin_double(const double *y, int n_elem) {
  double min_val = (double) INT64_MAX;
  int min_val_pos = -1;
  for (int i = 0; i < n_elem; i++) {
    if (y[i] < min_val) {
      min_val = y[i];
      min_val_pos = i;
    }
  }
  DEBUG_ASSERT(min_val_pos != -1);
  return min_val_pos;
}


