//
// Created by Mike Smith on 2018-12-23.
//

#ifndef WATERYSQL_RECORD_DESCRIPTOR_PRINTER_H
#define WATERYSQL_RECORD_DESCRIPTOR_PRINTER_H

#include "../type/non_constructible.h"
#include "../../data/record_descriptor.h"
#include "printer.h"
#include "html_table_printer.h"
#include "../../data/type_tag_helper.h"

namespace watery {

struct RecordDescriptorPrinter : NonConstructible {
    
    static std::vector<std::string> make_row(
        const std::string &name, size_t size, TypeTag type,
        bool implicit, bool nullable, bool foreign,
        const std::string &foreign_table, const std::string &foreign_column,
        bool primary, bool unique) noexcept {
        
        return {
            name,                                           // name
            std::to_string(size).append(" bytes"),          // size
            std::string{TypeTagHelper::name(type)},         // type
            std::string{implicit ? "Yes" : "No"},           // implicit
            std::string{nullable ? "Yes" : "No"},           // nullable
            foreign ?
            (foreign_table + ".").append(foreign_column) :
            std::string{"No"},                              // foreign
            std::string{primary ? "Yes" : "No"},            // primary
            std::string{unique ? "Yes" : "No"}              // unique
        };
        
    }
    
    static void print(std::ofstream &f, const RecordDescriptor &descriptor) noexcept {
        
        HtmlTablePrinter table_printer{f};
        
        table_printer.print_header(
            {"Field", "Size", "Type", "Implicit", "Nullable", "Foreign Key", "Primary Key", "Unique"});
        
        if (descriptor.null_mapped) {
            table_printer.print_row(make_row(
                "null_bitmap", sizeof(NullFieldBitmap), TypeTag::BITMAP,
                true, false, false, "", "", false, false));
        }
        if (descriptor.reference_counted) {
            table_printer.print_row(make_row(
                "ref_count", sizeof(uint32_t), TypeTag::INTEGER,
                true, false, false, "", "", false, false));
        }
        std::for_each(
            descriptor.field_descriptors.begin(),
            descriptor.field_descriptors.begin() + descriptor.field_count,
            [&](FieldDescriptor fd) {
                table_printer.print_row(make_row(
                    fd.name.data(), fd.data_descriptor.length, fd.data_descriptor.type,
                    false, fd.constraints.nullable(), fd.constraints.foreign(),
                    fd.foreign_table_name.data(), fd.foreign_column_name.data(), false, false));
            });
    }
    
};

}

#endif  // WATERYSQL_RECORD_DESCRIPTOR_PRINTER_H
