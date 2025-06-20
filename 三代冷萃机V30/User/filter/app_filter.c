#include "app_filter.h"


/**
 * ************************************************************************
 * @brief  初始化滤波器
 * 
 * @param[in,out] filter 滤波器指针
 * @param[in] alpha      滤波系数 (0.0 ~ 1.0)
 * 
 * @return 无
 * 
 * @version 2.0
 * @author jiaokai 
 * @date 2025-04-29
 * 
 * ************************************************************************
 */
void ewma_filter_init(EwmaFilter *filter, float alpha) {
    if (filter) {
        filter->filtered_value = 0;
        filter->initialized = 0;
        filter->alpha = (alpha > 0.0f && alpha <= 1.0f) ? alpha : DEFAULT_ALPHA;
    }
}



/**
 * ************************************************************************
 * @brief  应用移动指数加权滤波
 * 
 * @param[in,out] filter    滤波器指针
 * @param[in] new_value     新数据
 * 
 * @return 滤波后数据
 * 
 * @version 2.0
 * @author jiaokai 
 * @date 2025-04-29
 * 
 * ************************************************************************
 */
uint32_t ewma_filter_apply(EwmaFilter *filter, uint32_t new_value) {
    if (!filter) {
        return new_value;  // 如果滤波器指针无效，直接返回原始值
    }

    // 如果滤波器未初始化，则初始化
    if (!filter->initialized) {
        filter->filtered_value = new_value;
        filter->initialized = 1;
    }

    // 应用滤波算法
    filter->filtered_value = (uint32_t)(filter->alpha * new_value + (1 - filter->alpha) * filter->filtered_value);
    
    return filter->filtered_value;
}

