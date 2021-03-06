#ifndef MEASUREMENT_INFO_H
#define MEASUREMENT_INFO_H

#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <stdarg.h>
#include "rawdata.h"
#include "stat.h"
#include "default_values.h"
#include "list.h"
struct powerlog_struct;
struct tegralog_struct;
//struct stat_info_struct;
struct measurement_info_struct;

/*
 *  Tegralog
 */
//  REGEX
//  regmatch[0] is the whole match. Do not care about it
//  regmatch[1]:   CPU0 utilization
//  regmatch[2]:   CPU0 frequency
//  regmatch[3]:   CPU1 status: utilization and frequency
//  regmatch[4]:   CPU2 status: utilization and frequency
//  regmatch[5]:   CPU3 status: utilization and frequency
//  regmatch[6]:   CPU4 status: utilization and frequency
//  regmatch[7]:   CPU5 status: utilization and frequency
//  regmatch[8]:   EMC UTIL
//  regmatch[9]:   EMC FREQ
//  regmatch[10]:  GPU UTIL
//  regmatch[11]:  GPU FREQ
//  regmatch[12]:  GPU Temparature
//  regmatch[13]:  CPU Power
//  regmatch[14]:  GPU Power
//  regmatch[15]:  DDR Power
#define TEGRALOG_PATTERN                                                   \
    "RAM[[:space:]]+[[:digit:]]+/[[:digit:]]+MB[[:space:]]+"               \
    "[(]lfb[[:space:]]+[[:digit:]]+x[[:digit:]]+MB[)][[:space:]]+"         \
    "CPU[[:space:]]+[\[]([[:digit:]]+)%@([[:digit:]]+),"                   \
    "([[:digit:]]+%@[[:digit:]]+|off),"                                    \
    "([[:digit:]]+%@[[:digit:]]+|off),"                                    \
    "([[:digit:]]+%@[[:digit:]]+|off),"                                    \
    "([[:digit:]]+%@[[:digit:]]+|off),"                                    \
    "([[:digit:]]+%@[[:digit:]]+|off)[\]][[:space:]]+"                     \
    "EMC_FREQ[[:space:]]+([[:digit:]]+)%@([[:digit:]]+)[[:space:]]+"       \
    "GR3D_FREQ[[:space:]]+([[:digit:]]+)%@([[:digit:]]+)[[:space:]]+"      \
    "APE[[:space:]]+[[:digit:]]+[[:space:]]+"                              \
    "BCPU@[.[:digit:]]+C[[:space:]]+"                                      \
    "MCPU@[.[:digit:]]+C[[:space:]]+"                                      \
    "GPU@([.[:digit:]]+)+C[[:space:]]+"                                    \
    "PLL@[.[:digit:]]+C[[:space:]]+"                                       \
    "Tboard@[.[:digit:]]+C[[:space:]]+"                                    \
    "Tdiode@[.[:digit:]]+C[[:space:]]+"                                    \
    "PMIC@[.[:digit:]]+C[[:space:]]+"                                      \
    "thermal@[.[:digit:]]+C[[:space:]]+"                                   \
    "VDD_IN[[:space:]]+[[:digit:]]+/[[:digit:]]+[[:space:]]+"              \
    "VDD_CPU[[:space:]]+([[:digit:]]+)/[[:digit:]]+[[:space:]]+"           \
    "VDD_GPU[[:space:]]+([[:digit:]]+)/[[:digit:]]+[[:space:]]+"           \
    "VDD_SOC[[:space:]]+[[:digit:]]+/[[:digit:]]+[[:space:]]+"             \
    "VDD_WIFI[[:space:]]+[[:digit:]]+/[[:digit:]]+[[:space:]]+"            \
    "VDD_DDR[[:space:]]+([[:digit:]]+)/[[:digit:]]+"                       \
    "[[:space:]]*"

