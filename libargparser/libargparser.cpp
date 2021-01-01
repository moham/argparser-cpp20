export module argparser;

#if defined(__GXX_WEAK__)
#include <bits/gthr-default.h>
#endif

#include <iostream>
#include <string>
#include <variant>
#include <utility>
#include <algorithm>
#include <vector>
#include <map>
#include <optional>
#include <cstdlib>
#include <functional>
#include <memory>
#include <list>

namespace argparser {

template<typename T>
struct Result {
    T value;
    std::optional<std::string> error = std::nullopt;

    explicit operator bool() {
        return error ? false : true;
    }
};

using Error = std::optional<std::string>;

void throw_error(const std::string& message) {
    std::cout << "[ERROR] libargparser: " << message << "\n";
    std::exit(1);
}

template<typename T>
bool is_in_vector(const std::vector<T>& vector, const T& element) {
    auto iter = std::find(vector.begin(), vector.end(), element);
    return iter != vector.end();
}

using ArgTypes = std::variant<
    std::string,
    int,
    long,
    unsigned long,
    long long,
    unsigned long long,
    float,
    double,
    long double,
    bool
>;

std::vector<std::string> ArgTypeIndex = {
    "string",
    "int",
    "long",
    "unsigned long",
    "long long",
    "unsigned long long",
    "float",
    "double",
    "long double",
    "bool"
};

export class Argument {
private:
    std::string m_type;
    std::string m_name;
    std::string m_long_argument;
    std::string m_short_argument;
    std::string m_description;
    std::optional<ArgTypes> m_default_value;
    std::optional<ArgTypes> m_value;
    bool m_positional;
    std::vector<std::string> m_options;
    std::function<ArgTypes(const std::string&)> converter;

    Error check_long_argument() {
        if (m_name.starts_with("--")) {
            if (m_name.starts_with("---"))
                return "long argument must be start with two or none `-'";
            m_positional = false;
            m_long_argument = m_name;
            m_name.erase(0, 1);
            m_options.push_back(m_long_argument);
        } else if (m_name.starts_with("-")) {
            return "long argument must be start with two or none `-'";
        } else {
            m_positional = true;
        }
        return std::nullopt;
    }

    Error check_short_argument() {
        if (! m_short_argument.empty())
            if (! m_short_argument.starts_with("-") || m_short_argument.starts_with("--"))
                return "short argument must be start with single `-'";
        m_options.push_back(m_short_argument);
        return std::nullopt;
    }

    bool is_valid_type(const ArgTypes &value) {
        return m_type == ArgTypeIndex[value.index()];
    }

    Error set_converter() {
        if (m_type == ArgTypeIndex[0]) {
            converter = [](const std::string &str) -> ArgTypes {
                return str;
            };
            return std::nullopt;
        } else if (m_type == ArgTypeIndex[1]) {
            converter = [](const std::string &str) -> ArgTypes {
                return std::stoi(str);
            };
            return std::nullopt;
        } else if (m_type == ArgTypeIndex[2]) {
            converter = [](const std::string &str) -> ArgTypes {
                return std::stol(str);
            };
            return std::nullopt;
        } else if (m_type == ArgTypeIndex[3]) {
            converter = [](const std::string &str) -> ArgTypes {
                return std::stoul(str);
            };
            return std::nullopt;
        } else if (m_type == ArgTypeIndex[4]) {
            converter = [](const std::string &str) -> ArgTypes {
                return std::stoll(str);
            };
            return std::nullopt;
        } else if (m_type == ArgTypeIndex[5]) {
            converter = [](const std::string &str) -> ArgTypes {
                return std::stoull(str);
            };
            return std::nullopt;
        } else if (m_type == ArgTypeIndex[6]) {
            converter = [](const std::string &str) -> ArgTypes {
                return std::stof(str);
            };
            return std::nullopt;
        } else if (m_type == ArgTypeIndex[7]) {
            converter = [](const std::string &str) -> ArgTypes {
                return std::stod(str);
            };
            return std::nullopt;
        } else if (m_type == ArgTypeIndex[8]) {
            converter = [](const std::string &str) -> ArgTypes {
                return std::stold(str);
            };
            return std::nullopt;
        } else if (m_type == ArgTypeIndex[9]) {
            converter = [](const std::string &str) -> ArgTypes {
                if (str == "true" || str == "yes" || str == "on" || str == "1")
                    return true;
                else if (str == "false" || str == "no" || str == "off" || str == "0")
                    return false;
                else
                    throw std::invalid_argument("invalid boolean value `" + str + "'");
            };
            return std::nullopt;
        }

        return "unsupported type `" + m_type + "'";
    }

public:
    Argument() = default;
    ~Argument() = default;

