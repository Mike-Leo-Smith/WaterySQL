//
// Created by Mike Smith on 2019-01-01.
//

#ifndef WATERYSQL_HTML_TABLE_PRINTER_H
#define WATERYSQL_HTML_TABLE_PRINTER_H

#include <fstream>
#include <vector>
#include <string>

namespace watery {

class HtmlTablePrinter {

private:
    std::ofstream &_stream;

public:
    HtmlTablePrinter(std::ofstream &s);
    ~HtmlTablePrinter();
    void print_header(const std::vector<std::string> &header) noexcept;
    void print_row(const std::vector<std::string> &row) noexcept;
    
};

}

#endif  // WATERYSQL_HTML_TABLE_PRINTER_H
