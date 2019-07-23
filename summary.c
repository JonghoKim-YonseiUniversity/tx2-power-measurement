#include <stdio.h>
#include <stdint.h>
#include "default_values.h"
#include "constants.h"
#include "summary.h"

void init_summary(summary_struct *summary) {

#ifdef TRACE_CPU
    int i;
#endif   // TRACE_CPU

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->num_powerlog = 0;

    // Maximum values are initialized to INIT_MAX
    summary->max_gpu_util        = INIT_MAX;
    summary->max_gpu_freq        = INIT_MAX;
    summary->max_gpu_power       = INIT_MAX;
    summary->max_all_power       = INIT_MAX;

#ifdef TRACE_CPU
    for(i=0; i<NUM_CPUS; ++i) {
        summary->max_cpu_util[i] = INIT_MAX;
        summary->max_cpu_freq[i] = INIT_MAX;
    }
    summary->max_allcpu_power    = INIT_MAX;
#endif   // TRACE_CPU

#ifdef TRACE_MEM
    summary->max_emc_util        = INIT_MAX;
    summary->max_emc_freq        = INIT_MAX;
    summary->max_mem_power       = INIT_MAX;
#endif   // TRACE_MEM

    // Minimum values are initialized to INIT_MIN
    summary->min_gpu_util        = INIT_MIN;
    summary->min_gpu_freq        = INIT_MIN;
    summary->min_gpu_power       = INIT_MIN;
    summary->min_all_power       = INIT_MIN;

#ifdef TRACE_CPU
    for(i=0; i<NUM_CPUS; ++i) {
        summary->min_cpu_util[i] = INIT_MIN;
        summary->min_cpu_freq[i] = INIT_MIN;
    }
    summary->min_allcpu_power    = INIT_MIN;
#endif   // TRACE_CPU

#ifdef TRACE_MEM
    summary->min_emc_util        = INIT_MIN;
    summary->min_emc_freq        = INIT_MIN;
    summary->min_mem_power       = INIT_MIN;
#endif   // TRACE_MEM

    // Summation values are initialized to INIT_SUM
    summary->psum_gpu_util_sec      = INIT_SUM;
    summary->psum_gpu_util_ns       = INIT_SUM;
    summary->gpu_energy_J           = INIT_SUM;
    summary->gpu_energy_uJ          = INIT_SUM;
    summary->gpu_energy_pJ          = INIT_SUM;
    summary->gpu_energy_dotone_pJ   = INIT_SUM;
    summary->all_energy_J           = INIT_SUM;
    summary->all_energy_mJ          = INIT_SUM;
    summary->all_energy_uJ          = INIT_SUM;
    summary->all_energy_pJ          = INIT_SUM;
    summary->all_energy_fJ          = INIT_SUM;

#ifdef TRACE_CPU
    summary->allcpu_energy_J        = INIT_SUM;
    summary->allcpu_energy_pJ       = INIT_SUM;
#endif   // TRACE_CPU

#ifdef TRACE_MEM
    summary->psum_emc_util_ms       = INIT_SUM;
    summary->psum_emc_util_fs       = INIT_SUM;
    summary->psum_emc_freq_sec      = INIT_SUM;
    summary->psum_emc_freq_ns       = INIT_SUM;
    summary->mem_energy_J           = INIT_SUM;
    summary->mem_energy_mJ          = INIT_SUM;
    summary->mem_energy_uJ          = INIT_SUM;
    summary->mem_energy_pJ          = INIT_SUM;
    summary->mem_energy_fJ          = INIT_SUM;
#endif   // TRACE_MEM

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   FINISH", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_SUMMARY
    return;
}

