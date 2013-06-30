/*
Copyright (c) 2013 Aerys

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#include "minko/Common.hpp"

namespace minko
{
	namespace render
	{
		namespace stream
		{
			class VertexAttribute
			{
			public:
				typedef std::shared_ptr<VertexAttribute> Ptr;

			private:
				const std::string	_name;
				const unsigned int	_size;
				const unsigned int	_offset;

			public:
				inline static
				Ptr
				create(const std::string& name, unsigned int size, unsigned int offset)
				{
					return std::shared_ptr<VertexAttribute>(new VertexAttribute(name, size, offset));
				}

				inline
				const std::string&
				name()
				{
					return _name;
				}

				inline
				const unsigned int
				size()
				{
					return _size;
				}

				inline
				const unsigned int
				offset()
				{
					return _offset;
				}

			private:
				VertexAttribute(const std::string& name, const unsigned int size, const unsigned int offset) :
					_name(name),
					_size(size),
					_offset(offset)
				{
				}
			};
		}
	}
}
