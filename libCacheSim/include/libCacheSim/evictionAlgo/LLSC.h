//
//  LLSC.h
//  libCacheSim
//

#pragma once

#include "../cache.h"


#define USE_XGBOOST

#ifdef USE_XGBOOST
#include <xgboost/c_api.h>
//#include <LightGBM/c_api.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_N_BUCKET 120
#define N_TRAIN_ITER 8
#define N_MAX_VALIDATION 1000

/* change to 10 will speed up */
#define N_FEATURE_TIME_WINDOW 30

//#define TRAINING_CONSIDER_RETAIN    1


#define TRAINING_TRUTH_ORACLE   1
#define TRAINING_TRUTH_ONLINE   2
#define TRAINING_TRUTH    TRAINING_TRUTH_ONLINE

#define ADAPTIVE_RANK  1

/*********** exp *************/
//#define dump_ranked_seg_frac 0.05
//#undef ADAPTIVE_RANK


//#define DUMP_MODEL 1

#define HIT_PROB_MAX_AGE 86400
//#define HIT_PROB_MAX_AGE 172800
/* enable this on i5 slows down by two times */
//#define HIT_PROB_MAX_AGE 864000    /* 10 day for akamai */
#define HIT_PROB_COMPUTE_INTVL 1000000
#define LHD_EWMA 0.9

#define MAGIC 1234567890

#define DEFAULT_RANK_INTVL 20

#ifdef USE_XGBOOST
typedef float feature_t;
typedef float pred_t;
typedef float train_y_t;
#elif defined(USE_GBM)
typedef double feature_t;
typedef double pred_t;
typedef float train_y_t;
#else
typedef double feature_t;
typedef double pred_t;
typedef double train_y_t;
#endif

typedef enum {
  SEGCACHE = 0,
  SEGCACHE_ITEM_ORACLE = 1,
  SEGCACHE_SEG_ORACLE = 2,
  SEGCACHE_BOTH_ORACLE = 3,

  LOGCACHE_START_POS = 4,

  LOGCACHE_BOTH_ORACLE = 5,
  LOGCACHE_LOG_ORACLE = 6,
  LOGCACHE_ITEM_ORACLE = 7,
  LOGCACHE_LEARNED = 8,
  //  LOGCACHE_RAMCLOUD,
  //  LOGCACHE_FIFO,
} LSC_type_e;

typedef enum obj_score_type {
  OBJ_SCORE_FREQ = 0,
  OBJ_SCORE_FREQ_BYTE = 1,
  OBJ_SCORE_FREQ_AGE = 2,

  OBJ_SCORE_HIT_DENSITY = 3,

  OBJ_SCORE_ORACLE = 4,
} obj_score_e;

typedef enum bucket_type {
  NO_BUCKET = 0,

  SIZE_BUCKET = 1,
  TTL_BUCKET = 2,
  CUSTOMER_BUCKET = 3,
  BUCKET_ID_BUCKET = 4,
  CONTENT_TYPE_BUCKET = 5,
} bucket_type_e;

typedef struct {
  int segment_size;
  int n_merge;
  LSC_type_e type;
  int size_bucket_base;
  int rank_intvl;
  int min_start_train_seg;
  int max_start_train_seg;
  int n_train_seg_growth;
  int sample_every_n_seg_for_training;
  int re_train_intvl;
  int hit_density_age_shift;
  bucket_type_e bucket_type;
} LLSC_init_params_t;


typedef struct {
  int32_t n_hit_per_min[N_FEATURE_TIME_WINDOW];
//  int16_t n_active_item_per_min[N_FEATURE_TIME_WINDOW];

  int32_t n_hit_per_ten_min[N_FEATURE_TIME_WINDOW];
//  int16_t n_active_item_per_ten_min[N_FEATURE_TIME_WINDOW];

  int32_t n_hit_per_hour[N_FEATURE_TIME_WINDOW];
//  int16_t n_active_item_per_hour[N_FEATURE_TIME_WINDOW];

  int64_t last_min_window_ts;
  int64_t last_ten_min_window_ts;
  int64_t last_hour_window_ts;
} seg_feature_t;