    Argument(
        const std::string &type,
        const std::string &name,
        const std::string &short_argument = "",
        const std::string &description = "---",
        const std::optional<ArgTypes> &default_value = std::nullopt
    ) : m_type(type),
        m_name(name),
        m_short_argument(short_argument),
        m_description(description),
        m_default_value(default_value) {}

    Error validation() {
        if (auto error = check_long_argument())
            return error.value();
        
        if (auto error = check_short_argument())
            return error.value();
        
        if (auto error = set_converter())
            return error.value();

        if (m_default_value && ! is_valid_type(m_default_value.value()))
            return "invalid default value type, must be '" + m_type + "'";

        return std::nullopt;
    }

    Error set_value(const std::string &new_value) {
        try {
            m_value = converter(new_value);
        } catch (const std::invalid_argument &ex) {
            return "converter error: " + std::string(ex.what());
        } catch (const std::out_of_range &ex) {
            return "converter error: " + std::string(ex.what());
        }

        return std::nullopt;
    }

    template<typename T>
    std::optional<T> value() const {
        if (! m_value && ! m_default_value)
            return std::nullopt;
        
        try {
            if (m_value)
                return std::get<T>(m_value);
            if (m_default_value)
                return std::get<T>(m_default_value);
        } catch (const std::bad_variant_access&) {
            return std::nullopt;
        }
    }

    bool is_positional() const {
        return m_positional;
    }

    std::string name() const {
        return m_name;
    }

    std::vector<std::string> options() const {
        return m_options;
    }

    std::string type() const {
        return m_type;
    }
}; // class Argument


export class Command {
private:
    std::string m_name;
    std::string m_description;
    std::map<std::string, Argument> m_optional_arguments;
    std::map<std::string, Argument> m_positional_arguments;
    std::map<std::string, Argument>::iterator m_positional_iterator = m_positional_arguments.begin();
    bool m_active = false;

public:
    Command() = default;
    ~Command() = default;

    Command(
        const std::string &name,
        const std::string &description
    ) : m_name(name), m_description(description) {};

    std::string name() const {
        return m_name;
    }

    void activate() {
        m_active = true;
    }

    bool is_active() const {
        return m_active;
    }

    Error add_argument(const Argument &argument, const bool replace = true) {
        if (argument.is_positional()) {
            const auto [it, ok] = m_positional_arguments.insert({argument.name(), argument});
            if (! ok) {
                if (replace)
                    m_positional_arguments.insert_or_assign(argument.name(), argument);
                else
                    return argument.name() + ": already exists";
            }
        } else {
            const auto [it, ok] = m_optional_arguments.insert({argument.name(), argument});
            if (! ok) {
                if (replace)
                    m_optional_arguments.insert_or_assign(argument.name(), argument);
                else
                    return argument.name() + ": already exists";
            }
        }

        return std::nullopt;
    }

    std::optional<Argument> get_argument(const std::string &name) const {
        auto positional_argument = m_positional_arguments.find(name);
        if (positional_argument != m_positional_arguments.end())
            return positional_argument->second;

        auto argument = m_optional_arguments.find(name);
        if (argument != m_optional_arguments.end())
            return argument->second;

        return std::nullopt;
    }

