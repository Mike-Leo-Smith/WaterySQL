//
// Created by Mike Smith on 2018-12-23.
//

#ifndef WATERYSQL_RECORD_DESCRIPTOR_PRINTER_H
#define WATERYSQL_RECORD_DESCRIPTOR_PRINTER_H

#include "../type/non_trivial_constructible.h"
#include "../../data/record_descriptor.h"
#include "printer.h"

namespace watery {

struct RecordDescriptorPrinter : NonTrivialConstructible {
    
    template<typename Stream>
    static void print(Stream &os, const RecordDescriptor &descriptor) noexcept {
        Printer::println(os, "  Record of length ", descriptor.length);
        std::for_each(
            descriptor.field_descriptors.begin(),
            descriptor.field_descriptors.begin() + descriptor.field_count,
            [&os](FieldDescriptor fd) {
                Printer::print(
                    os, "    ", fd.name.data(), ": ", fd.data_descriptor.type,
                    "(", fd.data_descriptor.length, "B) | ",
                    fd.constraints.nullable() ? "NULL " : "NOT NULL ");
                if (fd.constraints.foreign()) {
                    Printer::print(
                        os, "| FOREIGN KEY REFERENCES ", fd.foreign_table_name.data(),
                        "(", fd.foreign_column_name.data(), ") ");
                }
                if (fd.constraints.primary()) {
                    Printer::print(os, "| PRIMARY KEY ");
                }
                if (fd.constraints.unique()) {
                    Printer::print(os, "| UNIQUE ");
                }
                Printer::print(os, "\n");
            });
        if (descriptor.null_mapped) {
            Printer::println(os, "    [implicit] null_map: ", sizeof(NullFieldBitmap));
        }
        if (descriptor.reference_counted) {
            Printer::println(os, "    [implicit] ref_count: ", sizeof(uint32_t));
        }
        Printer::println(os);
    }
    
};

}

#endif  // WATERYSQL_RECORD_DESCRIPTOR_PRINTER_H