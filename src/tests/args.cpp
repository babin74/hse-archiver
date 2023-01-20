#include <catch.hpp>

#include "../args.hpp"

TEST_CASE("CLIOption") {
    const auto option1 = CLIOption("create", "create archive file").WithArgument().DefaultValue("a.zip");
    const auto option2 = CLIOption("summary", "get summary size of archives").WithArguments();
    const auto option3 = CLIOption("sum", "get sum of 3 ints").WithArguments(3).ShortName('s');
    const auto option4 = CLIOption("help", "output help").ShortName('h');

    REQUIRE(option1.GetName() == "create");
    REQUIRE(option2.GetName() == "summary");
    REQUIRE(option3.GetName() == "sum");
    REQUIRE(option4.GetName() == "help");

    REQUIRE(option1.GetArgumentCount() == 1);
    REQUIRE(option2.GetArgumentCount() == CLIOption::INDEFINITE_NUMBER_OF_ARGUMENTS);
    REQUIRE(option3.GetArgumentCount() == 3);
    REQUIRE(option4.GetArgumentCount() == 0);

    REQUIRE(option1.GetDescription() == "create archive file");
    REQUIRE(option2.GetDescription() == "get summary size of archives");
    REQUIRE(option3.GetDescription() == "get sum of 3 ints");
    REQUIRE(option4.GetDescription() == "output help");

    REQUIRE(option1.GetDefaultValue() == "a.zip");
    REQUIRE(option2.GetDefaultValue().empty());
    REQUIRE(option3.GetDefaultValue().empty());
    REQUIRE(option4.GetDefaultValue().empty());

    REQUIRE(option1.GetShortName() == CLIOption::NO_SHORT_NAME);
    REQUIRE(option2.GetShortName() == CLIOption::NO_SHORT_NAME);
    REQUIRE(option3.GetShortName() == 's');
    REQUIRE(option4.GetShortName() == 'h');

    const auto option5 = CLIOption("size", "set size limit to archive in KiB").WithArgument().DefaultValue(1 << 20);
    REQUIRE(option5.GetDefaultValue() == std::to_string(1 << 20));
}

TEST_CASE("CLIOption exceptions") {
    REQUIRE_THROWS_AS(CLIOption("isleet", "").DefaultValue(1337), CLIOption::SetDefaultValueException);
    REQUIRE_THROWS_AS(CLIOption("isleet", "").DefaultValue("kek"), CLIOption::SetDefaultValueException);
    REQUIRE_THROWS_AS(CLIOption("isleet", "").DefaultValue(std::string_view("wow")),
                      CLIOption::SetDefaultValueException);
    REQUIRE_THROWS_AS(CLIOption("isleet", "").WithArguments().DefaultValue(1337), CLIOption::SetDefaultValueException);
    REQUIRE_THROWS_AS(CLIOption("isleet", "").WithArguments(2).DefaultValue(1337), CLIOption::SetDefaultValueException);
    REQUIRE_THROWS_AS(CLIOption("a", "").WithArgument().DefaultValue(1337).WithArguments(),
                      CLIOption::SetDefaultValueException);
    REQUIRE_NOTHROW(CLIOption("isleet", "").WithArguments(1).DefaultValue(1337));
    REQUIRE_NOTHROW(CLIOption("isleet", "").WithArgument().DefaultValue(1337));

    REQUIRE_THROWS_AS(CLIOption("", "some beautiful option without name"), CLIOption::InvalidNameException);
    REQUIRE_THROWS_AS(CLIOption("-why", ""), CLIOption::InvalidNameException);
    REQUIRE_THROWS_AS(CLIOption("never gonna give you up", ""), CLIOption::InvalidNameException);
    REQUIRE_THROWS_AS(CLIOption(" param ", ""), CLIOption::InvalidNameException);
    REQUIRE_THROWS_AS(CLIOption("say!", "").DefaultValue("no matter"), CLIOption::InvalidNameException);
    REQUIRE_NOTHROW(CLIOption("norm", ""));
    REQUIRE_NOTHROW(CLIOption("p0z1t1ve", ""));
    REQUIRE_NOTHROW(CLIOption("1337", ""));
    REQUIRE_NOTHROW(CLIOption("HeLLo", ""));

    REQUIRE_THROWS_AS(CLIOption("always", "").ShortName('-'), CLIOption::InvalidShortNameException);
    REQUIRE_THROWS_AS(CLIOption("always", "").ShortName(' '), CLIOption::InvalidShortNameException);
    REQUIRE_THROWS_AS(CLIOption("always", "").ShortName('?'), CLIOption::InvalidShortNameException);
    REQUIRE_NOTHROW(CLIOption("always", "").ShortName('a'));
    REQUIRE_NOTHROW(CLIOption("always", "").ShortName('3'));
    REQUIRE_NOTHROW(CLIOption("always", "").ShortName('Z'));
}

