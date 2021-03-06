#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <libgen.h>
#include <errno.h>

#include "measurement_info.h"
#include "rawdata.h"
#include "powerlog.h"
#include "summary.h"
#include "caffelog.h"
#include "log_to_stat.h"
#include "stat.h"
#include "tx2_sysfs_power.h"
#include "enhanced_shcmd.h"
#include "constants.h"
#include "default_values.h"
#include "governor/governor.h"
#include "privilege.h"

#define HELP_FIRST_COLWIDTH         30
#define AVAILABLE_OPTIONS   "-"   "c:f:g:hi:"

 
static summary_struct   summary_caffe, summary_cnn, summary_batch;

void help() {

    printf("\nJetson TX2 Power Measurement Script");
    printf("\nUsage: tx2_power_measurement [options] arguments\n");
    printf("\n\t%-*s%s", HELP_FIRST_COLWIDTH, "-c <component>",
            "A component whose power consumption will be measured.");
    printf("\n\t%-*s%s", HELP_FIRST_COLWIDTH, "", "Supported components: all, cpu, gpu, ddr, wifi, soc");
    printf("\n\t%-*s%s", HELP_FIRST_COLWIDTH, "-f <file name>",
                    "An execution file");
    printf("\n\t%-*s%s", HELP_FIRST_COLWIDTH, "-g <governor>",
                    "Set an userspace governor");
    printf("\n\t%-*s%s", HELP_FIRST_COLWIDTH, "-h",
            "Print help message");
    printf("\n\t%-*s%s", HELP_FIRST_COLWIDTH, "-i <interval in us>",
            "Measurement interval in micro-second (Default and MIN: 10000)");
    printf("\n");
}

