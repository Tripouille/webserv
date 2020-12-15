#include "BufferQ.hpp"

s_buffer::s_buffer(streamsize s) : b(new char[s]), size(s), occupiedSize(0)
{
}

s_buffer::~s_buffer(void)
{
	delete[] b;
}

s_buffer::s_buffer(s_buffer const & other) : size(other.size), occupiedSize(other.occupiedSize)
{
	b = new char [size];
}

s_buffer &
s_buffer::operator=(s_buffer const & other)
{
	static_cast<void>(other);
	return (*this);
}