#include <stdio.h>
#include <unistd.h>

#include "tx2_sysfs_power.h"
#include "constants.h"
#include "log_to_stat.h"

// Timestamp to Statistics
ssize_t elapsedtime_to_stat(const int stat_fd, const int colwidth, const struct timespec timestamp, const struct timespec baseline) {

    // return value
    ssize_t num_written_bytes;
    time_t sec;
    int32_t nsec;
    char buff[MAX_COLWIDTH];
    int buff_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    sec  = timestamp.tv_sec  - baseline.tv_sec;
    nsec = timestamp.tv_nsec - baseline.tv_nsec;

    if(nsec < 0) {
        --sec;
        nsec += ONE_SECOND_TO_NANOSECOND;
    }

    buff_len = snprintf(buff, MAX_COLWIDTH, "%*ld.%09d", (colwidth - 1 - 9), sec, nsec);
    num_written_bytes = write(stat_fd, buff, buff_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}

ssize_t timestamp_to_stat(const int stat_fd, const int colwidth, const struct timespec timestamp) {

    ssize_t num_written_bytes;
    struct tm *calendar_timestamp;
    char buff1[MAX_COLWIDTH], buff2[MAX_COLWIDTH], buff3[MAX_COLWIDTH];
    int buff3_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    calendar_timestamp = localtime(&timestamp.tv_sec);
    strftime(buff1, MAX_COLWIDTH, "%H:%M:%S", calendar_timestamp);
    snprintf(buff2, MAX_COLWIDTH, "'%s.%09ld", buff1, timestamp.tv_nsec);
    buff3_len = snprintf(buff3, MAX_COLWIDTH, "%*s", colwidth, buff2);
    num_written_bytes = write(stat_fd, buff3, buff3_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    return num_written_bytes;
}

// Powerlog to Statistics
ssize_t system_power_to_stat(const int stat_fd, const int colwidth, const powerlog_struct powerlog) {

    // @system_power: mW
    ssize_t num_written_bytes;
    int32_t system_power;
    char buff[MAX_COLWIDTH];
    int buff_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    system_power = powerlog.allcpu_power + powerlog.gpu_power + powerlog.mem_power;
    buff_len = snprintf(buff, MAX_COLWIDTH, "%*d", colwidth, system_power);
    num_written_bytes = write(stat_fd, buff, buff_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}

#ifdef TRACE_POWER
ssize_t boardpower_to_stat(const int stat_fd, const int colwidth, const powerlog_struct powerlog) {

    // @powerlog.board_power: mW
    ssize_t num_written_bytes;
    char buff[MAX_COLWIDTH];
    int buff_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    buff_len = snprintf(buff, MAX_COLWIDTH, "%*d", colwidth,  powerlog.board_power);
    num_written_bytes = write(stat_fd, buff, buff_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}

ssize_t socpower_to_stat(const int stat_fd, const int colwidth, const powerlog_struct powerlog) {

    // @powerlog.soc_power: mW
    ssize_t num_written_bytes;
    char buff[MAX_COLWIDTH];
    int buff_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    buff_len = snprintf(buff, MAX_COLWIDTH, "%*d", colwidth,  powerlog.soc_power);
    num_written_bytes = write(stat_fd, buff, buff_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}

ssize_t wifipower_to_stat(const int stat_fd, const int colwidth, const powerlog_struct powerlog) {

    // @powerlog.wifi_power: mW
    ssize_t num_written_bytes;
    char buff[MAX_COLWIDTH];
    int buff_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    buff_len = snprintf(buff, MAX_COLWIDTH, "%*d", colwidth,  powerlog.wifi_power);
    num_written_bytes = write(stat_fd, buff, buff_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}
#endif   // TRACE_POWER


ssize_t gpupower_to_stat(const int stat_fd, const int colwidth, const powerlog_struct powerlog) {

    // @powerlog.gpu_power: mW
    ssize_t num_written_bytes;
    char buff[MAX_COLWIDTH];
    int buff_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    buff_len = snprintf(buff, MAX_COLWIDTH, "%*d", colwidth,  powerlog.gpu_power);
    num_written_bytes = write(stat_fd, buff, buff_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}

ssize_t gpufreq_to_stat(const int stat_fd, const int colwidth, const powerlog_struct powerlog) {

    // @powerlog.gpu_freq: MHz
    ssize_t num_written_bytes;
    char buff[MAX_COLWIDTH];
    int buff_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    buff_len = snprintf(buff, MAX_COLWIDTH, "%*d", colwidth, powerlog.gpu_freq);
    num_written_bytes = write(stat_fd, buff, buff_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}

ssize_t gpuutil_to_stat(const int stat_fd, const int colwidth, const powerlog_struct powerlog) {

    // @powerlog.gpu_util: x0.1%
    ssize_t num_written_bytes;
    char buff1[MAX_COLWIDTH], buff2[MAX_COLWIDTH];
    int buff2_len;
    int upper, lower;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    upper = powerlog.gpu_util / 10;
    lower = powerlog.gpu_util % 10;

    snprintf(buff1, MAX_COLWIDTH, "%3d.%1d", upper, lower);
    buff2_len = snprintf(buff2, MAX_COLWIDTH, "%*s", colwidth, buff1);
    num_written_bytes = write(stat_fd, buff2, buff2_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}

#ifdef TRACE_CPU
ssize_t allcpu_power_to_stat (const int stat_fd, const int colwidth, const powerlog_struct powerlog) {

    // @powerlog.allcpu_power: mW
    ssize_t num_written_bytes;
    char buff[MAX_COLWIDTH];
    int buff_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    buff_len = snprintf(buff, MAX_COLWIDTH, "%*d", colwidth,  powerlog.allcpu_power);
    num_written_bytes = write(stat_fd, buff, buff_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}
#endif   // TRACE_CPU

#ifdef TRACE_MEM
ssize_t mempower_to_stat (const int stat_fd, const int colwidth, const powerlog_struct powerlog) {

    // @powerlog.mem_power: mW
    ssize_t num_written_bytes;
    char buff[MAX_COLWIDTH];
    int buff_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    buff_len = snprintf(buff, MAX_COLWIDTH, "%*d", colwidth, powerlog.mem_power);
    num_written_bytes = write(stat_fd, buff, buff_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}

ssize_t emcfreq_to_stat  (const int stat_fd, const int colwidth, const powerlog_struct powerlog) {

    // @powerlog.emc_freq: MHz
    ssize_t num_written_bytes;
    char buff[MAX_COLWIDTH];
    int buff_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    buff_len = snprintf(buff, MAX_COLWIDTH, "%*d", colwidth, powerlog.emc_freq);
    num_written_bytes = write(stat_fd, buff, buff_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    return num_written_bytes;
}

ssize_t emcutil_to_stat  (const int stat_fd, const int colwidth, const powerlog_struct powerlog) {

    // @powerlog.emc_util: x0.0001%
    ssize_t num_written_bytes;
    char buff1[MAX_COLWIDTH], buff2[MAX_COLWIDTH];
    int buff2_len;
    int upper, lower;
    const int TO_PERCENT = 10000;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    upper = powerlog.emc_util / TO_PERCENT;
    lower = powerlog.emc_util % TO_PERCENT;

    snprintf(buff1, MAX_COLWIDTH, "%3d.%04d", upper, lower);
    buff2_len = snprintf(buff2, MAX_COLWIDTH, "%*s", colwidth, buff1);
    num_written_bytes = write(stat_fd, buff2, buff2_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}
#endif   // TRACE_MEM

#ifdef TRACE_TEMP
ssize_t gputemp_to_stat  (const int stat_fd, const int colwidth, const powerlog_struct powerlog) {

    // @powerlog.gpu_temp: x0.001 Celsius degree
    ssize_t num_written_bytes;
    char buff1[MAX_COLWIDTH], buff2[MAX_COLWIDTH];
    int buff2_len;
    int upper, lower;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    upper = powerlog.gpu_temp / 1000;
    lower = powerlog.gpu_temp % 1000;

    snprintf(buff1, MAX_COLWIDTH, "%d.%03d", upper, lower);
    buff2_len = snprintf(buff2, MAX_COLWIDTH, "%*s", colwidth, buff1);
    num_written_bytes = write(stat_fd, buff2, buff2_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}
#endif   // TRACE_TEMP

// Summary to Statistics
ssize_t system_energy_to_stat(const int stat_fd, const int colwidth, const summary_struct summary) {

    // return value
    ssize_t num_written_bytes;
    char buff1[MAX_COLWIDTH], buff2[MAX_COLWIDTH];
    int buff2_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    snprintf(buff1, MAX_COLWIDTH, "%ld.%03ld%03ld%06ld%01ld", summary.system_energy_J, summary.system_energy_mJ, summary.system_energy_uJ, summary.system_energy_pJ, (summary.system_energy_fJ / 100) );
    buff2_len = snprintf(buff2, MAX_COLWIDTH, "%*s", colwidth, buff1);
    num_written_bytes = write(stat_fd, buff2, buff2_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}

ssize_t psum_gpufreq_to_stat (const int stat_fd, const int colwidth, const summary_struct summary) {

    // Return value
    ssize_t num_written_bytes;
    char buff[MAX_COLWIDTH], buff2[MAX_COLWIDTH];
    size_t buff_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    snprintf(buff, MAX_COLWIDTH, "%ld.%09ld%", summary.psum_gpu_freq_sec, summary.psum_gpu_freq_ns);
    buff_len = snprintf(buff2, MAX_COLWIDTH, "%*s", colwidth, buff);
    num_written_bytes = write(stat_fd, buff2, buff_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    return num_written_bytes;
}

ssize_t gpuenergy_to_stat(const int stat_fd, const int colwidth, const summary_struct summary) {

    // return value
    ssize_t num_written_bytes;
    char buff1[MAX_COLWIDTH], buff2[MAX_COLWIDTH];
    int buff2_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    snprintf(buff1, MAX_COLWIDTH, "%ld.%03ld%03ld%06ld%01ld", summary.gpu_energy_J, summary.gpu_energy_mJ, summary.gpu_energy_uJ, summary.gpu_energy_pJ, (summary.gpu_energy_fJ / 100) );
    buff2_len = snprintf(buff2, MAX_COLWIDTH, "%*s", colwidth, buff1);
    num_written_bytes = write(stat_fd, buff2, buff2_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}

// Prints product-sum of GPU utilization-time. Unit: sec * %
ssize_t psum_gpuutil_to_stat (const int stat_fd, const int colwidth, const summary_struct summary) {

    // Return value
    ssize_t num_written_bytes;
    char buff[MAX_COLWIDTH];
    size_t buff_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    if(colwidth <= 10) {
        perror("The column width for product-sum of GPU utilization-time should be larger than 10");
        return -1;
    }

    buff_len = snprintf(buff, MAX_COLWIDTH, "%*ld.%01ld%08ld%01ld", (colwidth-10), summary.psum_gpu_util_e2ms / 10, summary.psum_gpu_util_e2ms % 10, summary.psum_gpu_util_e2ps / 10, summary.psum_gpu_util_e2ps % 10);
    num_written_bytes = write(stat_fd, buff, buff_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}

ssize_t avg_gpuutil_to_stat (const int stat_fd, const int colwidth, const summary_struct summary) {

    // Return value
    ssize_t num_written_bytes;

    // Product-sum of time-utilization
    double psum;

    // Elapsed time
    time_t sec_int;
    int32_t ns_int;
    double elapsed_time;    // ns

    // Buffer
    char buff[MAX_COLWIDTH];
    size_t buff_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    // Calculate and convert elapsed time in ns
    sec_int = summary.finish_timestamp.tv_sec  - summary.start_timestamp.tv_sec;
    ns_int  = summary.finish_timestamp.tv_nsec - summary.start_timestamp.tv_nsec;
    elapsed_time = sec_int * 1e9 + ns_int;

    if(elapsed_time == 0) {

        buff_len = snprintf(buff, MAX_COLWIDTH, "%*s", colwidth, "#N/A");
        goto write_avg_gpu_util;
    }

    // Calculate and convert product-sum of utilization-time.
    psum = summary.psum_gpu_util_e2ms * 1e8 + summary.psum_gpu_util_e2ps * 1e-1;

    buff_len = snprintf(buff, MAX_COLWIDTH, "%lf", psum / elapsed_time);
write_avg_gpu_util:
    num_written_bytes = write(stat_fd, buff, buff_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}

ssize_t avg_emcutil_to_stat (const int stat_fd, const int colwidth, const summary_struct summary) {

    // Return value
    ssize_t num_written_bytes;

    // Product-sum of time-utilization
    double psum;

    // Elapsed time
    time_t sec_int;
    int32_t ns_int;
    double elapsed_time;    // ns

    // Buffer
    char buff[MAX_COLWIDTH];
    size_t buff_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    // Calculate and convert elapsed time in ns
    sec_int = summary.finish_timestamp.tv_sec  - summary.start_timestamp.tv_sec;
    ns_int  = summary.finish_timestamp.tv_nsec - summary.start_timestamp.tv_nsec;
    elapsed_time = sec_int * 1e9 + ns_int;

    if(elapsed_time == 0) {

        buff_len = snprintf(buff, MAX_COLWIDTH, "%*s", colwidth, "#N/A");
        goto write_avg_emc_util;
    }

    // Calculate and convert product-sum of utilization-time.
    psum = summary.psum_emc_util_e2us * 1e5 + summary.psum_emc_util_e2fs * 1e-4;

    buff_len = snprintf(buff, MAX_COLWIDTH, "%lf", psum / elapsed_time);
write_avg_emc_util:
    num_written_bytes = write(stat_fd, buff, buff_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}


ssize_t boardenergy_to_stat  (const int stat_fd, const int colwidth, const summary_struct summary) {

    // return value
    ssize_t num_written_bytes;
    char buff1[MAX_COLWIDTH], buff2[MAX_COLWIDTH];
    int buff2_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    //  J: xxxx.
    // mJ:      xxx
    // uJ:         xxx
    // pJ:            xxxxxx
    // fJ:                  x
    snprintf(buff1, MAX_COLWIDTH, "%ld.%03ld%03ld%06ld%01ld", summary.board_energy_J, summary.board_energy_mJ, summary.board_energy_uJ, summary.board_energy_pJ, (summary.board_energy_fJ / 100) );
    buff2_len = snprintf(buff2, MAX_COLWIDTH, "%*s", colwidth, buff1);
    num_written_bytes = write(stat_fd, buff2, buff2_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;

}

ssize_t memenergy_to_stat  (const int stat_fd, const int colwidth, const summary_struct summary) {

    // return value
    ssize_t num_written_bytes;
    char buff1[MAX_COLWIDTH], buff2[MAX_COLWIDTH];
    int buff2_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    //  J: xxxx.
    // mJ:      xxx
    // uJ:         xxx
    // pJ:            xxxxxx
    // fJ:                  x
    snprintf(buff1, MAX_COLWIDTH, "%ld.%03ld%03ld%06ld%01ld", summary.mem_energy_J, summary.mem_energy_mJ, summary.mem_energy_uJ, summary.mem_energy_pJ, (summary.mem_energy_fJ / 100) );
    buff2_len = snprintf(buff2, MAX_COLWIDTH, "%*s", colwidth, buff1);
    num_written_bytes = write(stat_fd, buff2, buff2_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}


// Caffelog to Statistics
ssize_t caffeevent_to_stat(const int stat_fd, const int colwidth, const caffelog_struct caffelog) {

    // return value
    ssize_t num_written_bytes;
    char buff[MAX_COLWIDTH];
    int buff_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    buff_len = snprintf(buff, MAX_COLWIDTH, "%*s", colwidth, caffelog.event);
    num_written_bytes = write(stat_fd, buff, buff_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}

ssize_t caffe_start_to_stat(const int stat_fd, const int colwidth, const caffelog_struct caffelog) {

    // return value
    ssize_t num_written_bytes;
    char buff[MAX_COLWIDTH];
    int buff_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    if(caffelog.caffe_start > 0) {
        buff_len = snprintf(buff, MAX_COLWIDTH, "%*d", colwidth, caffelog.caffe_start);
        num_written_bytes = write(stat_fd, buff, buff_len);
    }
    else {
        num_written_bytes = write(stat_fd, WHITESPACE, colwidth-4);
        num_written_bytes = write(stat_fd, "#N/A", 4);
    }

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}

ssize_t cnn_event_to_stat(const int stat_fd, const int colwidth, const caffelog_struct caffelog) {

    // return value
    ssize_t num_written_bytes;
    char buff[MAX_COLWIDTH];
    int buff_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    if(caffelog.cnn_start > 0) {
        buff_len = snprintf(buff, MAX_COLWIDTH, "%*d", colwidth, caffelog.cnn_start);
        num_written_bytes = write(stat_fd, buff, buff_len);
    }
    else if(caffelog.cnn_finish > 0) {
        buff_len = snprintf(buff, MAX_COLWIDTH, "%*d", colwidth, caffelog.cnn_finish);
        num_written_bytes = write(stat_fd, buff, buff_len);
    }
    else {
        num_written_bytes = write(stat_fd, WHITESPACE, colwidth-4);
        num_written_bytes = write(stat_fd, "#N/A", 4);
    }

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;

}

ssize_t batch_idx_to_stat(const int stat_fd, const int colwidth, const caffelog_struct caffelog) {

    // return value
    ssize_t num_written_bytes;
    char buff[MAX_COLWIDTH];
    int buff_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    if(caffelog.batch_idx <= 0) {
        num_written_bytes = write(stat_fd, WHITESPACE, colwidth-4);
        num_written_bytes = write(stat_fd, "#N/A", 4);
    }
    else {
        buff_len = snprintf(buff, MAX_COLWIDTH, "%*d", colwidth, caffelog.batch_idx);
        num_written_bytes = write(stat_fd, buff, buff_len);
    }

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}


ssize_t batch_finish_to_stat(const int stat_fd, const int colwidth, const caffelog_struct caffelog) {

    // return value
    ssize_t num_written_bytes;
    char buff[MAX_COLWIDTH];
    int buff_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    if(!caffelog.batch_finish) {
        num_written_bytes = write(stat_fd, WHITESPACE, colwidth-4);
        num_written_bytes = write(stat_fd, "#N/A", 4);
    }
    else {
        buff_len = snprintf(buff, MAX_COLWIDTH, "%*d", colwidth, caffelog.batch_finish);
        num_written_bytes = write(stat_fd, buff, buff_len);
    }

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
    if(num_written_bytes < 0)
        perror("Error while write()");
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}
