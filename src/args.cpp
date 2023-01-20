#include "args.hpp"

#include <cctype>
#include <stdexcept>
#include <algorithm>
#include <sstream>

CLIArgumentParser::CLIArgumentParser(std::initializer_list<Option> options_definition) : options_(options_definition) {
    for (const Option& option : options_) {
        bool had_name_inserted = option_by_name_.try_emplace(option.GetName(), std::cref(option)).second;
        if (!had_name_inserted) {
            throw NotUniqueNameExcpetion();
        }

        if (option.GetShortName() != Option::NO_SHORT_NAME) {
            bool had_short_name_inserted =
                option_by_short_name_.try_emplace(option.GetShortName(), std::cref(option)).second;
            if (!had_short_name_inserted) {
                throw NotUniqueShortNameExcpetion();
            }
        }
    }
}

CLIParsedArguments CLIArgumentParser::Parse(size_t argc, const char* argv[]) const {
    ParsedArguments parsed;
    size_t i = 1;
    std::unordered_set<const Option*> defined_options;

    auto process_option = [&](const Option& option) {
        bool inserted = defined_options.insert(&option).second;
        if (!inserted) {
            std::stringstream fmt;
            fmt << "Redefinition of " << option.GetName();
            if (option.HasShortName()) {
                fmt << "(aka " << option.GetShortName() << ")";
            }
            fmt << ". Notice that each option can be specified no more than once.";
            throw ArgumentParsingException(fmt.str());
        }
    };

    auto process_option_equal_form = [&](const Option& option, std::string_view value) {
        if (option.GetArgumentCount() != 1) {
            throw ArgumentParsingException("The equal sign is allowed only for options with one argument.");
        }

        if (value.empty()) {
            throw ArgumentParsingException("It is not allowed to set an empty option value");
        }

        parsed.values_[option.GetName()] = std::vector{std::string(value)};
    };

    auto process_option_by_short_name = [&](char short_name) -> const Option& {
        const auto option_iterator = option_by_short_name_.find(short_name);
        if (option_iterator == option_by_short_name_.end()) {
            std::stringstream fmt;
            fmt << "A non-existent '" << short_name << "' option is specified.";
            throw ArgumentParsingException(fmt.str());
        }

        process_option(option_iterator->second);
        return std::cref(option_iterator->second);
    };

    auto process_option_arguments = [&](const Option& option) {
        std::vector<std::string> arguments;
        while (i != argc && arguments.size() < option.GetArgumentCount()) {
            std::string_view argument = argv[i];
            if (argument[0] == '-') {
                break;
            }

            ++i;
            arguments.emplace_back(argument);
        }

        if (!option.HasIndefiniteArgumentCount() && arguments.size() != option.GetArgumentCount()) {
            std::stringstream fmt;
            fmt << "The option --" << option.GetName() << " is specified with the wrong number of arguments.";
            throw ArgumentParsingException(fmt.str());
        }

        parsed.values_[option.GetName()] = std::move(arguments);
    };

    while (i != argc) {
        std::string_view lexem = argv[i++];
        if (lexem.empty()) {
            continue;
        }

        if (lexem.front() == '-') {
            if (lexem.size() == 1) {
                throw ArgumentParsingException("A set of flags was expected after the hyphen.");
            }

            auto it = std::find(lexem.begin(), lexem.end(), '=');

            if (lexem[1] == '-') {
                if (lexem.size() == 2) {
                    continue;
                }

                std::string option_name(lexem.begin() + 2, it);  // TODO: Make option_name std::string_view
                const auto option_iterator = option_by_name_.find(option_name);
                if (option_iterator == option_by_name_.end()) {
                    std::stringstream fmt;
                    fmt << "A non-existent '" << option_name << "' option is specified.";
                    throw ArgumentParsingException(fmt.str());
                }

                const Option& option = option_iterator->second;
                process_option(option);

                if (it != lexem.end()) {
                    process_option_equal_form(option, std::string_view(std::next(it), lexem.end()));
                } else if (option.IsFlag()) {
                    parsed.flags_.insert(option.GetName());
                } else {
                    process_option_arguments(option);
                }
            } else {
                if (it != lexem.end()) {
                    if (it != lexem.begin() + 2) {
                        std::stringstream fmt;
                        fmt << "Expected -<shortname>=..., but got " << lexem << ".";
                        throw ArgumentParsingException(fmt.str());
                    }

                    const Option& option = process_option_by_short_name(lexem[1]);
                    process_option_equal_form(option, std::string_view(std::next(it), lexem.end()));
                } else {
                    std::string_view options_shortnames_list(lexem.begin() + 1, lexem.end());

                    for (auto shortname : options_shortnames_list) {
                        const Option& option = process_option_by_short_name(shortname);
                        if (!option.IsFlag() && options_shortnames_list.size() == 1) {
                            process_option_arguments(option);
                            break;
                        }

                        if (!option.IsFlag()) {
                            std::stringstream fmt;
                            fmt << "When you use style like this -<O1><O2><...>, all options in this list should be "
                                   "flags.";
                            throw ArgumentParsingException(fmt.str());
                        }
                        parsed.flags_.insert(option.GetName());
                    }
                }
            }
        } else {
            parsed.free_arguments_.emplace_back(lexem);
        }
    }

    return parsed;
}

