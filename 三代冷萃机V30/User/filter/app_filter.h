#ifndef __APP_FILTER_H__
#define __APP_FILTER_H__

#include "gd32f30x.h"

#define DEFAULT_ALPHA 0.1f

/**
 * 滤波器结构体
 */
typedef struct {
    uint32_t filtered_value;    // 滤波后的值
    int initialized;            // 初始化标志
    float alpha;                // 滤波系数
} EwmaFilter;

void ewma_filter_init(EwmaFilter *filter, float alpha);
uint32_t ewma_filter_apply(EwmaFilter *filter, uint32_t new_value);

#endif

