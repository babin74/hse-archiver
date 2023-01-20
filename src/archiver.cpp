#include "args.hpp"
#include "encode.hpp"
#include "decode.hpp"

#include <iostream>
#include <memory>
#include <fstream>

void ProcessCreateArchiveCommand(const CLIParsedArguments& parsed_arguments) {
    const auto& archive_name = parsed_arguments.GetValue("create");
    const auto& files = parsed_arguments.GetValueArray();
    if (files.empty()) {
        throw CLIArgumentParser::ArgumentParsingException("Files for archiving are not specified.");
    }

    std::cerr << "Creating archive " << archive_name << "..." << std::endl;

    auto archive_stream = std::make_unique<std::ofstream>();
    try {
        archive_stream->exceptions(std::ofstream::failbit | std::ofstream::badbit);
        archive_stream->open(archive_name, std::ios::binary);
        BitWriterStream bitstream(std::move(archive_stream));

        ArchiveEncoder encoder(bitstream);
        for (const auto& filename : files) {
            std::cerr << "Archiving " << filename << "..." << std::endl;
            encoder.EncodeFile(filename);
        }

        encoder.Close();

        std::cerr << "Done!" << std::endl;
    } catch (const std::ios_base::failure& exception) {
        std::cerr << "A file system error has occurred: " << exception.what() << std::endl;
        std::exit(222);
    }
}

void ProcessUnzipArchiveCommand(const CLIParsedArguments& parsed_arguments) {
    const auto& archive_name = parsed_arguments.GetValue("unzip");
    std::cerr << "Unzipping archive " << archive_name << "..." << std::endl;

    auto archive_stream = std::make_unique<std::ifstream>();
    try {
        archive_stream->exceptions(std::ofstream::badbit);
        archive_stream->open(archive_name, std::ios::binary);
        BitReaderStream bitstream(std::move(archive_stream));

        ArchiveDecoder decoder(bitstream);
        while (!decoder.Done()) {
            auto decoded_file = decoder.DecodeFile();
            std::cerr << "Decoded " << decoded_file << "." << std::endl;
        }

        std::cerr << "Done!" << std::endl;
    } catch (const std::ios_base::failure& exception) {
        std::cerr << "A file system error has occurred: " << exception.what() << std::endl;
        std::exit(111);
    } catch (const ArchiveDecoder::ProcessError& exception) {
        std::cerr << "A problem with " << archive_name << " has occured: " << exception.what() << std::endl;
        std::exit(111);
    }
}

int main(int argc, const char* argv[]) {
    CLIArgumentParser parser_archiver{
        CLIOption("help", "output help information").ShortName('h'),
        CLIOption("create", "create archive").ShortName('c').WithArgument(),
        CLIOption("unzip", "unzip archive").ShortName('d').WithArgument(),
    };

    parser_archiver.AddUsageCase("archiver -h");
    parser_archiver.AddUsageCase("archiver -c <archive> <file...>");
    parser_archiver.AddUsageCase("archiver -d <archive>");

    try {
        auto parsed_arguments = parser_archiver.Parse(argc, argv);

        if (parsed_arguments.IsDefined("create") && parsed_arguments.IsDefined("unzip")) {
            using ParsingException = CLIArgumentParser::ArgumentParsingException;
            throw ParsingException("Options --create and --unzip cannot be mentioned in a single program call.");
        }

        if (parsed_arguments.HasFlag("help")) {
            parser_archiver.PrintHelpInformation(std::cout);
        } else if (parsed_arguments.IsDefined("create")) {
            ProcessCreateArchiveCommand(parsed_arguments);
        } else if (parsed_arguments.IsDefined("unzip")) {
            ProcessUnzipArchiveCommand(parsed_arguments);
        } else {
            throw CLIArgumentParser::ArgumentParsingException("No operation specified");
        }
    } catch (const CLIArgumentParser::ArgumentParsingException& exception) {
        std::cerr << exception.what() << std::endl;
        parser_archiver.PrintHelpInformation(std::cerr);
        std::exit(333);
    }

    return 0;
}
