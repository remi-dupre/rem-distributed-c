/**
 * Manualy bufferize standart library's buffers.
 * This allows to call much faster little write as iostream::write is too slow.
 */
#pragma once

#include <cstdio>
#include <cassert>
#include <cstring>
#include <iostream>


constexpr int MANUAL_BUFF_SIZE = 32768;


/**
 * Creates a buffer over an output stream.
 * It can be used by calling buffer << value;
 */
class OStreamBuff
{
public:
    /**
     * Create the buffer over the stream.
     */
    OStreamBuff(std::ostream& stream);

    /**
     * Flush into the stream before destroy.
     */
    ~OStreamBuff();

    /**
     * Writes a buffer into this buffer.
     */
    void write(char* data, int data_size);

private:
    // buffer to be outputed
    char buffer[MANUAL_BUFF_SIZE];

    // quantity of datas waiting to be flushed
    int buffer_load;

    // stream data are sent to
    std::ostream& stream;
};

/**
 * Creates a buffer over an input stream.
 * It can be used by calling buffer >> value;
 */
class IStreamBuff
{
public:
    /**
     * Create a buffer over the stream.
     */
    IStreamBuff(std::istream& stream);

    /**
     * Read into a buffer of datas.
     */
    void read(char* data, int data_size);

    /**
     * Check if all the datas have been read.
     */
    bool eof() const;

private:
    // buffer containing datas to read
    char buffer[MANUAL_BUFF_SIZE];

    // position where next datas to read are located
    int buffer_pos;

    // current end of the buffer
    int buffer_load;

    // buffer we read data from
    std::istream& stream;
};

/**
 * Stream operator to read unsigned ints.
 */
OStreamBuff& operator<<(OStreamBuff& buffer, uint32_t x);
IStreamBuff& operator>>(IStreamBuff& buffer, uint32_t& x);
