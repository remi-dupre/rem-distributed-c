#include "streambuff.hpp"


OStreamBuff::OStreamBuff(std::ostream& stream) :
    buffer_load(0),
    stream(stream)
{}

OStreamBuff::~OStreamBuff()
{
    flush();
}

void OStreamBuff::write(char* data, int data_size)
{
    if (buffer_load + data_size < MANUAL_BUFF_SIZE) {
        memcpy(buffer + buffer_load, data, data_size);
        buffer_load += data_size;
    }
    else {
        int part_size = MANUAL_BUFF_SIZE - buffer_load;
        memcpy(buffer + buffer_load, data, part_size);

        stream.write(buffer, MANUAL_BUFF_SIZE);
        buffer_load = 0;
        write(data + part_size, data_size - part_size);
    }
}

void OStreamBuff::flush()
{
    stream.write(buffer, buffer_load);
    buffer_load = 0;
}

IStreamBuff::IStreamBuff(std::istream& stream) :
    buffer_pos(0),
    buffer_load(0),
    stream(stream)
{}

void IStreamBuff::read(char* data, int data_size)
{
    if (eof())
        return;

    if (buffer_pos + data_size <= buffer_load) {
        memcpy(data, buffer + buffer_pos, data_size);
        buffer_pos += data_size;
    }
    else {
        int part_size = buffer_load - buffer_pos;
        memcpy(data, buffer + buffer_pos, part_size);

        stream.read(buffer, MANUAL_BUFF_SIZE);
        buffer_load = stream.gcount();
        buffer_pos = 0;

        read(data + part_size, data_size - part_size);
    }
}

bool IStreamBuff::eof() const
{
    return buffer_pos == buffer_load && stream.eof();
}

OStreamBuff& operator<<(OStreamBuff& buffer, uint32_t x)
{
    buffer.write(reinterpret_cast<char*>(&x), sizeof(uint32_t));
    return buffer;
}

IStreamBuff& operator>>(IStreamBuff& buffer, uint32_t& x)
{
    buffer.read(reinterpret_cast<char*>(&x), sizeof(uint32_t));
    return buffer;
}