void CLIArgumentParser::AddUsageCase(std::string_view usage) {
    usage_cases_.emplace_back(usage);
}

void CLIArgumentParser::PrintHelpInformation(std::ostream& os) const {
    if (!usage_cases_.empty()) {
        os << "Usage: ";
        if (usage_cases_.size() != 1) {
            os << std::endl;
        }

        for (const auto& usage : usage_cases_) {
            os << "  " << usage << std::endl;
        }
    }

    os << "Options:" << std::endl;
    for (const auto& option : options_) {
        std::stringstream line_buffer;

        line_buffer << "--" << option.GetName();
        if (option.HasShortName()) {
            line_buffer << "(aka -" << option.GetShortName() << ")";
        }

        if (!option.IsFlag()) {
            if (option.HasIndefiniteArgumentCount()) {
                line_buffer << "[..]";
            } else {
                line_buffer << "[" << option.GetArgumentCount() << "]";
            }
        }

        const auto line = line_buffer.str();

        os << line;
        for (size_t i = line.size(); i < 22; ++i) {
            os << ' ';
        }
        os << ' ' << option.GetDescription() << std::endl;
    }
}

CLIParsedArguments::ParsedArguments() {
}

bool CLIParsedArguments::HasFlag(std::string_view name) const {
    return flags_.count(std::string(name));
}

bool CLIParsedArguments::IsDefined(std::string_view name) const {
    return values_.count(std::string(name));
}

const std::string& CLIParsedArguments::GetValue(std::string_view name) const {
    return values_.at(std::string(name))[0];
}

const std::vector<std::string>& CLIParsedArguments::GetValueArray(std::string_view name) const {
    return values_.at(std::string(name));
}

const std::vector<std::string>& CLIParsedArguments::GetValueArray() const {
    return free_arguments_;
}

CLIOption::Option(std::string_view name, std::string_view description)
    : name_(name), description_(description), argument_count_(0), default_value_(), short_name_(NO_SHORT_NAME) {
    if (name_.empty()) {
        throw InvalidNameException();
    }

    for (char ch : name_) {
        if (!std::isalnum(ch)) {
            throw InvalidNameException();
        }
    }
}

CLIOption& CLIOption::WithArgument() {
    argument_count_ = 1;
    return *this;
}

CLIOption& CLIOption::WithArguments(size_t argument_count) {
    if (argument_count != 1 && HasDefaultValue()) {
        throw SetDefaultValueException();
    }

    argument_count_ = argument_count;
    return *this;
}

CLIOption& CLIOption::DefaultValue(std::string_view default_value) {
    CheckApprovalDefaultValue();
    default_value_ = std::string(default_value);
    return *this;
}

CLIOption& CLIOption::DefaultValue(const char* default_value) {
    CheckApprovalDefaultValue();
    default_value_ = std::string(default_value);
    return *this;
}

CLIOption& CLIOption::ShortName(char short_name) {
    if (!std::isalnum(short_name) && short_name != NO_SHORT_NAME) {
        throw InvalidShortNameException();
    }

    short_name_ = short_name;
    return *this;
}

const std::string& CLIOption::GetName() const {
    return name_;
}

const std::string& CLIOption::GetDescription() const {
    return description_;
}

const std::string& CLIOption::GetDefaultValue() const {
    return default_value_;
}

size_t CLIOption::GetArgumentCount() const {
    return argument_count_;
}

char CLIOption::GetShortName() const {
    return short_name_;
}

bool CLIOption::HasDefaultValue() const {
    return !default_value_.empty();
}

bool CLIOption::IsFlag() const {
    return argument_count_ == 0;
}

void CLIOption::CheckApprovalDefaultValue() const {
    if (argument_count_ != 1) {
        throw SetDefaultValueException();
    }
}

bool CLIOption::HasShortName() const {
    return short_name_ != NO_SHORT_NAME;
}

bool CLIOption::HasIndefiniteArgumentCount() const {
    return argument_count_ == INDEFINITE_NUMBER_OF_ARGUMENTS;
}