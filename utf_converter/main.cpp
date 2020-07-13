#include <iostream>
#include <fstream>
#include <vector>
#include <string>
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

	//	For readin text from input_file
	//	Why unsigned int? Because it can contain from 1 to 4 bytes decoded characters
	vector<unsigned int> text;

	try {
		check_invalid_inputs( argv, input_file, output_file );

		read_from_file( source_encoding, text, input_file );

		switch ( target_encoding ) {
		case UTF::UTF_8:
		{
			convert<UTF_8_type>( text, output_file );
		}
		break;
		case UTF::UTF_16:
		{
			convert<UTF_16_type>( text, output_file );
		}
		break;
		case UTF::UTF_32:
		{
			convert<UTF_32_type>( text, output_file );
		}
		break;
		}
	}
	catch ( string error ) {
		std::cerr << error << std::endl;
		input_file.close();
		output_file.close();
		std::cout << "Exiting...\n\n";
		return 1;
	}

	std::cout << "Complete!\n";

	input_file.close();
	output_file.close();

	return 0;
}
