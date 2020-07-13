#include "utf.h"
#include <vector>
#include <string>

UTF find_utf( const string& encoding ) {

	string lowercase;
	for ( auto character : encoding ) {
		lowercase.push_back( tolower( character ) );
	}

	if ( !lowercase.compare( "utf-8" ) )
		return UTF::UTF_8;
	if ( !lowercase.compare( "utf-16" ) )
		return UTF::UTF_16;
	if ( !lowercase.compare( "utf-32" ) )
		return UTF::UTF_32;

	return UTF::UTF_ERROR;
}

void check_invalid_inputs( char* argv[], ifstream& input_file, ofstream& output_file ) {

	//	For all error messages
	string error_message( "" );

	auto source = find_utf( argv[ 1 ] );
	auto target = find_utf( argv[ 3 ] );


	if ( source == UTF::UTF_ERROR ) {
		error_message += "* Invalid source encoding: " + (string)argv[ 1 ] + "\n\n";
	}

	if ( target == UTF::UTF_ERROR ) {
		error_message += "* Invalid target encoding: " + (string)argv[ 3 ] + "\n\n";
	}

	if ( !input_file.is_open() ) {
		error_message += "* Failed to open " + (string)argv[ 2 ] + " for reading" + "\n\n";
	}

	if ( !output_file.is_open() ) {
		error_message += "* Failed to open " + (string)argv[ 4 ] + " for writing" + "\n\n";
	}

	//	Meaningless to convert to the same encoding
	if ( source == target && source != UTF::UTF_ERROR ) {
		error_message += "* Source encoding and target encoding are equivalent\n\tSource encoding: "
			+ (string)argv[ 1 ] + "\n\tTarget encoding: " + argv[ 3 ] + "\n\n";
	}

	if ( error_message.size() )
		throw error_message;
}

void decode_utf_8( vector<unsigned int>& text, ifstream& input_file ) {

	UTF_8_type character;
	auto byte_count = sizeof( character );
	string error_message = "Input file is not in UTF-8 format\n";

	//	Checks, if leading bits, after first byte, is 10
	auto is_valid_utf_8 = []( const UTF_8_type& c ) {return ( ( c >> 6 ) == 0x2 ); };

	while ( input_file.read( (char*)&character, byte_count ) ) {
		//	If the number of significant bits is no more than seven, 
		//	it means that character was encoded in one byte
		if ( ( character >> 7 ) == 0x00 ) {
			text.push_back( character );
		}
		//	If characer's leading three bits are 110,
		//	means that character was encoded in two bytes
		else if ( ( character >> 5 ) == 0x6 ) {
			auto first = ( character ^ 0xC0 ) << 8;

			input_file.read( (char*)&character, byte_count );

			if ( !is_valid_utf_8( character ) ) {
				throw error_message;
			}

			auto second = ( character ^ 0x80 ) << 2;
			auto all = ( first | second ) >> 2;

			text.push_back( all );
		}
		//	If characer's leading four bits are 1110,
		//	means that character was encoded in three bytes 
		else if ( ( character >> 4 ) == 0xE ) {
			UTF_8_type point[ 2 ];

			input_file.read( (char*)point, byte_count );
			input_file.read( (char*)( point + 1 ), byte_count );

			if ( !is_valid_utf_8( point[ 0 ] ) || !is_valid_utf_8( point[ 1 ] ) ) {
				throw error_message;
			}

			auto first = ( character ^ 0xE0 ) << 16;
			auto second = ( point[ 0 ] ^ 0x80 ) << 2 << 8;
			auto third = ( point[ 1 ] ^ 0x80 ) << 2;
			auto all = ( ( ( first | second ) >> 2 ) | third ) >> 2;

			text.push_back( all );
		}
		//	If characer's leading five bits are 11110,
		//	means that character was encoded in four bytes
		else if ( ( character >> 3 ) == 0x1E ) {
			UTF_8_type point[ 3 ];

			input_file.read( (char*)point, byte_count );
			input_file.read( (char*)( point + 1 ), byte_count );
			input_file.read( (char*)( point + 2 ), byte_count );

			if ( !is_valid_utf_8( point[ 0 ] ) ||
				!is_valid_utf_8( point[ 1 ] ) ||
				!is_valid_utf_8( point[ 2 ] ) ) {
				throw error_message;
			}

			auto first = ( character ^ 0xF0 ) << 16 << 8;
			auto second = ( point[ 0 ] ^ 0x80 ) << 2 << 16;
			auto third = ( point[ 1 ] ^ 0x80 ) << 2 << 8;
			auto forth = ( point[ 2 ] ^ 0x80 ) << 2;
			auto all = ( ( ( ( ( first | second ) >> 2 ) | third ) >> 2 ) | forth ) >> 2;

			text.push_back( all );
		}
		else {
			throw error_message;
		}
	}
}

void decode_utf_16( vector<unsigned int>& text, ifstream& input_file ) {

	UTF_16_type character;
	auto byte_count = sizeof( character );
	string error_message = "Input file is not in UTF-16 format\n";

	//	Checks, if bytes, after first byte, starts with 110111(b)
	auto is_valid_utf_16 = []( const UTF_16_type& c ) {return ( ( c >> 10 ) == 0x37 ); };

	while ( input_file.read( (char*)( &character ), byte_count ) ) {
		//	If first byte is'n start with 1101_10..
		if ( ( character & 0xD800 ) != 0xD800 ) {
			//	and if leading bit is 1
			//	means that this character is not valid UTF-16 format
			if ( ( character >> 15 ) == 0x1 ) {
				throw error_message;
			}
			else {
				text.push_back( character );
			}
		}
		else {
			auto high = character - 0xD800;

			input_file.read( (char*)( &character ), byte_count );

			if ( !is_valid_utf_16( character ) ) {
				throw error_message;
			}

			auto low = character - 0xDC00;
			auto all = ( ( high << 10 ) | low ) + 0x10000;

			text.push_back( all );
		}
	}
}

void decode_utf_32( vector<unsigned int>& text, ifstream& input_file ) {

	UTF_32_type character;
	auto byte_count = sizeof( character );

	while ( input_file.read( (char*)&character, byte_count ) ) {
		text.push_back( character );
	}
}

void read_from_file( UTF& first_type, vector<unsigned int>& input_vector, ifstream& input_file )
{
	try {
		switch ( first_type ) {
		case UTF::UTF_8:
			decode_utf_8( input_vector, input_file );
			break;
		case UTF::UTF_16:
			decode_utf_16( input_vector, input_file );
			break;
		case UTF::UTF_32:
			decode_utf_32( input_vector, input_file );
			break;
		}
	}
	catch ( string e ) {
		throw e;
	}
}
