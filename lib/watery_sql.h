//
// Created by Mike Smith on 2019-01-01.
//

#ifndef WATERYSQL_WATERY_SQL_H
#define WATERYSQL_WATERY_SQL_H

#include <functional>
#include <string>
#include <vector>

namespace watery {

void execute_sql(const std::string &command, const std::function<void(const std::vector<std::string> &)> &recv);

}

#endif  // WATERYSQL_WATERY_SQL_H
