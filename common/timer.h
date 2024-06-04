#ifndef _NTGRAPH_COMMON_TIMER_H_
#define _NTGRAPH_COMMON_TIMER_H_

#include <string>
#include <ctime>

static size_t get_current_timestamp() {
    time_t rawtime;
    struct tm* ptm;

    time (&rawtime);
    ptm = gmtime (&rawtime);

    /* YYYYMMDDHHMMSS */

    size_t timestamp = 0;
    timestamp += (ptm->tm_year + 1900) * 10000000000;
    timestamp += (ptm->tm_mon + 1) *       100000000;
    timestamp += ptm->tm_mday *              1000000;
    timestamp += ptm->tm_hour *                10000;
    timestamp += ptm->tm_min *                   100;
    timestamp += ptm->tm_sec;

    return timestamp;
}

static const std::string get_date() {
    time_t t = time(0);
    struct tm* dt = localtime(&t);
    std::string month = dt->tm_mon + 1 < 10 ? "0" + std::to_string(dt->tm_mon + 1) : std::to_string(dt->tm_mon + 1);
    std::string day = dt->tm_mday < 10 ? "0" + std::to_string(dt->tm_mday) : std::to_string(dt->tm_mday);
    return std::to_string(dt->tm_year + 1900) + month + day;
}

static const std::string get_time() {
    time_t t = time(0);
    struct tm* dt = localtime(&t);
    std::string hour = dt->tm_hour < 10 ? "0" + std::to_string(dt->tm_hour) : std::to_string(dt->tm_hour);
    std::string min = dt->tm_min < 10 ? "0" + std::to_string(dt->tm_min) : std::to_string(dt->tm_min);
    std::string sec = dt->tm_sec < 10 ? "0" + std::to_string(dt->tm_sec) : std::to_string(dt->tm_sec);
    return hour + ":" + min + ":" + sec;
}


#endif // _NTGRAPH_COMMON_TIMER_H_
