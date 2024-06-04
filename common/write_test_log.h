#ifndef _NTGRAPH_COMMON_WRITE_TEST_LOG_H_
#define _NTGRAPH_COMMON_WRITE_TEST_LOG_H_

#include "common/file_util.h"
#include "common/timer.h"


static void write_log_file(const std::string log_directory, const std::string content) {

    std::string date = get_date();
    std::string file_name = log_directory + date + "_" + "insert_test.log";
    append_file(file_name, content);
}


#endif // _NTGRAPH_COMMON_WRITE_TEST_LOG_H_