    Error set_optional_argument_value(
        const std::string& argument_value,
        std::list<std::string>& m_arguments_list
    ) {
        for (auto &argument : m_optional_arguments) {
            if (is_in_vector<std::string>(argument.second.options(), argument_value)) {
                if (argument.second.type() == "bool") {
                    argument.second.set_value("true");
                    return std::nullopt;
                } else {
                    if (m_arguments_list.empty() || m_arguments_list.front().starts_with("-"))
                        return argument_value + ": expected argument";                            
                    
                    if (auto error = argument.second.set_value(m_arguments_list.front())) {
                        return argument_value + ": " + error.value();
                    } else {
                        m_arguments_list.pop_front();
                        return std::nullopt;
                    }
                }
            }
        }

        return "unknown argument `" + argument_value + "'";
    }

    Error set_positional_argument_value(const std::string& argument_value) {
        if (m_positional_iterator == m_positional_arguments.end())
            return argument_value + ": unknown argument";

        if (auto error = m_positional_iterator->second.set_value(argument_value))
            return argument_value + ": " + error.value();

        m_positional_iterator++;
        return std::nullopt;
    }

}; // class Command


export class ArgParser {
private:
    bool m_raise_error;
    bool m_print_usage;
    std::string m_application_name;
    std::list<std::string> m_arguments_list;
    Command m_default_command;
    std::map<std::string, Command> m_commands;
    std::string m_active_command_name;

public:
    ArgParser() = default;
    ~ArgParser() = default;

    ArgParser(
        bool raise_error = true,
        bool print_usage = true
    ) : m_raise_error(raise_error),
        m_print_usage(print_usage) 
    {
        m_default_command = Command("default", "this is default command");
    }

    void print_usage();
    std::string usage();

    Error parse(int argc, char** argv) {
        m_application_name = argv[0];
        std::shared_ptr<Command> command {&m_default_command};

        if (argc > 1)
            m_arguments_list.assign(argv+1, argv+argc);

        while (! m_arguments_list.empty()) {
            auto arg_str = m_arguments_list.front();
            m_arguments_list.pop_front();

            if (arg_str.starts_with("-")) {
                if (auto error = command->set_optional_argument_value(arg_str, m_arguments_list))
                    return error;
            } else if (! m_active_command_name.empty()) {
                if (auto error = command->set_positional_argument_value(arg_str))
                    return error;
            } else if (auto search = m_commands.find(arg_str); search != m_commands.end()) {
                command = std::shared_ptr<Command>(&search->second);
                command->activate();
                m_active_command_name = arg_str;
            } else if (auto error = command->set_positional_argument_value(arg_str)) {
                return error;
            }
        }

        return std::nullopt;
    }

    void add_argument(Argument &argument) {
        m_default_command.add_argument(argument);
    }

    void add_command(Command &command) {
        m_commands.insert_or_assign(command.name(), command);
    }

    std::optional<Command> get_active_command() const {
        if (m_active_command_name.empty())
            return std::nullopt;
        return m_commands.at(m_active_command_name);
    }

    std::string get_active_command_name() const {
        return m_active_command_name;
    }

    template <typename T>
    std::optional<T> get(const std::string &argument_name) const {
        if (auto argument = m_default_command.get_argument(argument_name))
            return argument.value().value<T>();
        else
            return std::nullopt;
    }

    template <typename T>
    std::optional<T> get(
        const std::string &command_name,
        const std::string &argument_name
    ) const {
        auto command = m_commands.find(command_name);
        if (command == m_commands.end())
            return std::nullopt;

        if (auto argument = command->second.get_argument(argument_name))
            return argument.value().value<T>();

        return std::nullopt;
    }
}; // class ArgParser

} // namespace argparser