void prepare_measurement(const int argc, char *argv[], measurement_info_struct *info) {

    //
    int option, index;
    int cflag = 0, fflag = 0, gflag = 0, iflag = 0;
    int interval_us;
    char component_str[16];
    char given_dirname[MAX_BUFFLEN], filename_prefix[MAX_BUFFLEN], stat_filename_buff[MAX_BUFFLEN];
    const char *stat_filename, *basename_ptr;
    int stat_fd;
    char rawdata_filename[MAX_BUFFLEN];
    int rawdata_fd;
    char caffelog_filename[MAX_BUFFLEN];
    int caffelog_fd;
    char powerlog_filename[MAX_BUFFLEN];
    int powerlog_fd;
    char token[MAX_BUFFLEN], *next_token;
    char **child_cmd, child_cmd_str[MAX_BUFFLEN];

    char buff[MAX_BUFFLEN], filename_buff[MAX_BUFFLEN];
    struct timeval walltime;
    struct tm *walltime_calendar;

#ifdef DEBUG
    printf("\nprepare_measurement()   START");
#endif   // DEBUG

    drop_root_privilege_temp();

    init_info(info);

    // GMT (Greenwich Mean Time)
    if(gettimeofday(&walltime, NULL) == -1) {
        perror("gettimeofday() call error");
        exit(-1);
    }

    walltime_calendar = localtime(&walltime.tv_sec);
    if(!walltime_calendar) {
        perror("localtime() call error");
        exit(-1);
    }

    info->start_time.tv_sec = walltime.tv_sec;
    info->start_time.tv_nsec = MICRO_PER_NANO * walltime.tv_usec;
    info->calendar_start_time = *walltime_calendar;

    while((option = getopt(argc, argv, AVAILABLE_OPTIONS)) != -1) {
        switch(option) {

        case 'c':   // option -c
            strcpy(component_str, optarg);
            cflag = 1;
            break;

        case 'f':   // option -f with required argument
            strcpy(stat_filename_buff, optarg);
            fflag = 1;
            break;

        case 'g':   // option -g with optional argument
            gflag = 1;
            strcpy(info->gpugov_name, optarg);
            break;

        case 'h':   // option -h without argument
            help();
            exit(0);

        case 'i':   // option -i with required argument
            interval_us = atoi(optarg);
            info->powertool_interval.tv_sec = interval_us / ONE_PER_MICRO;
            info->powertool_interval.tv_nsec = (interval_us % ONE_PER_MICRO) * MICRO_PER_NANO;
            iflag = 1;
            break;

        case 1:   // non-optional argument
            // End argument processing
            optind--;
            goto end_arg_processing;

        case ':':   // Missing arguments
            fprintf(stderr, "\nMissing arguments for option %c", optopt);
            help();
            exit(-1);

        case '?':   // Invalid option
            fprintf(stderr, "\nInvalid option: %c\n", optopt);
            break;
        }
    }

end_arg_processing:

    if(!fflag) {

        strcpy(stat_filename_buff, "exp_result/stats.txt");
        printf("\nYou did not specify statistics file name; thus, the file name is set to the default name, %s", stat_filename_buff);
    }

    if(gflag) {
        info->userspace_gpugovernor = 1;
        init_gpugovernor();
        select_gpugovernor(info->gpugov_name, NULL);
    }

    if(argc == optind) {

        fprintf(stderr, "\nYou should give more arguments: the program to run is missing");
        help();
        exit(-1);
    }

    child_cmd = (char **)malloc(sizeof(char *) * (argc-optind+1));

    for(index=0; index < (argc-optind); index++){
        child_cmd[index] = (char *)malloc(sizeof(char) * strlen(argv[index + optind]));
        strcpy(child_cmd[index], argv[index+optind]);
        strcat(child_cmd_str, child_cmd[index]);
        if(index != (argc-optind-1))
            strcat(child_cmd_str, " ");
    }

    child_cmd[argc-optind] = NULL;

    info->child_cmd = child_cmd;

    // Set Caffe sleep request
    info->caffe_sleep_request.tv_sec =  5;
    info->caffe_sleep_request.tv_nsec = 0;

    // Set powertool measurement interval
    if(!iflag) {
        printf("\nSet measurement interval as default: 1 ms");
        info->powertool_interval.tv_sec = 0;
        info->powertool_interval.tv_nsec = ONE_MILLISECOND_TO_NANOSECOND;
    }

    // Set cooldown period
    info->cooldown_period.tv_sec =  6;
    info->cooldown_period.tv_nsec = 0;

    printf("\nCommand: %s\n", child_cmd_str);


    // Extract dirname and basename from the given stat file name
    //    * Note that dirname(), basename(), token_r()  may modify argument,
    //      thus, we use copied argument
    stat_filename = stat_filename_buff;
    strcpy(filename_buff, stat_filename);
    strcpy(given_dirname, dirname(filename_buff));
    strcpy(info->result_dirname, given_dirname);
    basename_ptr = stat_filename + strlen(given_dirname);
    strcpy(token, basename_ptr);
    strtok_r(token, ".", &next_token);
    strcpy(filename_prefix, token);

    if(access(given_dirname, F_OK) == -1) {

        info->flag_mkdir = 1;

        // $ mkdir -p
        if(mkdir_p(given_dirname, 0755) == -1)
            perror("mkdir_p()   fail");
    }
    else
        info->flag_mkdir = 0;

    // Powerlog File: OOO.powerlog.txt
    strcpy(powerlog_filename, given_dirname);
    strcat(powerlog_filename, "/");
    strcat(powerlog_filename, filename_prefix);
    strcat(powerlog_filename, ".powerlog.txt");
    strcpy(info->powerlog_filename, powerlog_filename);
    powerlog_fd = open(powerlog_filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    info->powerlog_fd = powerlog_fd;

    dup2(powerlog_fd, STDERR_FILENO);
    dup2(powerlog_fd, STDOUT_FILENO);

    // Rawdata File: OOO.rawdata.bin
    strcpy(rawdata_filename, given_dirname);
    strcat(rawdata_filename, "/");
    strcat(rawdata_filename, filename_prefix);
    strcat(rawdata_filename, ".rawdata.bin");
    strcpy(info->rawdata_filename, rawdata_filename);
    rawdata_fd = open(rawdata_filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    info->rawdata_fd = rawdata_fd;

    // Caffelog File: OOO.caffelog.txt
    strcpy(caffelog_filename, given_dirname);
    strcat(caffelog_filename, "/");
    strcat(caffelog_filename, filename_prefix);
    strcat(caffelog_filename, ".caffelog.txt");
    strcpy(info->caffelog_filename, caffelog_filename);
    caffelog_fd = open(caffelog_filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    info->caffelog_fd = caffelog_fd;

    // Summary File: OOO.summary.txt
    strcpy(filename_buff, given_dirname);
    strcat(filename_buff, "/");
    strcat(filename_buff, filename_prefix);
    strcat(filename_buff, ".summary.txt");
    strcpy(info->summary_filename, filename_buff);

    // Statistics File: OOO.txt
    stat_fd = open(stat_filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    printf("\nCreated statistic file: %s", stat_filename);

    print_expinfo(stat_fd, *info);

    // START RESERVATION
    info->summary_start = lseek(stat_fd, 0, SEEK_CUR);
    info->summary_len  = 0;

    // Reserve space to write summary
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n\nGPU Stat Summary");
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n   * Elapsed Time:    %*s.%9s seconds", 9, "", "");
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n   * GPU Utilization: (MIN) %*s.%*s %s - %*s.%*s %s (MAX)", (TX2_SYSFS_GPU_UTIL_MAX_STRLEN - 1), "", 1, "", "%", (TX2_SYSFS_GPU_UTIL_MAX_STRLEN - 1), "", 1, "", "%");
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n   * GPU Frequency:   (MIN) %*s MHz - %*s MHz (MAX)", TX2_SYSFS_GPU_MHZFREQ_MAX_STRLEN, "", TX2_SYSFS_GPU_MHZFREQ_MAX_STRLEN, "");
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n   * GPU Power:       (MIN) %*s mW - %*s mW (MAX)", TX2_SYSFS_GPU_POWER_MAX_STRLEN, "", TX2_SYSFS_GPU_UTIL_MAX_STRLEN, "");
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n   * GPU Energy:      %*s.%6s%6s%1s J", 9, "", "", "", "");

    // Reserve space to write summary during CNN
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n\nGPU Stat Summary during CNN");
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n   * Elapsed Time:    %*s.%9s seconds", 9, "", "");
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n   * GPU Utilization: (MIN) %*s.%*s %s - %*s.%*s %s (MAX)", (TX2_SYSFS_GPU_UTIL_MAX_STRLEN - 1), "", 1, "", "%", (TX2_SYSFS_GPU_UTIL_MAX_STRLEN - 1), "", 1, "", "%");
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n   * Avg. GPU Utilization: %2s.%*s%", "", 6, "");
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n   * GPU Frequency:   (MIN) %*s MHz - %*s MHz (MAX)", TX2_SYSFS_GPU_MHZFREQ_MAX_STRLEN, "", TX2_SYSFS_GPU_MHZFREQ_MAX_STRLEN, "");
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n   * GPU Power:       (MIN) %*s mW - %*s mW (MAX)", TX2_SYSFS_GPU_POWER_MAX_STRLEN, "", TX2_SYSFS_GPU_UTIL_MAX_STRLEN, "");
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n   * GPU Energy:      %*s.%6s%6s%1s J", 9, "", "", "", "");

    // FINISH RESERVATION
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n");
    info->summary_len += 500;   // Guard-banding
    info->metadata_end = lseek(stat_fd, info->summary_len, SEEK_CUR);
    close(stat_fd);

#ifdef DEBUG
    printf("\n%s() in %s:%d   info->summary_start: %ld", __func__, __FILE__, __LINE__, info->summary_start);
    printf("\n%s() in %s:%d   info->summary_len: %ld", __func__, __FILE__, __LINE__, info->summary_len);
    printf("\n%s() in %s:%d   info->metadata_end: %ld", __func__, __FILE__, __LINE__, info->metadata_end);
#endif   // DEBUG

    // Statistics file informations
    strcpy(info->stat_filename, stat_filename);

    // Register rows
    register_row_message(info, "\n\nGPU Statistics during Caffe");
    register_row1(info, row_avg_gpu_util,   &summary_caffe);
    register_row1(info, row_avg_emc_util,   &summary_caffe);
    register_row1(info, row_system_energy,  &summary_caffe);
    register_row1(info, row_gpu_energy,     &summary_caffe);
    register_row1(info, row_mem_energy,     &summary_caffe);

    register_row_message(info, "\n\nGPU Statistics during CNN");
    register_row1(info, row_avg_gpu_util,   &summary_cnn);
    register_row1(info, row_avg_emc_util,   &summary_cnn);
    register_row1(info, row_system_energy,  &summary_cnn);
    register_row1(info, row_gpu_energy,     &summary_cnn);
    register_row1(info, row_mem_energy,     &summary_cnn);


    // Register rawdata to collect
    register_rawdata(info,  collect_timestamp,
                     timestamp_to_powerlog,
                     NO_SYSFS_FILE);
    register_rawdata(info,  collect_boardpower,
                     boardpower_to_powerlog,
                     ONE_SYSFS_FILE,  TX2_SYSFS_BOARD_POWER);
    register_rawdata(info,  collect_socpower,
                     socpower_to_powerlog,
                     ONE_SYSFS_FILE,  TX2_SYSFS_SOC_POWER);
    register_rawdata(info,  collect_wifipower,
                     wifipower_to_powerlog,
                     ONE_SYSFS_FILE,  TX2_SYSFS_WIFI_POWER);
    register_rawdata(info,  collect_gpupower,
                     gpupower_to_powerlog,
                     ONE_SYSFS_FILE,  TX2_SYSFS_GPU_POWER);
    register_rawdata(info,  collect_gpufreq,
                     gpufreq_to_powerlog,
                     ONE_SYSFS_FILE,  TX2_SYSFS_GPU_FREQ);
    register_rawdata(info,  collect_gpuutil,
                     gpuutil_to_powerlog,
                     ONE_SYSFS_FILE,  TX2_SYSFS_GPU_UTIL);
    register_rawdata(info,  collect_allcpu_power,
                     allcpu_power_to_powerlog,
                     ONE_SYSFS_FILE,  TX2_SYSFS_CPU_POWER);
    register_rawdata(info,  collect_mempower,
                     mempower_to_powerlog,
                     ONE_SYSFS_FILE,  TX2_SYSFS_MEM_POWER);
    register_rawdata(info,  collect_emcfreq,
                     emcfreq_to_powerlog,
                     ONE_SYSFS_FILE,  TX2_SYSFS_EMC_FREQ);
    register_rawdata(info,  collect_emcutil,
                     emcutil_to_powerlog,
                     ONE_SYSFS_FILE,  TX2_SYSFS_EMC_UTIL);

    register_rawdata(info,  collect_gputemp,
                     gputemp_to_powerlog,
                     ONE_SYSFS_FILE,  TX2_SYSFS_GPU_TEMP);

    // Register statistics
    register_stat(info, "Time(s)", 18,
                  LOGTYPE_TIME,
                  elapsedtime_to_stat);
    register_stat(info, "BOARD-power(mW)", 15,
                  LOGTYPE_POWERLOG,
                  boardpower_to_stat);
    register_stat(info, "SYSTEM-power(mW)", 16,
                  LOGTYPE_POWERLOG,
                  system_power_to_stat);
    register_stat(info, "GPU-power(mW)", 13,
                  LOGTYPE_POWERLOG,
                  gpupower_to_stat);
    register_stat(info, "All-CPU-power(mW)", 17,
                  LOGTYPE_POWERLOG,
                  allcpu_power_to_stat);
    register_stat(info, "MEM-power(mW)", 13,
                  LOGTYPE_POWERLOG,
                  mempower_to_stat);
    register_stat(info, "MiscCore-power(mW)", 18,
                  LOGTYPE_POWERLOG,
                  socpower_to_stat);
    register_stat(info, "Wifi-power(mW)", 14,
                  LOGTYPE_POWERLOG,
                  wifipower_to_stat);
    register_stat(info, "SYSTEM-energy(J)", 19,
                  LOGTYPE_SUMMARY,
                  system_energy_to_stat);
    register_stat(info, "GPU-energy(J)", 19,
                  LOGTYPE_SUMMARY,
                  gpuenergy_to_stat);
    register_stat(info, "MEM-energy(J)", 19,
                  LOGTYPE_SUMMARY,
                  memenergy_to_stat);
    register_stat(info, "GPU-freq(MHz)", 13,
                  LOGTYPE_POWERLOG,
                  gpufreq_to_stat);
    register_stat(info, "psum_GPU-freq(s*MHz)", 19,
                  LOGTYPE_SUMMARY,
                  psum_gpufreq_to_stat);
    register_stat(info, "GPU-util(%)", 11,
                  LOGTYPE_POWERLOG,
                  gpuutil_to_stat);
    register_stat(info, "psum_GPU-util(s*%)", 18,
                  LOGTYPE_SUMMARY,
                  psum_gpuutil_to_stat);
    register_stat(info, "EMC-freq(MHz)", 13,
                  LOGTYPE_POWERLOG,
                  emcfreq_to_stat);
    register_stat(info, "EMC-util(%)", 11,
                  LOGTYPE_POWERLOG,
                  emcutil_to_stat);
    register_stat(info, "GPU-temp(degree-C)", 18,
                  LOGTYPE_POWERLOG,
                  gputemp_to_stat);
    register_stat(info, "Timestamp", 19,
                  LOGTYPE_TIMESTAMP,
                  timestamp_to_stat);
    register_stat(info, "Caffe-start", 11,
                  LOGTYPE_CAFFELOG,
                  caffe_start_to_stat);
    register_stat(info, "CNN-start/finish", 16,
                  LOGTYPE_CAFFELOG,
                  cnn_event_to_stat);
    register_stat(info, "Batch-idx", 9,
                  LOGTYPE_CAFFELOG,
                  batch_idx_to_stat);
    register_stat(info, "Batch-finish", 13,
                  LOGTYPE_CAFFELOG,
                  batch_finish_to_stat);
    register_stat(info, "Caffe-Event", 35,
                  LOGTYPE_CAFFELOG,
                  caffeevent_to_stat);

    init_caffelog_parser();

#ifdef DEBUG
    printf("\nprepare_measurement()   FINISHED");
#endif   // DEBUG
    return;
}

void calculate_2ndstat(const measurement_info_struct info) {

    rawdata_info_struct            *rawdata_info;
    powerlog_struct                powerlog;
    caffelog_struct                *caffelog, list_caffelog;
    stat_info_struct               *stat_info;
    ssize_t num_read_bytes, num_written_bytes;

    struct timespec *timestamp_ptr;
    int rawdata_fd;
    int caffelog_fd;
    off_t caffelog_offset;
    int summary_fd;
    int stat_fd;
    int batch_idx;
    int i, j;

    int flag_powerlog;
    int flag_cnnstart;
    int flag_cnnfinish;

    char buff[MAX_BUFFLEN];
    size_t buff_len;

#ifdef DEBUG
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG

    // Rawdata
    rawdata_fd = open(info.rawdata_filename, O_RDONLY | O_NONBLOCK);
    lseek(rawdata_fd, 0, SEEK_SET);

    // Caffelog
    caffelog_fd = open(info.caffelog_filename, O_RDONLY | O_NONBLOCK);
    caffelog_offset = 0;
    lseek(caffelog_fd, 0, SEEK_SET);

    // Summary
    summary_fd = open(info.summary_filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    lseek(summary_fd, 0, SEEK_SET);
    write(summary_fd, "BATCH SUMMARY", 13);
    batch_idx = 1;

    // Statistics
    stat_fd = open(info.stat_filename, O_WRONLY);
    lseek(stat_fd, info.metadata_end, SEEK_SET);
    print_header_row(stat_fd, info);

#ifdef DEBUG
    printf("\n%s() in %s:%d   Start initializing summary", __func__, __FILE__, __LINE__);
#endif   // DEBUG
    init_summary(&summary_caffe, "GPU statistics summary during Caffe");
    init_summary(&summary_cnn, "GPU statistics summary during CNN");
    init_summary(&summary_batch, "BATCH 1");
    flag_cnnstart  = 0;
    flag_cnnfinish = 0;

#ifdef DEBUG
    printf("\n%s() in %s:%d   Finish initializing summary", __func__, __FILE__, __LINE__);
    printf("\n%s() in %s:%d   Start parsing caffelogs", __func__, __FILE__, __LINE__);
#endif   // DEBUG

    // Process Caffe log file
    INIT_LIST_HEAD(&list_caffelog.list);
#ifdef DEBUG
    printf("\n%s() in %s:%d   Linked List for Caffelog: %p", __func__, __FILE__, __LINE__, &list_caffelog);
#endif   // DEBUG
    do {
        caffelog = malloc(sizeof(struct caffelog_struct));
        caffelog_offset = parse_caffelog(caffelog_fd, caffelog_offset, info.calendar_start_time, caffelog);
        if(caffelog_offset <= 0) {
            free(caffelog);
            break;
        }
        list_add_tail(caffelog, &list_caffelog.list);
    } while(1);

    close(caffelog_fd);

    if(!list_empty(&list_caffelog.list))
        caffelog = list_entry(list_caffelog.list.next, struct caffelog_struct, list);
    else
        caffelog = NULL;

#ifdef DEBUG
    printf("\n%s() in %s:%d   Finish parsing caffelogs", __func__, __FILE__, __LINE__);
#endif   // DEBUG

    // Process rawdata and write to stat file.
    // Note that making powerlogs to linked list is unreasonable,
    // because we have too many powerlogs
#define PAD_COLUMN \
    do { \
        num_written_bytes  = write(stat_fd, WHITESPACE, (stat_info->colwidth - 4)); \
        num_written_bytes += write(stat_fd, "#N/A", 4); \
    } while(0);

    while(1) {

        // Read rawdata: GPU frequency, GPU utilization, CPU infos, etc.
        for(i=0; i<info.num_rawdata; i++) {
            rawdata_info = &info.rawdata_info[i];
            num_read_bytes = rawdata_info->func_rawdata_to_powerlog(&powerlog, rawdata_fd);
            if (num_read_bytes <= 0) goto rawdata_eof_found;
        }

        // Update summary
        update_summary(&summary_caffe, &powerlog);

compare_timestamp:
        // Compare timestamps and set/unset flag
        if(caffelog == NULL || diff_timestamp(powerlog.timestamp, caffelog->timestamp) < 0)
            flag_powerlog = 1;
        else
            flag_powerlog = 0;

        // Update summary for CNN inference
        if(flag_powerlog & flag_cnnstart & (!flag_cnnfinish)) {
            update_summary(&summary_cnn, &powerlog);
            update_summary(&summary_batch, &powerlog);
        }


        // Convert rawdata to stat and write to statfile
        write(stat_fd, "\n", 1);
        for(j=0; j<info.num_stat; j++) {
            write(stat_fd, "  ", 2);
            stat_info = &info.stat_info[j];
            switch(info.stat_info[j].logtype) {

                case LOGTYPE_TIME:
                    if(flag_powerlog)
                        timestamp_ptr = &powerlog.timestamp;
                    else
                        timestamp_ptr = &caffelog->timestamp;
                    num_written_bytes = stat_info->func_log_to_stat(stat_fd, stat_info->colwidth, *timestamp_ptr, info.start_time);
                    break;

                case LOGTYPE_TIMESTAMP:
                    if(flag_powerlog)
                        timestamp_ptr = &powerlog.timestamp;
                    else
                        timestamp_ptr = &caffelog->timestamp;
                    num_written_bytes = stat_info->func_log_to_stat(stat_fd, stat_info->colwidth, *timestamp_ptr);
                    break;

                case LOGTYPE_POWERLOG:
                    if(flag_powerlog)
                        num_written_bytes = stat_info->func_log_to_stat(stat_fd, stat_info->colwidth, powerlog);
                    else 
                        PAD_COLUMN;
                    break;

                case LOGTYPE_SUMMARY:
                    if(flag_powerlog)
                        num_written_bytes = stat_info->func_log_to_stat(stat_fd, stat_info->colwidth, summary_caffe);
                    else
                        PAD_COLUMN;
                    break;

                case LOGTYPE_CAFFELOG:
                    if(!flag_powerlog)
                        num_written_bytes = stat_info->func_log_to_stat(stat_fd, stat_info->colwidth, *caffelog);
                    else
                        PAD_COLUMN;
                 break;

                case LOGTYPE_TEGRALOG:
                case LOGTYPE_NA:
                default:
                    break;
            }

            if (num_written_bytes <= 0) break;
        }
 
        if(!flag_powerlog) {
            if(!list_empty(&list_caffelog.list)) {

#ifdef DEBUG
                printf("\n%s() in %s:%d   Iterate to next caffelog: %p -> ", __func__, __FILE__, __LINE__, caffelog);
#endif   // DEBUG

                // Batch finish
                if(caffelog->batch_finish == INFINITE) {
                    print_summary_name(summary_fd, &summary_batch);
                    print_summary_runtime(summary_fd, &summary_batch);
                    print_summary_gpu_util_range(summary_fd, &summary_batch);
                    print_summary_emc_util_range(summary_fd, &summary_batch);
                    print_summary_gpu_freq_range(summary_fd, &summary_batch);
                    print_summary_gpu_power_range(summary_fd, &summary_batch);

                    // New batch summary
                    ++batch_idx;
                    snprintf(buff, MAX_BUFFLEN, "BATCH %d", batch_idx);
                    init_summary(&summary_batch, buff);
                }

                // Delete from list and deallocate
                list_del(&caffelog->list);
                free(caffelog);
                caffelog = list_entry(list_caffelog.list.next, struct caffelog_struct, list);
                if(caffelog == &list_caffelog)
                    caffelog = NULL;
                else {
                    if(caffelog->cnn_start == INFINITE)
                        flag_cnnstart = 1;
                    if(caffelog->cnn_finish == INFINITE)
                        flag_cnnfinish = 1;
                }
#ifdef DEBUG
                printf("%p", caffelog);
#endif   // DEBUG
            }
            else
                caffelog = NULL;
            goto compare_timestamp;
        }
    }   // while(1)
#undef PAD_COLUMN
rawdata_eof_found:

    // Write registered rows
    print_registered_rows(stat_fd, info);

    // Write summary
#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    print_summary(STDOUT_FILENO, &summary_caffe);
#endif   // DEBUG or DEBUG_SUMMARY

    print_summary_name(stat_fd, &summary_caffe);
    print_summary_runtime(stat_fd, &summary_caffe);
    print_summary_gpu_util_range(stat_fd, &summary_caffe);
    print_summary_emc_util_range(stat_fd, &summary_caffe);
    print_summary_gpu_freq_range(stat_fd, &summary_caffe);
    print_summary_gpu_power_range(stat_fd, &summary_caffe);

    // Write summary during CNN
#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    print_summaryptr(STDOUT_FILENO, &summary_cnn);
#endif   // DEBUG or DEBUG_SUMMARY

    print_summary_name(stat_fd, &summary_cnn);
    print_summary_runtime(stat_fd, &summary_cnn);
    print_summary_gpu_util_range(stat_fd, &summary_cnn);
    print_summary_emc_util_range(stat_fd, &summary_cnn);
    print_summary_gpu_freq_range(stat_fd, &summary_cnn);
    print_summary_gpu_power_range(stat_fd, &summary_cnn);

    // FINISH writting summary
    buff_len = snprintf(buff, MAX_BUFFLEN, "\n");
    write(stat_fd, buff, buff_len);

    // Close and remove rawdata.bin file
    close(rawdata_fd);
    close(summary_fd);
    close(stat_fd);

#ifdef DEBUG
    printf("\n%s() in %s:%d   FINISH", __func__, __FILE__, __LINE__);
#endif   // DEBUG
    return;
}

void finish_measurement(measurement_info_struct *info) {

    remove(info->rawdata_filename);
    close(info->powerlog_fd);

    if(info->flag_mkdir) {

        /*
        printf("\nchown -R %s", info->result_dirname);

        // $ chown -R
        if(chown_R(info->result_dirname, DEFAULT_UID, DEFAULT_GID) == -1)
            perror("chown_R() fail");
        */
    }

    if(info->userspace_gpugovernor)
        finish_gpugovernor();

    free_caffelog_parser();

    return;
}

int main(int argc, char *argv[]) {

    int pid;
    measurement_info_struct info;
    struct timespec sleep_remain;

#ifdef DEBUG
    printf("\nYou are running debug mode");
#endif   // DEBUG

    /*
    if(geteuid() != 0) {

        printf("\nPlease run this power measurement tool as root privilege\n");
        exit(0);
    }
    */

    prepare_measurement(argc, argv, &info);

    // Run
    pid = fork();

    if(pid == 0) {
        // Child Process
        dup2(info.caffelog_fd, STDOUT_FILENO);
        dup2(info.caffelog_fd, STDERR_FILENO);

        // Sleep enough
        while(nanosleep(&info.caffe_sleep_request, &sleep_remain) == -1)
            nanosleep(&sleep_remain, &sleep_remain);

        execvp(info.child_cmd[0], info.child_cmd);

        // If error, execve() returns -1. Otherwise, execve() does not return value
        perror("\nexecve() error");
    }

    // Parent Process
    measure_rawdata(pid, info);
    calculate_2ndstat(info);
    finish_measurement(&info);

    return 0;
}
