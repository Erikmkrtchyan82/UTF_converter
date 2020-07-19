#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <iterator>

using std::vector;
using std::string;
using std::ifstream;
using std::ofstream;

using UTF_8_type = unsigned char;
using UTF_16_type = char16_t;
using UTF_32_type = char32_t;

//	To find out UTF formats
enum class UTF {
	//	If encoding format is invalid
	UTF_ERROR = 0,

	UTF_8, UTF_16, UTF_32
};

UTF find_utf( const string& );

//	Throws exception
//	If everything is okay does nothing
void check_invalid_inputs( char* [], ifstream&, ofstream& );

void convert( UTF, UTF, ifstream&, ofstream& );

template<class T>
vector<T> read_from_file( ifstream& input_file ) {

	vector<T> vector;
	T c;

	while ( input_file.read( (char*)&c, sizeof( c ) ) ) {
		vector.emplace_back( c );
	}
	return vector;
}

template<typename OutputIterator>
void encode_utf_8( vector<unsigned int>::iterator begin, vector<unsigned int>::iterator end, OutputIterator& out ) {

	for ( ; begin != end; ++begin, ++out ) {
		//	Needs one byte to encode
		if ( *begin <= 0x7F ) {
			*out = *begin;
		}
		//	Needs two bytes to encode
		else if ( *begin <= 0x7FF ) {
			auto first = ( ( ( *begin >> 6 ) & 0x1F ) | 0xC0 );
			auto second = ( *begin & 0x3F ) | 0x80;

			*out = first;
			*++out = second;
		}
		//	Needs three bytes to encode
		else if ( *begin <= 0x7FFF ) {
			auto first = ( ( ( *begin >> 8 >> 4 ) & 0xF ) | 0xE0 );
			auto second = ( ( ( *begin >> 6 ) & 0x3F ) | 0x80 );
			auto third = ( *begin & 0x3F ) | 0x80;

			*out = first;
			*++out = second;
			*++out = third;
		}
		//	Needs four bytes to encode
		else if ( *begin <= 0xFFFF ) {
			auto first = ( ( ( *begin >> 16 >> 2 ) & 0x7 ) | 0x1E );
			auto second = ( ( ( *begin >> 8 >> 4 ) & 0x3F ) | 0x80 );
			auto third = ( ( ( *begin >> 6 ) & 0x3F ) | 0x80 );
			auto forth = ( *begin & 0x3F ) | 0x80;

			*out = first;
			*++out = second;
			*++out = third;
			*++out = forth;
		}
	}
}

//	Throws exception
template<typename OutputIterator>
void encode_utf_16( vector<unsigned int>::iterator begin, vector<unsigned int>::iterator end, OutputIterator& out ) {

	for ( ; begin != end; ++begin, ++out ) {
		if ( *begin >= 0xD800 && *begin <= 0xDFFF ) {
			string error_message = "Can't convert ";
			error_message += *begin + " to UTF-16\n";

			throw std::invalid_argument( error_message );
		}
		//	Needs four bytes to encode
		if ( *begin >= 0x010000 && *begin <= 0x10FFFF )
		{
			auto copy = *begin - 0x10000;
			auto high = ( copy >> 10 ) + 0xD800;
			auto low = ( copy & 0x3FF ) + 0xDC00;

			*out = high;
			*++out = low;
		}
		//	Needs two bytes to encode
		else {
			*out = *begin;
		}
	}
}

template<typename OutputIterator>
void encode_utf_32( vector<unsigned int>::iterator begin, vector<unsigned int>::iterator end, OutputIterator& out ) {
	for ( ; begin != end; ++begin, ++out ) {
		*out = *begin;
	}
}

template<typename InputIterator>
void write_in_file( InputIterator begin, InputIterator end, ofstream& output_file ) {
	for ( ; begin != end; ++begin ) {
		if ( *begin != 0 )
			output_file.write( (char*)( &*begin ), sizeof( typename std::iterator_traits<InputIterator>::value_type ) );
	}
}

