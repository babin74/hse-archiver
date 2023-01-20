#include <catch.hpp>

#include "../core.hpp"
#include "../encode.hpp"
#include "../decode.hpp"
#include "../bitstream_writer.hpp"
#include "../bitstream_reader.hpp"

#include <sstream>
#include <memory>

TEST_CASE("BitStreamWriter") {
    BitWriterString bws;
    bws.WriteInt(257, 9);
    bws.WriteInt(259, 9);
    bws.Close();
    REQUIRE(bws.Data() == "100000001100000011");

    BitWriterU8 bwu8;
    bwu8.WriteInt(257, 9);
    bwu8.WriteInt(259, 9);
    bwu8.Close();
    REQUIRE(bwu8.Data() == std::vector<uint8_t>{128, 192, 192});
}

TEST_CASE("BitStreamReader") {
    BitReaderU8 bru8(std::vector<uint8_t>{128, 192, 192});

    size_t holder = 0;
    REQUIRE(bru8.ReadInt(holder, 9));
    REQUIRE(holder == 257);
    REQUIRE(bru8.ReadInt(holder, 9));
    REQUIRE(holder == 259);
    REQUIRE(!bru8.ReadInt(holder, 9));
}

TEST_CASE("ArchiveEncoder") {
    BitWriterU8 writer;
    ArchiveEncoder encoder(writer);

    auto input = std::make_unique<std::istringstream>("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    encoder.Encode("a", std::move(input));
    encoder.Close();

    std::vector<uint8_t> expected{0x02, 0x18, 0x60, 0x50, 0x08, 0x08, 0x04, 0x02,
                                  0x02, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80};

    REQUIRE(writer.Data() == expected);
}

TEST_CASE("ArchiveDecoder") {
    std::vector<uint8_t> archive{0x02, 0x18, 0x60, 0x50, 0x08, 0x08, 0x04, 0x02,
                                 0x02, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80};
    BitReaderU8 reader(std::move(archive));
    ArchiveDecoder decoder(reader);

    std::stringstream output;
    auto filename = decoder.Decode(output);

    REQUIRE(filename == "a");
    REQUIRE(output.str() == "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    REQUIRE(decoder.Done());
}