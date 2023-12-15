//
// Created by Jasper Edbrooke on 12/6/23.
//
#include <iostream>
#include <fstream>
#include <string>
#include "JSON.h"

JSON_Object execute(JSON_Object program, const std::string& function_name, JSON_Object input_args, JSON_Object stack);
JSON_Object evaluate(const JSON_Object& program, JSON_Object token, const JSON_Object& stack);


JSON_Object evaluate_string(JSON_Object program, std::string token, JSON_Object stack) {
    if (stack.items().contains(token)) {
        return stack[token];
    }

    if (stack.items().contains("args")) {
        if (stack["args"].items().contains(token)) {
            return stack["args"][token];
        }
    }

    if (stack.items().contains("outer")) {
        return evaluate_string(program, token, stack["outer"]);
    }

    if (program["data"].items().contains(token)){
        return program["data"][token];
    }


    std::cerr << "unknown token: " << token << std::endl;
}

JSON_Object evaluate_function_call(const JSON_Object& program, JSON_Object call, JSON_Object stack) {
    JSON_Object value;
    for (auto& [function, args]: call.items()) {
        value = execute(program, function, args, stack);
    }
    return value;
}

JSON_Object evaluate_list(const JSON_Object& program, JSON_Object list, JSON_Object stack) {
    std::vector<JSON_Object> items;
    for (auto&& item: list) {
        items.push_back(evaluate(program, item, stack));
    }
    return JSON_Object(items);
}

JSON_Object evaluate(const JSON_Object& program, JSON_Object token, const JSON_Object& stack) {
    JSON_Object value;
    switch (token.getType()) {
        case object:
            value = evaluate_function_call(program, token, stack);
            break;
        case list:
            value = evaluate_list(program, token, stack);
            break;
        case string:
            value = evaluate_string(program, token.toString(), stack);
            break;
        case number:
        case json_bool:
        case json_null:
            value = token;
    }

    return value;
}



JSON_Object execute(JSON_Object program, const std::string &function_name, JSON_Object input_args, JSON_Object stack) {
    // handle builtin functions
    if (function_name == "print") {
        for (auto&& arg: input_args) {
            std::cout << evaluate(program, arg, stack) << std::endl;
        }
        return JSON_Object(0.0);
    }

    if (function_name == "add") {
        double sum = 0;
        for (auto&& arg: input_args) {
            JSON_Object value = evaluate(program, arg, stack);
            sum += std::get<double>(value.getValue());
        }
        return JSON_Object(sum);
    }

    if (function_name == "mul") {
        double product = 1;
        for (auto&& arg: input_args) {
            JSON_Object value = evaluate(program, arg, stack);
            product *= std::get<double>(value.getValue());
        }
        return JSON_Object(product);
    }

    if (function_name == "if") {
        JSON_Object condition = evaluate(program, input_args[0], stack);
        JSON_Object value;
        if (std::get<bool>(condition.getValue())) {
            value = evaluate(program, input_args[1], stack);
        } else {
            value = evaluate(program, input_args[2], stack);
        }
        return value;
    }

    if (function_name == "eq") {
        JSON_Object lhs = evaluate(program, input_args[0], stack);
        JSON_Object rhs = evaluate(program, input_args[1], stack);
        return JSON_Object(lhs == rhs);
    }

    if (function_name == "float") {
        return JSON_Object(std::stod(evaluate(program, input_args[0], stack).toString()));
    }

    JSON_Object function = program["functions"][function_name];

    JSON_Object new_stack;
    if (function.items().contains("args")) {
        for (int i = 0; i < function["args"].size(); i++) {
            new_stack.insert(function["args"][i].toString(), evaluate(program, input_args[i], stack));
        }
    }
    new_stack.insert("outer",stack);
    JSON_Object retval;
    for (auto& [name, args]: function["statements"].items()) {
        retval = execute(program, name, args, new_stack);
    }
    return retval;

}

int main(int argc, char* args[]) {
    std::string path;
    if (argc > 1) {
        path.append(args[1]);
    } else {
        std::cerr << "please provide input file" << std::endl;
        exit(1);
    }


    std::ifstream file(path.c_str());

    std::vector<JSON_Object> main_args_list;
    if (argc > 2) {
        for (int i = 2; i < argc; ++i) {
            main_args_list.push_back(JSON_Object(std::string(args[i])));
        }
    }
    JSON_Object main_args(main_args_list);

    if (file.is_open()) {
        try {
            JSON_Object json = JSON::loads(file);
            std::cerr << "beginning execution" << std::endl;
            auto result = execute(json, "main", main_args, json["data"]);
            std::cout << result << std::endl;

        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
    } else {
        std::cerr << "could not open " << path << std::endl;
        std::exit(1);
    }
}