//
// Created by Mike Smith on 2019-01-01.
//

#include "html_table_printer.h"
#include "printer.h"

namespace watery {

HtmlTablePrinter::HtmlTablePrinter(std::ofstream &s)
    : _stream{s} {
    Printer::print(_stream, "<table border='1' cellspacing='0'>\n");
}

HtmlTablePrinter::~HtmlTablePrinter() {
    Printer::print(_stream, "</table><br/>\n");
}

void HtmlTablePrinter::print_header(const std::vector<std::string> &header) noexcept {
    Printer::print(_stream, "<tr>\n");
    for (auto &&col : header) { Printer::print(_stream, "<th>", col, "</th>\n"); }
    Printer::print(_stream, "</tr>\n");
}

void HtmlTablePrinter::print_row(const std::vector<std::string> &row) noexcept {
    Printer::print(_stream, "<tr>\n");
    for (auto &&col : row) {
        Printer::print(_stream, "<td>", col, "</td>\n");
    }
    Printer::print(_stream, "</tr>\n");
}

}
