#ifndef POWERLOG_H
#define POWERLOG_H

#include <sys/types.h>
#include "default_values.h"

/*
 *  Powerlog
 */
typedef struct powerlog_struct {

    struct timespec timestamp;    // GMT

#ifdef TRACE_POWER
    int32_t board_power;          // mW
    int32_t soc_power;            // mW
    int32_t wifi_power;           // mW
#endif   // TRACE_POWER

    // GPU informations
    int16_t gpu_util;             // x0.1%
    int16_t gpu_freq;             // MHz
    int16_t gpu_power;            // mW

#ifdef TRACE_CPU
    int16_t allcpu_power;         // mW
    int16_t cpu_freq[NUM_CPUS];   // MHz
#endif   // TRACE_CPU

#ifdef TRACE_MEM
    int32_t mem_power;            // mW
    int16_t emc_freq;             // MHz
    int32_t emc_util;             // x0.0001%
#endif   // TRACE_MEM

#ifdef TRACE_TEMP
    int32_t gpu_temp;             // x0.001 Celsius degree
#endif   // TRACE_TEMP
} powerlog_struct;


ssize_t timestamp_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd);

#ifdef TRACE_POWER
ssize_t boardpower_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd);
ssize_t socpower_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd);
ssize_t wifipower_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd);
#endif   // TRACE_POWER

ssize_t gpupower_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd);
ssize_t gpufreq_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd);
ssize_t gpuutil_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd);

#ifdef TRACE_CPU
ssize_t allcpu_power_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd);
#endif   // TRACE_CPU

#ifdef TRACE_MEM
ssize_t mempower_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd);
ssize_t emcfreq_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd);
ssize_t emcutil_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd);
#endif   // TRACE_MEM

#ifdef TRACE_TEMP
ssize_t gputemp_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd);
#endif   // TRACE_TEMP

#endif   // POWERLOG_H
