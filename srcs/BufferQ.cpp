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
	b = new char[size];
	memcpy(b, other.b, static_cast<size_t>(occupiedSize));
}

s_buffer &
s_buffer::operator=(s_buffer const & other)
{
	if (this != &other)
	{
		size = other.size;
		occupiedSize = other.occupiedSize;
		delete[] b;
		b = new char[size];
		memcpy(b, other.b, static_cast<size_t>(occupiedSize));
	}
	return (*this);
}

void deleteQ(t_bufferQ & q)
{
	while (!q.empty())
	{
		delete q.front();
		q.pop();
	}
}
