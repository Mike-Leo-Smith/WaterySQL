from time import strftime, localtime
from os import system


def extract_arg_name(arg: str) -> str:
    name = arg.split()[-1]
    for i in range(len(name)):
        if name[i] != "&" and name[i] != "*":
            return name[i:]
    return "<BAD>"


def fill_message_template(template: str, arg_names: [str]) -> str:
    for i in range(len(arg_names) - 1, -1, -1):
        template = template.replace(f"${i}", arg_names[i])
    return template


def assemble_message(messages: [str]) -> str:
    result = "std::string{" + messages[0] + "}"
    for msg in messages[1:]:
        result = f"{result}\n            .append({msg})"
    return result


def convert_to_snake_case(error_name: str) -> str:
    error_name = f"{str.lower(error_name[0])}{error_name[1:]}"
    for i in range(ord("A"), ord("Z")):
        error_name = error_name.replace(chr(i), f"_{str.lower(chr(i))}")
    return error_name


def convert_to_upper_case(error_name: str) -> str:
    return str.upper(convert_to_snake_case(error_name))


def date_string() -> str:
    return strftime("%Y-%m-%d", localtime())


def generate_formals(args: [str]) -> str:
    if len(args) == 0:
        return ""
    return f"{args[0]}" + "".join(f", {arg}" for arg in args[1:])


def main():
    error_name = input("Error Class Name: ")
    constructor_args = []
    arg = input(f"Arg #{len(constructor_args)}: ")
    while arg != "":
        constructor_args.append(arg)
        arg = input(f"Arg #{len(constructor_args)}: ")
    arg_names = [extract_arg_name(arg) for arg in constructor_args]
    messages = []
    msg_template = input("Message Template:\n")
    while msg_template != "":
        messages.append(fill_message_template(msg_template, arg_names))
        msg_template = input()
    file_name = f"{convert_to_snake_case(error_name)}.h"
    marco_name = f"WATERYSQL_{convert_to_upper_case(error_name)}_H"
    assembled_message = assemble_message(messages)
    date = date_string()
    formals = generate_formals(constructor_args)
    with open(file_name, "w+") as file:
        print(f'''//
// Created by Mike Smith on {date}.
//

#ifndef {marco_name}
#define {marco_name}

#include <string>
#include "error.h"

namespace watery {{

struct {error_name} : public Error {{

    {"explicit " if len(constructor_args) == 1 else ""}{error_name}({formals})
        : Error{{
        "{error_name}",
        {assembled_message}}} {{}}

}};

}}

#endif  // {marco_name}''', file=file)
    system(f"git add {file_name}")


if __name__ == "__main__":
    main()
