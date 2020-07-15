#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include "utf.h"

int main( int argc, char* argv[] ) {
	if ( argc != 5 ) {
		std::cout << "* Usage: " << argv[ 0 ] <<
			" <source-encoding> <source-file> <target-encoding> <target-file>\n\n";
		return 1;
	}

	ifstream input_file( argv[ 2 ], std::ios_base::binary );
	ofstream output_file( argv[ 4 ], std::ios_base::binary );

	auto source_encoding = find_utf( argv[ 1 ] );
	string source_file = argv[ 2 ];

	auto target_encoding = find_utf( argv[ 3 ] );
	string target_file = argv[ 4 ];

	try {
		check_invalid_inputs( argv, input_file, output_file );

		//	For readin text from input_file
		//	Why unsigned int? Because it can contain from 1 to 4 bytes decoded characters
		vector<unsigned int> code_point;

		read_from_file( source_encoding, code_point, input_file );

		switch ( target_encoding ) {
		case UTF::UTF_8:
		{
			//	code_point can contain code points that requires 4 times more bytes to encode to UTF-8
			vector<UTF_8_type> target( 4 * code_point.size() );
			utf_convert( code_point.begin(), code_point.end(), target.begin() );
			write_in_file( target.begin(), target.end(), output_file );
		}
		break;
		case UTF::UTF_16:
		{
			//	code_point can contain code points that requires 2 times more bytes to encode to UTF-16
			vector<UTF_16_type> target( 2 * code_point.size() );
			utf_convert( code_point.begin(), code_point.end(), target.begin() );
			write_in_file( target.begin(), target.end(), output_file );
		}
		break;
		case UTF::UTF_32:
		{
			vector<UTF_32_type> target( code_point.size() );
			utf_convert( code_point.begin(), code_point.end(), target.begin() );
			write_in_file( target.begin(), target.end(), output_file );
		}
		break;
		}
	}
	catch ( std::exception error ) {
		std::cerr << error.what() << std::endl;
		std::cout << "Exiting...\n\n";

		input_file.close();
		output_file.close();

		return 1;
	}

	std::cout << "Complete!\n";

	input_file.close();
	output_file.close();

	return 0;
}
