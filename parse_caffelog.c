#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse_caffelog.h"

#define MICROSECOND_TO_NANOSECOND 1000

off_t parse_caffelog(const int caffelog_fd, const regex_t timestamp_pattern, const off_t offset, struct caffe_event *event) {

    off_t new_offset = offset;

    // Timestamp
    const size_t num_timestamp = 1;
    regmatch_t timestamp[num_timestamp];
    char timebuff[7];
    const char *start_ptr;
    struct tm date_timestamp;

    // Line
    char buff[256];
    const char *eol;
    ssize_t read_bytes;

#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   START");
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG

    read_bytes = pread(caffelog_fd, buff, 256, offset);

    if(read_bytes <= 0) {
        perror("pread() FAIL");
        return -1;
    }

    for(eol=buff; (*eol)!='\n'; eol++) {/* Just finding line */}
    buff[eol-buff] = '\0';

    new_offset += (eol-buff+1);

#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nLine: %s", buff);
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG

    regexec(&timestamp_pattern, buff, num_timestamp, timestamp, 0);

    // Hour
    start_ptr = buff + timestamp[0].rm_so;
    strncpy(timebuff, start_ptr, 2);
    timebuff[3] = '\0';
#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\ntimebuff hour: %s", timebuff);
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG
    date_timestamp.tm_hour = atoi(timebuff);

    // Minute
    start_ptr += 3;
    strncpy(timebuff, start_ptr, 2);
    timebuff[3] = '\0';
#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\ntimebuff min: %s", timebuff);
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG
    date_timestamp.tm_min = atoi(timebuff);

    // Second
    start_ptr += 3;
    strncpy(timebuff, start_ptr, 2);
    timebuff[3] = '\0';
#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\ntimebuff sec: %s", timebuff);
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG
    date_timestamp.tm_sec = atoi(timebuff);
    (&event->gmt_timestamp)->tv_sec = mktime(&date_timestamp);

    // Nanosecond
    start_ptr += 3;
    strncpy(timebuff, start_ptr, 6);
    timebuff[7] = '\0';
#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\ntimebuff ns: %s", timebuff);
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG
    (&event->gmt_timestamp)->tv_nsec = MICROSECOND_TO_NANOSECOND * atoi(timebuff);

    return new_offset;
}