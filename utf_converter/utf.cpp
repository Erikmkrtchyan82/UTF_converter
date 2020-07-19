#include "utf.h"
#include <vector>
#include <string>
#include <fstream>
#include <cctype>
#include <algorithm>

UTF find_utf( const string& encoding ) {

	string lowercase = encoding;

	for_each( lowercase.begin(), lowercase.end(), []( char& c ) {c = tolower( c ); } );

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

	if ( error_message.size() )
		throw std::invalid_argument( error_message );
}

void convert( UTF source, UTF target, ifstream& ifs, ofstream& ofs ) {

	if ( source == UTF::UTF_8 ) {
		auto input = read_from_file<UTF_8_type>( ifs );

		find_target_and_convert( input, target, ofs );
		return;
	}
	if ( source == UTF::UTF_16 ) {
		auto input = read_from_file<UTF_16_type>( ifs );

		find_target_and_convert( input, target, ofs );
		return;
	}
	if ( source == UTF::UTF_32 ) {
		auto input = read_from_file<UTF_32_type>( ifs );

		find_target_and_convert( input, target, ofs );
		return;
	}
}