template <typename InputIterator, typename OutputIterator>
void decode_utf_8( InputIterator begin, InputIterator end, OutputIterator out ) {
	
	string error_message = "Input file is not in UTF-8 format\n";

	//	Checks, if leading bits, after first byte, is 10
	auto is_valid_utf_8 = []( const UTF_8_type& c ) {return ( ( c >> 6 ) == 0x2 ); };

	while ( begin != end ) {
		//	If the number of significant bits is no more than seven, 
		//	it means that character was encoded in one byte
		if ( ( *begin >> 7 ) == 0x00 ) {
			*out = *begin;
		}
		//	If characer's leading three bits are 110,
		//	means that character was encoded in two bytes
		else if ( ( *begin >> 5 ) == 0x6 ) {
			auto first = ( *begin ^ 0xC0 ) << 8;

			++begin;

			if ( !is_valid_utf_8( *begin ) ) {
				throw std::invalid_argument( error_message );
			}

			auto second = ( *begin ^ 0x80 ) << 2;
			auto all = ( first | second ) >> 2;

			*out = all;
		}
		//	If characer's leading four bits are 1110,
		//	means that character was encoded in three bytes 
		else if ( ( *begin >> 4 ) == 0xE ) {
			UTF_8_type point[ 2 ];

			point[ 0 ] = *++begin;
			point[ 1 ] = *++begin;

			if ( !is_valid_utf_8( point[ 0 ] ) || !is_valid_utf_8( point[ 1 ] ) ) {
				throw std::invalid_argument( error_message );
			}

			auto first = ( *begin ^ 0xE0 ) << 16;
			auto second = ( point[ 0 ] ^ 0x80 ) << 2 << 8;
			auto third = ( point[ 1 ] ^ 0x80 ) << 2;
			auto all = ( ( ( first | second ) >> 2 ) | third ) >> 2;

			*out = all;
		}
		//	If characer's leading five bits are 11110,
		//	means that character was encoded in four bytes
		else if ( ( *begin >> 3 ) == 0x1E ) {
			UTF_8_type point[ 3 ];

			point[ 0 ] = *++begin;
			point[ 1 ] = *++begin;
			point[ 2 ] = *++begin;

			if ( !is_valid_utf_8( point[ 0 ] ) ||
				!is_valid_utf_8( point[ 1 ] ) ||
				!is_valid_utf_8( point[ 2 ] ) ) {
				throw std::invalid_argument( error_message );
			}

			auto first = ( *begin ^ 0xF0 ) << 16 << 8;
			auto second = ( point[ 0 ] ^ 0x80 ) << 2 << 16;
			auto third = ( point[ 1 ] ^ 0x80 ) << 2 << 8;
			auto forth = ( point[ 2 ] ^ 0x80 ) << 2;
			auto all = ( ( ( ( ( first | second ) >> 2 ) | third ) >> 2 ) | forth ) >> 2;

			*out = all;
		}
		else {
			throw std::invalid_argument( error_message );
		}
		++begin;
		out++;
	}
}

template <typename InputIterator, typename OutputIterator>
void decode_utf_16( InputIterator begin, InputIterator end, OutputIterator out ) {

	string error_message = "Input file is not in UTF-16 format\n";

	//	Checks, if bytes, after first byte, starts with 110111(b)
	auto is_valid_utf_16 = []( const UTF_16_type& c ) {return ( ( c >> 10 ) == 0x37 ); };

	while ( begin != end ) {
		//	If first byte is'n start with 1101_10..
		if ( ( *begin & 0xD800 ) != 0xD800 ) {
			//	and if leading bit is 1
			//	means that this character is not valid UTF-16 format
			if ( ( *begin >> 15 ) == 0x1 ) {
				throw std::invalid_argument( error_message );
			}
			else {
				*out = *begin;
			}
		}
		else {
			auto high = *begin - 0xD800;

			++begin;

			if ( !is_valid_utf_16( *begin ) ) {
				throw std::invalid_argument( error_message );
			}

			auto low = *begin - 0xDC00;
			auto all = ( ( high << 10 ) | low ) + 0x10000;

			*out = all;
		}
		++begin;
		out++;
	}

}

template <typename InputIterator, typename OutputIterator>
void decode_utf_32( InputIterator begin, InputIterator end, OutputIterator out ) {
	string error_message = "Input file is not in UTF-32 format\n";

	while ( begin != end ) {
		if ( ( *begin >> 16 >> 5 ) != 0 )
			throw std::invalid_argument( error_message );
		*out = *begin;
		++begin;
		out++;
	}
}

//	Throws exception
template <typename InputIterator, typename OutputIterator>
void utf_convert( InputIterator begin, InputIterator end, OutputIterator out ) {

	vector<unsigned int> code_points;
	std::back_insert_iterator <vector<unsigned int>> it( code_points );

	// Decoding
	if ( typeid( typename std::iterator_traits<InputIterator>::value_type ) == typeid( UTF_8_type ) ) {
		decode_utf_8( begin, end, it );
	}
	else if ( typeid( typename std::iterator_traits<InputIterator>::value_type ) == typeid( UTF_16_type ) ) {
		decode_utf_16( begin, end, it );
	}
	else if ( typeid( typename std::iterator_traits<InputIterator>::value_type ) == typeid( UTF_32_type ) ) {
		decode_utf_32( begin, end, it );
	}

	// Encoding
	if ( typeid( typename std::iterator_traits<OutputIterator>::value_type ) == typeid( UTF_8_type ) ) {
		encode_utf_8( code_points.begin(), code_points.end(), out );
		return;
	}
	if ( typeid( typename std::iterator_traits<OutputIterator>::value_type ) == typeid( UTF_16_type ) ) {
		encode_utf_16( code_points.begin(), code_points.end(), out );
		return;
	}
	if ( typeid( typename std::iterator_traits<OutputIterator>::value_type ) == typeid( UTF_32_type ) ) {
		encode_utf_32( code_points.begin(), code_points.end(), out );
		return;
	}
}

template<class T>
void find_target_and_convert( vector<T>& input, UTF target, ofstream& ofs ) {

	if ( target == UTF::UTF_8 ) {
		vector<UTF_8_type> target( 4 * input.size() );
		utf_convert( input.begin(), input.end(), target.begin() );
		write_in_file( target.begin(), target.end(), ofs );
		return;
	}
	if ( target == UTF::UTF_16 ) {
		vector<UTF_16_type> target( 4 * input.size() );
		utf_convert( input.begin(), input.end(), target.begin() );
		write_in_file( target.begin(), target.end(), ofs );
		return;
	}
	if ( target == UTF::UTF_32 ) {
		vector<UTF_32_type> target( 4 * input.size() );
		utf_convert( input.begin(), input.end(), target.begin() );
		write_in_file( target.begin(), target.end(), ofs );
		return;
	}
}