static void update_gpuenergy(summary_struct *summary, const powerlog_struct *powerlog_ptr) {

    int64_t sec, ms, ns;
    int64_t avg_gpupower_mW, avg_gpupower_dotone_mW;
    int64_t fraction, remainder;

    // Calculate average power
    avg_gpupower_mW = (powerlog_ptr->gpu_power + summary->last_powerlog.gpu_power) / 2;
    avg_gpupower_dotone_mW = ((powerlog_ptr->gpu_power + summary->last_powerlog.gpu_power) % 2) * 5;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   given GPU power: %d (mW)", __func__, __FILE__, __LINE__, powerlog_ptr->gpu_power);
    printf("\n%s() in %s:%d   last GPU power: %d (mW)", __func__, __FILE__, __LINE__, summary->last_powerlog.gpu_power);
    printf("\n%s() in %s:%d   avg.GPU power: %d.%d (mW)", __func__, __FILE__, __LINE__, avg_gpupower_mW, avg_gpupower_dotone_mW);
#endif   // DEBUG or DEBUG_SUMMARY

    // Calculate elapsed time in: ms, ns
    sec = powerlog_ptr->timestamp.tv_sec  - summary->finish_timestamp.tv_sec;
    ns  = powerlog_ptr->timestamp.tv_nsec - summary->finish_timestamp.tv_nsec;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   diff sec:  %d", __func__, __FILE__, __LINE__, sec);
    printf("\n%s() in %s:%d   diff nsec: %d", __func__, __FILE__, __LINE__, ns);
#endif   // DEBUG or DEBUG_SUMMARY

    // Note that negative numbers are also OK
    fraction  = ns / MILLI_PER_NANO;
    remainder = ns % MILLI_PER_NANO;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction:  %d", __func__, __FILE__, __LINE__, fraction);
    printf("\n%s() in %s:%d   remainder: %d", __func__, __FILE__, __LINE__, remainder);
#endif   // DEBUG or DEBUG_SUMMARY

    ms = sec * ONE_PER_MILLI + fraction;
    ns = remainder;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   elapsed ms: %d", __func__, __FILE__, __LINE__, ms);
    printf("\n%s() in %s:%d   elapsed ns: %d", __func__, __FILE__, __LINE__, ns);
#endif   // DEBUG or DEBUG_SUMMARY

    // Sum the calculated energy
    summary->gpu_energy_uJ += ms * avg_gpupower_mW;
    summary->gpu_energy_pJ += ns * avg_gpupower_mW;
    summary->gpu_energy_dotone_pJ += ms * avg_gpupower_dotone_mW * MICRO_PER_PICO;
    summary->gpu_energy_dotone_pJ += ns * avg_gpupower_dotone_mW;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   GPU energy before removing remainder ---|", __func__, __FILE__, __LINE__);
    printf("\n%s() in %s:%d   GPU energy  J:     %ld", __func__, __FILE__, __LINE__, summary->gpu_energy_J);
    printf("\n%s() in %s:%d   GPU energy uJ:     %ld", __func__, __FILE__, __LINE__, summary->gpu_energy_uJ);
    printf("\n%s() in %s:%d   GPU energy pJ:     %ld.%ld", __func__, __FILE__, __LINE__, summary->gpu_energy_pJ, summary->gpu_energy_dotone_pJ);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of 0.1 pJ
    fraction  = summary->gpu_energy_dotone_pJ / 10;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction:  %d", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->gpu_energy_pJ         += fraction;
    summary->gpu_energy_dotone_pJ  -= (fraction * 10);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   GPU energy after removing 0.1pJ remainder ---|", __func__, __FILE__, __LINE__);
    printf("\n%s() in %s:%d   GPU energy  J:     %ld", __func__, __FILE__, __LINE__, summary->gpu_energy_J);
    printf("\n%s() in %s:%d   GPU energy uJ:     %ld", __func__, __FILE__, __LINE__, summary->gpu_energy_uJ);
    printf("\n%s() in %s:%d   GPU energy pJ:     %ld.%ld", __func__, __FILE__, __LINE__, summary->gpu_energy_pJ, summary->gpu_energy_dotone_pJ);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of pJ
    fraction  = summary->gpu_energy_pJ / MICRO_PER_PICO;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction:  %d", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->gpu_energy_uJ += fraction;
    summary->gpu_energy_pJ -= (fraction * MICRO_PER_PICO);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   GPU energy after removing pJ remainder ---|", __func__, __FILE__, __LINE__);
    printf("\n%s() in %s:%d   GPU energy  J:     %ld", __func__, __FILE__, __LINE__, summary->gpu_energy_J);
    printf("\n%s() in %s:%d   GPU energy uJ:     %ld", __func__, __FILE__, __LINE__, summary->gpu_energy_uJ);
    printf("\n%s() in %s:%d   GPU energy pJ:     %ld.%ld", __func__, __FILE__, __LINE__, summary->gpu_energy_pJ, summary->gpu_energy_dotone_pJ);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of uJ
    fraction  = summary->gpu_energy_uJ / ONE_PER_MICRO;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction:  %d", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->gpu_energy_J  += fraction;
    summary->gpu_energy_uJ -= (fraction * ONE_PER_MICRO);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   GPU energy after removing uJ remainder ---|", __func__, __FILE__, __LINE__);
    printf("\n%s() in %s:%d   GPU energy  J:     %ld", __func__, __FILE__, __LINE__, summary->gpu_energy_J);
    printf("\n%s() in %s:%d   GPU energy uJ:     %ld", __func__, __FILE__, __LINE__, summary->gpu_energy_uJ);
    printf("\n%s() in %s:%d   GPU energy pJ:     %ld.%ld", __func__, __FILE__, __LINE__, summary->gpu_energy_pJ, summary->gpu_energy_dotone_pJ);
#endif   // DEBUG or DEBUG_SUMMARY

    return;
}