// Tegralog Example:
// RAM 1911/7854MB (lfb 224x4MB) CPU [0%@345,off,off,0%@345,0%@345,0%@345] EMC_FREQ 0%@665 GR3D_FREQ 0%@140 APE 150 BCPU@32C MCPU@32C GPU@31.5C PLL@32C Tboard@28C Tdiode@28.75C PMIC@100C thermal@31.8C VDD_IN 1182/1182 VDD_CPU 152/152 VDD_GPU 152/152 VDD_SOC 381/381 VDD_WIFI 0/0 VDD_DDR 192/192
typedef struct tegralog_struct {
    // Time (ns)
    // Note that the 'tegrastats' do NOT give any time informations
    int64_t time;

    // Utilizations (%) & Frequencies (MHz)
    int cpu_util[NUM_CPUS], cpu_freq[NUM_CPUS];
    int gpu_util;
    int gpu_freq;
    int emc_util;
    int emc_freq;
    // int ape_freq;

    // Power Measurement I (mW)
    int gpu_cur_power, gpu_avg_power;
    int soc_cur_power, soc_avg_power;
    int wifi_cur_power, wifi_avg_power;

    // Power Measurement II (mW)
    int ddr_cur_power, ddr_avg_power;
    int all_cur_power, all_avg_power;
    int cpu_cur_power, cpu_avg_power;

    // Thermal Informations (degree Celsius)
    //int bcpu_thinfo;   // BCPU Cluster: 4x ARM Cortex A57 CPUs
    //int mcpu_thinfo;   // MCPU Cluster: 2x NVidia Denver2 CPUs
    int gpu_thinfo;
    //int pll_thinfo;
    //int tboard;
    //int tdiode;
    //int pmic_thinfo;
    //int thermal_thinfo;

    // RAM Informations
    //int ram_use;
    //int ram_avail;
    //int ram_lfb;

} tegralog_struct;


/*
 *  Measurement Information
 */
typedef struct measurement_info_struct {

    // Caffe sleep request: in order to cool down CPUs
    struct timespec caffe_sleep_request;

    // Measurement interval in nanosecond
    struct timespec powertool_interval;

    // Cooldown period: in order to cool down GPU, etc.
    struct timespec cooldown_period;

    // Result Directory
    int flag_mkdir;
    char result_dirname[MAX_BUFFLEN];

    // GPU Governor
    int userspace_gpugovernor;
    char gpugov_name[GPU_GOVERNOR_NAME_LEN];

    // Powerlog
    char powerlog_filename[MAX_BUFFLEN];
    int powerlog_fd;

    //  I will use execve() to run child command.
    //  Therefore, the child_cmd is an NULL-terminated array of arguments.
    //  Child will redirect its stderr message to caffelog file;
    //  thus, we store caffelog file name and fd
    char **child_cmd;

    // Caffelog
    char caffelog_filename[MAX_BUFFLEN];
    int caffelog_fd;

    // Informations used by measure_rawdata()
    struct tm calendar_start_time;   // GMT
    struct timespec    start_time;   // GMT

    //  In order to use sysfs interface easily.
    //  See addsysfs(), read_sysfs(), rawdata_to_stat()
    struct rawdata_info_struct rawdata_info[MAX_NUM_RAWDATA];
    int num_rawdata;
    char rawdata_filename[MAX_BUFFLEN];
    int rawdata_fd;
    int rawdata_linesize;

    // Rows
    struct row_info_struct row[MAX_NUM_ROW];
    int num_row;
    off_t  summary_start;
    size_t summary_len;
    off_t metadata_end;

    // Statistics and summary
    struct stat_info_struct stat_info[MAX_NUM_STAT];
    int num_stat;
    char stat_filename[MAX_BUFFLEN], summary_filename[MAX_BUFFLEN];

} measurement_info_struct;


#ifdef DEBUG
void print_info(const measurement_info_struct info);
void print_rawdata_info(const rawdata_info_struct rawdata_info);
#endif   // DEBUG

void init_info(measurement_info_struct *info);

void close_sysfs_files(struct measurement_info_struct info);

#endif   // MEASUREMENT_INFO_H
