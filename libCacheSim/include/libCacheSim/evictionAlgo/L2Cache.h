//
//  L2Cache.h
//  libCacheSim
//

#pragma once

#include "../cache.h"

#define USE_XGBOOST

#ifdef USE_XGBOOST
#include <xgboost/c_api.h>
typedef float feature_t;
typedef float pred_t;
typedef float train_y_t;
#elif defined(USE_GBM)
#include <LightGBM/c_api.h>
typedef double feature_t;
typedef double pred_t;
typedef float train_y_t;
#else
#error "need to use XGBoost or LightGBM"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  SEGCACHE = 0,
  LOGCACHE_BOTH_ORACLE = 1,
  LOGCACHE_LOG_ORACLE = 2,
  LOGCACHE_ITEM_ORACLE = 3,  // FIFO for seg selection
  LOGCACHE_LEARNED = 4,
  //  LOGCACHE_RAMCLOUD,
  //  LOGCACHE_FIFO,
} L2Cache_type_e;

typedef enum obj_score_type {
  OBJ_SCORE_FREQ = 0,
  OBJ_SCORE_FREQ_BYTE = 1,
  OBJ_SCORE_FREQ_AGE = 2,
  OBJ_SCORE_FREQ_AGE_BYTE = 3,

  OBJ_SCORE_HIT_DENSITY = 4,

  OBJ_SCORE_ORACLE = 5,
  OBJ_SCORE_SIZE_AGE = 6,
} obj_score_type_e;

typedef enum bucket_type {
  NO_BUCKET = 0,

  SIZE_BUCKET = 1,
  TTL_BUCKET = 2,
  CUSTOMER_BUCKET = 3,
  BUCKET_ID_BUCKET = 4,
  CONTENT_TYPE_BUCKET = 5,
} bucket_type_e;

typedef enum training_source {
  TRAIN_Y_FROM_ONLINE,
  TRAIN_Y_FROM_ORACLE,
} train_source_e;

typedef struct {
  // how many objects in one segment
  int segment_size;
  // how many segments to merge (n_merge segments merge to one segment)
  int n_merge;
  double rank_intvl;  // how often to rank, in terms of fraction of total
                      // segments (0.0 - 1.0)
  // whether we merge consecutive segments (with the first segment has the
  // lowest utility) or we merge non-consecutive segments based on ranking
  bool merge_consecutive_segs;
  train_source_e train_source_x;
  train_source_e train_source_y;

  int retrain_intvl;

  int hit_density_age_shift;
  L2Cache_type_e type;
  obj_score_type_e obj_score_type;
  bucket_type_e bucket_type;
} L2Cache_init_params_t;

cache_t *L2Cache_init(const common_cache_params_t ccache_params,
                      const char *cache_specific_params);

void L2Cache_free(cache_t *cache);

cache_ck_res_e L2Cache_check(cache_t *cache, const request_t *req,
                             const bool update_cache);

cache_ck_res_e L2Cache_get(cache_t *cache, const request_t *req);

void L2Cache_insert(cache_t *L2Cache, const request_t *req);

void L2Cache_evict(cache_t *cache, const request_t *req,
                   cache_obj_t *evicted_obj);

void L2Cache_remove_obj(cache_t *cache, cache_obj_t *cache_obj);

void L2Cache_remove(cache_t *cache, const obj_id_t obj_id);

void L2Cache_set_default_init_params(L2Cache_init_params_t *params);

#ifdef __cplusplus
}
#endif