static void update_psum_gpuutil(summary_struct *summary, const powerlog_struct *powerlog_ptr) {

    int64_t sec, ns;
    int64_t fraction;

    // Calculate elapsed time in: sec, ns
    sec = powerlog_ptr->timestamp.tv_sec  - summary->finish_timestamp.tv_sec;
    ns  = powerlog_ptr->timestamp.tv_nsec - summary->finish_timestamp.tv_nsec;

    if(ns < 0) {
        --sec;
        ns += ONE_PER_NANO;
    }

    summary->psum_gpu_util_sec += (powerlog_ptr->gpu_util * sec);
    summary->psum_gpu_util_ns  += (powerlog_ptr->gpu_util * ns);

    fraction = summary->psum_gpu_util_ns / ONE_PER_NANO;
    if(fraction > 0) {
        summary->psum_gpu_util_sec += fraction;
        summary->psum_gpu_util_ns -= (fraction * ONE_PER_NANO);
    }

    return;
}

static inline void print_allenergy(summary_struct summary) {

    printf("\n%s() in %s:%d   ALL energy  J:     %ld", __func__, __FILE__, __LINE__, summary.all_energy_J);
    printf("\n%s() in %s:%d   ALL energy mJ:     %ld", __func__, __FILE__, __LINE__, summary.all_energy_mJ);
    printf("\n%s() in %s:%d   ALL energy uJ:     %ld", __func__, __FILE__, __LINE__, summary.all_energy_uJ);
    printf("\n%s() in %s:%d   ALL energy pJ:     %ld", __func__, __FILE__, __LINE__, summary.all_energy_pJ);
    printf("\n%s() in %s:%d   ALL energy fJ:     %ld", __func__, __FILE__, __LINE__, summary.all_energy_fJ);

    return;
}

