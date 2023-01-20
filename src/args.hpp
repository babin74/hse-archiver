#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include <vector>
#include <initializer_list>
#include <numeric>

/**
 * @brief Класс, предназначенный для обработки аргументов коммандной строки.
 */
class CLIArgumentParser {
public:
    class Option;
    class ParsedArguments;

    class NotUniqueNameExcpetion : public std::exception {};
    class NotUniqueShortNameExcpetion : public std::exception {};
    class ArgumentParsingException : public std::logic_error {
    public:
        inline ArgumentParsingException(const std::string& message) : std::logic_error(message) {
        }

        inline ArgumentParsingException(const char* message) : std::logic_error(message) {
        }
    };

    CLIArgumentParser() = delete;

    /// @brief Конструктор класса.
    /// @param options_definition Содержит список допустимых аргументов.
    CLIArgumentParser(std::initializer_list<Option> options_definition);

    /// @brief Эта функция обрабатывает конкретный набор аргументов.
    /// @param argc Количество аргументов командной строки
    /// @param argv Сами аргументы командной строки
    /// @return Возвращается
    ParsedArguments Parse(size_t argc, const char* argv[]) const;

    void AddUsageCase(std::string_view usage);

    /// @brief Эта функция выводит вспомогательную информацию для программы
    /// @param os Поок вывода
    void PrintHelpInformation(std::ostream& os) const;

private:
    std::vector<std::string> usage_cases_;
    std::vector<Option> options_;
    std::unordered_map<std::string, const Option&> option_by_name_;
    std::unordered_map<char, const Option&> option_by_short_name_;
};

class CLIArgumentParser::ParsedArguments {
public:
    friend CLIArgumentParser;

    bool HasFlag(std::string_view name) const;
    bool IsDefined(std::string_view name) const;
    const std::string& GetValue(std::string_view name) const;
    const std::vector<std::string>& GetValueArray(std::string_view name) const;
    const std::vector<std::string>& GetValueArray() const;

private:
    ParsedArguments();

    std::unordered_set<std::string> flags_;
    std::unordered_map<std::string, std::vector<std::string>> values_;
    std::vector<std::string> free_arguments_;
};

/**
 * @brief Класс, который используется в определении конфигурации CLIArgumentParser, содержит в себе
 * информацию о конкретном аргументе. А именно его название, его сокращенное название и количество
 * аргументов, которые он принимает, а также описание аргумента.
 */
class CLIArgumentParser::Option {
public:
    class SetDefaultValueException : public std::exception {};
    class InvalidNameException : public std::exception {};
    class InvalidShortNameException : public std::exception {};

    static constexpr size_t INDEFINITE_NUMBER_OF_ARGUMENTS = std::numeric_limits<size_t>::max();
    static constexpr char NO_SHORT_NAME = '\0';

    Option() = delete;
    /// @brief Конструктор класса.
    /// @param name название аргумента, в командной строке арумент будет парситься как --name
    /// @param description описание аргумента, должно содержать короткую информацию о применении
    /// аргумента и о том, на что он влияет.
    Option(std::string_view name, std::string_view description);

    /// @brief Дать информацию о том, что опция имеет один аргумент.
    /// Просто делает вызов WithArgument(1)
    Option& WithArgument();

    /// @brief Дать информацию о том, что опция имеет несколько аргументов.
    /// @param count количество аргументов у опции
    Option& WithArguments(size_t count = INDEFINITE_NUMBER_OF_ARGUMENTS);

    /// @brief Установить стандартное значение опции (перегрузка для std::string_view)
    /// @param default_value аргумент опции по умолчанию
    Option& DefaultValue(std::string_view default_value);

    /// @brief Установить стандартное значение опции (перегрузка для const char*)
    /// @param default_value аргумент опции по умолчанию
    Option& DefaultValue(const char* default_value);

    /// @brief Установить стандартное значение опции
    /// @param default_value аргумент опции по умолчанию
    template <typename Type>
    Option& DefaultValue(const Type& default_value) {
        CheckApprovalDefaultValue();
        default_value_ = std::to_string(default_value);
        return *this;
    }

    /// @brief Указать сокращение опации
    /// @param short_name сокращенное название
    Option& ShortName(char short_name);

    const std::string& GetName() const;
    const std::string& GetDescription() const;
    const std::string& GetDefaultValue() const;
    size_t GetArgumentCount() const;
    char GetShortName() const;

    bool HasDefaultValue() const;
    bool HasShortName() const;
    bool IsFlag() const;
    bool HasIndefiniteArgumentCount() const;

private:
    void CheckApprovalDefaultValue() const;

    std::string name_;
    std::string description_;
    size_t argument_count_;
    std::string default_value_;
    char short_name_;
};

using CLIOption = CLIArgumentParser::Option;
using CLIParsedArguments = CLIArgumentParser::ParsedArguments;