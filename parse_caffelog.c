#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "parse_caffelog.h"
#include "constants.h"

#define NO_REGEX_EFLAGS   0

int64_t diff_timestamp(const struct timespec timestamp1, const struct timespec timestamp2) {

    /*
     *   This function compares two timestamps.
     *   Like strcmp() function, this function returns positive number
     *   if timestamp1 is larger, zero if both timestamp is identical,
     *   or negative number if timestamp2 is larger.
     *
     *   This function returns the difference of two timestamps in nanosecond
     *   scale. If the difference is more than range of int64_t, this function
     *   returns INT64_MIN or INT64_MAX. However, there are enuogh room
     *   because the range covers about 292 years
     */

    int64_t diff_sec, diff_nsec, diff_ns;

    diff_sec   = timestamp1.tv_sec  - timestamp2.tv_sec;
    diff_nsec  = timestamp1.tv_nsec - timestamp2.tv_nsec;

    diff_ns   = ONE_SECOND_TO_NANOSECOND * diff_sec + diff_nsec;
    return diff_ns;
}

off_t parse_caffelog(const int caffelog_fd, const regex_t timestamp_pattern, const off_t offset, const struct tm calendar, caffelog_struct *caffelog) {

    /*
     *  This function returns end of line of parsed caffelog.
     *  If -1 is returned, the result should be ignored
     */
    off_t new_offset = offset;

    // Timestamp
    const size_t num_regexmatch = 2 + 1;   // 0th is whole match
    regmatch_t matched_regex[num_regexmatch];
    char timebuff[7];
    const char *start_ptr;

    // Line
    char buff[256];
    const char *eol;
    ssize_t read_bytes;

#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   START");
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG

    if(offset < 0) {
#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   FINISHED: Maybe end of file reached");
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG

        return -1;
    }

    // Copy calendar informations
    caffelog->date_hms.tm_isdst = calendar.tm_isdst;
    caffelog->date_hms.tm_yday  = calendar.tm_yday;
    caffelog->date_hms.tm_wday  = calendar.tm_wday;
    caffelog->date_hms.tm_year  = calendar.tm_year;
    caffelog->date_hms.tm_mon   = calendar.tm_mon;
    caffelog->date_hms.tm_mday  = calendar.tm_mday;

read_a_line:
    read_bytes = pread(caffelog_fd, buff, 256, new_offset);

    if(read_bytes <= 0) {
#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
        perror("pread() FAIL");
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG

        return -1;
    }

    for(eol=buff; (*eol)!='\n'; eol++) {/* Just finding line */}
    buff[eol-buff] = '\0';

    new_offset += (eol-buff+1);

#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   Line: %s", buff);
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG

    if(regexec(&timestamp_pattern, buff, num_regexmatch, matched_regex, NO_REGEX_EFLAGS))
        goto read_a_line;

#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   matched_regex[0].rm_so: %u", matched_regex[0].rm_so);
    printf("\nparse_caffelog()   matched_regex[0].rm_eo: %u", matched_regex[0].rm_eo);
    printf("\nparse_caffelog()   matched_regex[1].rm_so: %u", matched_regex[1].rm_so);
    printf("\nparse_caffelog()   matched_regex[1].rm_eo: %u", matched_regex[1].rm_eo);
    printf("\nparse_caffelog()   matched_regex[2].rm_so: %u", matched_regex[2].rm_so);
    printf("\nparse_caffelog()   matched_regex[2].rm_eo: %u", matched_regex[2].rm_eo);
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG

    if(matched_regex[0].rm_so == matched_regex[0].rm_eo) {
#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   No Match");
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG
        goto read_a_line;
    }

    // Hour
    start_ptr = buff + matched_regex[1].rm_so;
    strncpy(timebuff, start_ptr, 2);
    timebuff[2] = '\0';
#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   timebuff hour: %s", timebuff);
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG
    caffelog->date_hms.tm_hour = atoi(timebuff);

    // Minute
    start_ptr += 3;
    strncpy(timebuff, start_ptr, 2);
    timebuff[2] = '\0';
#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   timebuff min: %s", timebuff);
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG
    caffelog->date_hms.tm_min = atoi(timebuff);

    // Second
    start_ptr += 3;
    strncpy(timebuff, start_ptr, 2);
    timebuff[2] = '\0';
#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   timebuff sec: %s", timebuff);
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG
    caffelog->date_hms.tm_sec = atoi(timebuff);
    (&caffelog->timestamp)->tv_sec = mktime(&caffelog->date_hms);


    // Nanosecond
    start_ptr += 3;
    strncpy(timebuff, start_ptr, 6);
    timebuff[6] = '\0';
#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   timebuff ns: %s", timebuff);
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG
    (&caffelog->timestamp)->tv_nsec = MICROSECOND_TO_NANOSECOND * atoi(timebuff);

    // Event
    // Note that putting caffelog message in " " makes MS Excel to recognize
    // it as a single string
    strcpy(caffelog->event, "\"[Caffe] ");
    strcat(caffelog->event, buff + matched_regex[2].rm_so);
    strcat(caffelog->event, "\"");
#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   event: %s", caffelog->event);
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG

#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   FINISHED");
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG

    return new_offset;
}