typedef struct learner {
  int min_start_train_seg;
  int max_start_train_seg;
  int n_train_seg_growth;
  int next_n_train_seg;
  int sample_every_n_seg_for_training;
  int64_t last_train_vtime;
  int64_t last_train_rtime;
  int re_train_intvl;

  //  int32_t feature_history_time_window;
//  bool start_feature_recording;
  //  bool start_train;
//  int64_t start_feature_recording_time;

#ifdef USE_XGBOOST
  BoosterHandle booster;
  DMatrixHandle train_dm;
  DMatrixHandle valid_dm;
  DMatrixHandle inf_dm;
#elif defined(USE_GBM)
  DatasetHandle train_dm;

#endif

  int n_train;
  int n_inference;

  int n_feature;

  feature_t *training_x;
  train_y_t *training_y;
  feature_t *valid_x;
  train_y_t *valid_y;
  pred_t *valid_pred_y;


  int32_t n_training_samples;
  int32_t n_valid_samples;
  //  int32_t n_training_iter;

  int32_t training_n_row;
  int32_t validation_n_row;
  int32_t inference_n_row;

  feature_t *inference_data;
  pred_t *pred;


  int n_evictions_last_hour;
  int64_t last_hour_ts;

  //  int64_t last_inference_time;

//  int32_t full_cache_write_time; /* real time */
//  int32_t last_cache_full_time;  /* real time */
  int64_t n_byte_written;        /* cleared after each full cache write */

} learner_t;

typedef struct segment {
  //  cache_obj_t *first_obj;
  //  cache_obj_t *last_obj;
  cache_obj_t *objs;
  int64_t total_byte;
  int32_t n_total_obj;
  int32_t bucket_idx;

  struct segment *prev_seg;
  struct segment *next_seg;
  int32_t seg_id;
  double penalty;
  int n_skipped_penalty;

  int32_t n_total_hit;
  int32_t n_total_active;

  int32_t create_rtime;
  int64_t create_vtime;
  double req_rate; /* req rate when create the seg */
  double write_rate;
  double write_ratio;
  double cold_miss_ratio;

  int64_t eviction_vtime;
  int64_t eviction_rtime;

  seg_feature_t feature;

  int n_merge; /* number of merged times */
  bool is_training_seg;
  int32_t magic;
} segment_t;

typedef struct hitProb {
  int32_t n_hit[HIT_PROB_MAX_AGE];
  int32_t n_evict[HIT_PROB_MAX_AGE];

  double hit_density[HIT_PROB_MAX_AGE];
  int64_t n_overflow;
  int32_t age_shift;
} hitProb_t;

typedef struct {
  int64_t bucket_property;
  segment_t *first_seg;
  segment_t *last_seg;
  int32_t n_seg;
  int16_t bucket_idx;

  hitProb_t hit_prob;
} bucket_t;

typedef struct seg_sel {
  int64_t last_rank_time;
  segment_t **ranked_segs;
  int32_t ranked_seg_size;
  int32_t ranked_seg_pos;

  double *score_array;
  int score_array_size;

  //  double *obj_penalty;
  //  int obj_penalty_array_size;

} seg_sel_t;

typedef struct {
  int segment_size; /* in temrs of number of objects */
  int n_merge;
  int n_retain_from_seg;

  int n_used_buckets;
  bucket_t buckets[MAX_N_BUCKET];
  bucket_t training_bucket; /* segments that are evicted and used for training */

  int curr_evict_bucket_idx;
  segment_t *next_evict_segment;

  int32_t n_segs;
  int32_t n_training_segs;
  int64_t n_allocated_segs;
  int64_t n_evictions;

  int64_t start_rtime;
  int64_t curr_rtime;
  int64_t curr_vtime;

  seg_sel_t seg_sel;
  segment_t **segs_to_evict;

  learner_t learner;

  //  int64_t last_hit_prob_compute_rtime;
  int64_t last_hit_prob_compute_vtime;

  int n_req_since_last_eviction;
  int n_miss_since_last_eviction;

  int size_bucket_base;
  LSC_type_e type;
  bucket_type_e bucket_type;
  obj_score_e obj_score_type;
  int rank_intvl; /* in number of evictions */
} LLSC_params_t;

cache_t *LLSC_init(common_cache_params_t ccache_params, void *cache_specific_params);

void LLSC_free(cache_t *cache);

cache_ck_res_e LLSC_check(cache_t *cache, request_t *req, bool update_cache);

cache_ck_res_e LLSC_get(cache_t *cache, request_t *req);

void LLSC_insert(cache_t *LLSC, request_t *req);

void LLSC_evict(cache_t *cache, request_t *req, cache_obj_t *evicted_obj);

void LLSC_remove_obj(cache_t *cache, cache_obj_t *cache_obj);

#ifdef __cplusplus
}
#endif