static void update_allenergy(summary_struct *summary, const powerlog_struct *powerlog_ptr) {

    int64_t sec, ns;
    int64_t avg_allpower_mW, avg_allpower_uW;
    int64_t fraction;

    // Calculate average power
    avg_allpower_mW = (powerlog_ptr->all_power + summary->last_powerlog.all_power) / 2;
    avg_allpower_uW = (((powerlog_ptr->all_power + summary->last_powerlog.all_power) % 2) * MILLI_PER_MICRO) / 2;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   given ALL power: %d (mW)", __func__, __FILE__, __LINE__, powerlog_ptr->all_power);
    printf("\n%s() in %s:%d   last ALL power: %d (mW)", __func__, __FILE__, __LINE__, summary->last_powerlog.all_power);
    printf("\n%s() in %s:%d   avg.ALL power: %d.%d (mW)", __func__, __FILE__, __LINE__, avg_allpower_mW, avg_allpower_uW);
#endif   // DEBUG or DEBUG_SUMMARY

    // Calculate elapsed time in: sec, ns
    sec = powerlog_ptr->timestamp.tv_sec  - summary->finish_timestamp.tv_sec;
    ns  = powerlog_ptr->timestamp.tv_nsec - summary->finish_timestamp.tv_nsec;
    if(ns < 0) {
        --sec;
        ns += ONE_PER_NANO;
    }

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   diff sec:  %d", __func__, __FILE__, __LINE__, sec);
    printf("\n%s() in %s:%d   diff nsec: %d", __func__, __FILE__, __LINE__, ns);
#endif   // DEBUG or DEBUG_SUMMARY

    // Sum the calculated energy
    summary->all_energy_mJ += sec * avg_allpower_mW;
    summary->all_energy_uJ += sec * avg_allpower_uW;
    summary->all_energy_pJ += ns  * avg_allpower_mW;
    summary->all_energy_fJ += ns  * avg_allpower_uW;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   ALL energy before removing remainder ---|", __func__, __FILE__, __LINE__);
    print_allenergy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of fJ
    fraction  = summary->all_energy_fJ / PICO_PER_FEMTO;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction:  %d", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->all_energy_pJ   += fraction;
    summary->all_energy_fJ   -= (fraction * PICO_PER_FEMTO);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   ALL energy after removing fJ remainder ---|", __func__, __FILE__, __LINE__);
    print_allenergy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of pJ
    fraction  = summary->all_energy_pJ / MICRO_PER_PICO;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction:  %d", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->all_energy_uJ += fraction;
    summary->all_energy_pJ -= (fraction * MICRO_PER_PICO);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   ALL energy after removing pJ remainder ---|", __func__, __FILE__, __LINE__);
    print_allenergy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of uJ
    fraction  = summary->all_energy_uJ / MILLI_PER_MICRO;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction:  %d", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->all_energy_mJ += fraction;
    summary->all_energy_uJ -= (fraction * MILLI_PER_MICRO);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   ALL energy after removing uJ remainder ---|", __func__, __FILE__, __LINE__);
    print_allenergy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of mJ
    fraction  = summary->all_energy_mJ / ONE_PER_MILLI;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction:  %d", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->all_energy_J  += fraction;
    summary->all_energy_mJ -= (fraction * ONE_PER_MILLI);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   ALL energy after removing mJ remainder ---|", __func__, __FILE__, __LINE__);
    print_allenergy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    return;
}

#ifdef TRACE_MEM
static inline void print_memenergy(summary_struct summary) {

    printf("\n%s() in %s:%d   MEM energy  J:     %d", __func__, __FILE__, __LINE__, summary.mem_energy_J);
    printf("\n%s() in %s:%d   MEM energy mJ:     %d", __func__, __FILE__, __LINE__, summary.mem_energy_mJ);
    printf("\n%s() in %s:%d   MEM energy uJ:     %d", __func__, __FILE__, __LINE__, summary.mem_energy_uJ);
    printf("\n%s() in %s:%d   MEM energy pJ:     %ld", __func__, __FILE__, __LINE__, summary.mem_energy_pJ);
    printf("\n%s() in %s:%d   MEM energy fJ:     %ld", __func__, __FILE__, __LINE__, summary.mem_energy_fJ);

    return;
}

static void update_memenergy(summary_struct *summary, const powerlog_struct *powerlog_ptr) {

    int64_t sec, ns;
    int64_t avg_mempower_mW, avg_mempower_uW;
    int64_t fraction;

    // Calculate average power
    avg_mempower_mW = (powerlog_ptr->mem_power + summary->last_powerlog.mem_power) / 2;
    avg_mempower_uW = (((powerlog_ptr->mem_power + summary->last_powerlog.mem_power) % 2) * MILLI_PER_MICRO) / 2;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   given MEM power: %d (mW)", __func__, __FILE__, __LINE__, powerlog_ptr->mem_power);
    printf("\n%s() in %s:%d   last MEM power: %d (mW)", __func__, __FILE__, __LINE__, summary->last_powerlog.mem_power);
    printf("\n%s() in %s:%d   avg.MEM power: %d.%d (mW)", __func__, __FILE__, __LINE__, avg_mempower_mW, avg_mempower_uW);
#endif   // DEBUG or DEBUG_SUMMARY

    // Calculate elapsed time in: sec, ns
    sec = powerlog_ptr->timestamp.tv_sec  - summary->finish_timestamp.tv_sec;
    ns  = powerlog_ptr->timestamp.tv_nsec - summary->finish_timestamp.tv_nsec;
    if(ns < 0) {
        --sec;
        ns += ONE_PER_NANO;
    }

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   diff sec:  %d", __func__, __FILE__, __LINE__, sec);
    printf("\n%s() in %s:%d   diff nsec: %d", __func__, __FILE__, __LINE__, ns);
#endif   // DEBUG or DEBUG_SUMMARY

    // Sum the calculated energy
    summary->mem_energy_mJ += sec * avg_mempower_mW;
    summary->mem_energy_uJ += sec * avg_mempower_uW;
    summary->mem_energy_pJ += ns  * avg_mempower_mW;
    summary->mem_energy_fJ += ns  * avg_mempower_uW;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   MEM energy before removing remainder ---|", __func__, __FILE__, __LINE__);
    print_memenergy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of fJ
    fraction  = summary->mem_energy_fJ / PICO_PER_FEMTO;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction:  %d", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->mem_energy_pJ   += fraction;
    summary->mem_energy_fJ   -= (fraction * PICO_PER_FEMTO);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   MEM energy after removing fJ remainder ---|", __func__, __FILE__, __LINE__);
    print_memenergy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of pJ
    fraction  = summary->mem_energy_pJ / MICRO_PER_PICO;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction:  %d", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->mem_energy_uJ += fraction;
    summary->mem_energy_pJ -= (fraction * MICRO_PER_PICO);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   MEM energy after removing pJ remainder ---|", __func__, __FILE__, __LINE__);
    print_memenergy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of uJ
    fraction  = summary->mem_energy_uJ / MILLI_PER_MICRO;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction:  %d", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->mem_energy_mJ += fraction;
    summary->mem_energy_uJ -= (fraction * MILLI_PER_MICRO);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   MEM energy after removing uJ remainder ---|", __func__, __FILE__, __LINE__);
    print_memenergy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of mJ
    fraction  = summary->mem_energy_mJ / ONE_PER_MILLI;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction:  %d", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->mem_energy_J  += fraction;
    summary->mem_energy_mJ -= (fraction * ONE_PER_MILLI);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   MEM energy after removing mJ remainder ---|", __func__, __FILE__, __LINE__);
    print_memenergy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    return;
}
#endif   // TRACE_MEM

