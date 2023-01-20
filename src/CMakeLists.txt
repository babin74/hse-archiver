add_executable(
        archiver
        archiver.cpp
        args.cpp
        encode.cpp
        decode.cpp
        bitstream_writer.cpp
        bitstream_reader.cpp
)

add_catch(test_archiver_args
        tests/args.cpp
        args.cpp
)

add_catch(test_archiver_queue
        tests/priority_queue.cpp
)

add_catch(test_archiver_forest
        tests/binary_forest.cpp
)

add_catch(test_archiver_bitstream
        tests/bitstream.cpp
        encode.cpp
        decode.cpp
        bitstream_writer.cpp
        bitstream_reader.cpp
)

add_custom_target(
        test_archive_units
        DEPENDS test_archiver_args test_archiver_queue test_archiver_forest test_archiver_bitstream
        COMMAND test_archiver_args
        COMMAND test_archiver_queue
        COMMAND test_archiver_forest
        COMMAND test_archiver_bitstream
)