namespace {

std::vector<std::string> GetArguments(std::string_view str) {
    std::vector<std::string> result;

    auto it = str.begin();
    while (it != str.end()) {
        if (std::isspace(*it)) {
            ++it;
            continue;
        }

        auto word_end = std::find(it, str.end(), ' ');
        result.emplace_back(it, word_end);
        it = word_end;
    }

    return result;
}

std::vector<const char*> GetRavArguments(const std::vector<std::string>& args) {
    std::vector<const char*> result;
    for (const auto& s : args) {
        result.push_back(s.c_str());
    }
    return result;
}

}  // namespace

TEST_CASE("CLIArgumentParser exceptions") {
    CLIArgumentParser parser_archiver{
        CLIOption("help", "output help information").ShortName('h'),
        CLIOption("create", "create archive").ShortName('c').WithArgument(),
        CLIOption("unzip", "unzip archive").ShortName('d').WithArgument(),
    };

    {
        auto args = GetArguments("./archiver --help");
        auto args_rav = GetRavArguments(args);
        auto parsed = parser_archiver.Parse(args_rav.size(), args_rav.data());

        REQUIRE(parsed.HasFlag("help"));
        REQUIRE(!parsed.IsDefined("create"));
        REQUIRE(!parsed.IsDefined("unzip"));
    }

    {
        auto args = GetArguments("./archiver -h");
        auto args_rav = GetRavArguments(args);
        auto parsed = parser_archiver.Parse(args_rav.size(), args_rav.data());

        REQUIRE(parsed.HasFlag("help"));
        REQUIRE(!parsed.IsDefined("create"));
        REQUIRE(!parsed.IsDefined("unzip"));
    }

    {
        auto args = GetArguments("./archiver -c legends.zip main.cpp args.hpp args.cpp");
        auto args_rav = GetRavArguments(args);
        auto parsed = parser_archiver.Parse(args_rav.size(), args_rav.data());

        REQUIRE(!parsed.HasFlag("help"));
        REQUIRE(parsed.IsDefined("create"));
        REQUIRE(!parsed.IsDefined("unzip"));

        REQUIRE(parsed.GetValue("create") == "legends.zip");
        REQUIRE(parsed.GetValueArray() == std::vector<std::string>{"main.cpp", "args.hpp", "args.cpp"});
    }
}

TEST_CASE("CLIArgumentParser exceptions constructor") {
    REQUIRE_THROWS_AS(CLIArgumentParser({CLIOption("help", "output help information"), CLIOption("help", "meow")}),
                      CLIArgumentParser::NotUniqueNameExcpetion);

    REQUIRE_THROWS_AS(CLIArgumentParser({CLIOption("help", "output help information").ShortName('h'),
                                         CLIOption("hello", "say hello").ShortName('h')}),
                      CLIArgumentParser::NotUniqueShortNameExcpetion);

    REQUIRE_NOTHROW(CLIArgumentParser({CLIOption("help", "output help information").ShortName('h'),
                                       CLIOption("make", "").ShortName('m').WithArguments(),
                                       CLIOption("output", "").ShortName('o').WithArgument().DefaultValue("a.out")}));
}