void update_summary(summary_struct *summary, const powerlog_struct *powerlog_ptr) {

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_SUMMARY

    if(!summary->num_powerlog) {
        summary->start_timestamp = powerlog_ptr->timestamp;
        summary->finish_timestamp = powerlog_ptr->timestamp;
        summary->last_powerlog = *powerlog_ptr;
    }

    // Update maximum values
    if(summary->max_gpu_util < powerlog_ptr->gpu_util)
        summary->max_gpu_util = powerlog_ptr->gpu_util;
    if(summary->max_gpu_freq < powerlog_ptr->gpu_freq)
        summary->max_gpu_freq = powerlog_ptr->gpu_freq;
    if(summary->max_gpu_power < powerlog_ptr->gpu_power)
        summary->max_gpu_power = powerlog_ptr->gpu_power;

    // Update minimum values.
    // Note that we initialized minimum values to maximum possitive number.
    // Thus, correct values are always less than initialized minimum values
    if(summary->min_gpu_util > powerlog_ptr->gpu_util)
        summary->min_gpu_util = powerlog_ptr->gpu_util;
    if(summary->min_gpu_freq > powerlog_ptr->gpu_freq)
        summary->min_gpu_freq = powerlog_ptr->gpu_freq;
    if(summary->min_gpu_power > powerlog_ptr->gpu_power)
        summary->min_gpu_power = powerlog_ptr->gpu_power;

    update_gpuenergy(summary, powerlog_ptr);
    update_psum_gpuutil(summary, powerlog_ptr);
    update_allenergy(summary, powerlog_ptr);
#ifdef TRACE_MEM
    update_memenergy(summary, powerlog_ptr);
#endif   // TRACE_MEM

    // Count number of powerlogs
    ++(summary->num_powerlog);

    // Store timestamp
    summary->finish_timestamp = powerlog_ptr->timestamp;

    // AT LAST, Copy the contents of the last powerlog
    summary->last_powerlog = *powerlog_ptr;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   FINISH", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_SUMMARY
    return;
}

struct timespec elapsed_time(const summary_struct summary) {

    struct timespec ret;
    time_t diff_sec;
    int64_t diff_nsec;

    diff_sec  = summary.finish_timestamp.tv_sec  - summary.start_timestamp.tv_sec;
    diff_nsec = summary.finish_timestamp.tv_nsec - summary.start_timestamp.tv_nsec;

    if(diff_nsec < 0) {
        --diff_sec;
        diff_nsec += ONE_PER_NANO;
    }

    ret.tv_sec  = diff_sec;
    ret.tv_nsec = diff_nsec;

    return ret;
}
