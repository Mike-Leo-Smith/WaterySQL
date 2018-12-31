//
// Created by Mike Smith on 2019-01-01.
//

#ifndef WATERYSQL_WATERY_SQL_H
#define WATERYSQL_WATERY_SQL_H

#ifdef __cplusplus
extern "C" {
#endif

void watery_sql_init();
void watery_sql_execute(const char *command, void (*recv)(const char *row[], unsigned long field_count));

#ifdef __cplusplus
};
#endif

#endif  // WATERYSQL_WATERY_SQL_H
