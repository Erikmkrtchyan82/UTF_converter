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
		convert( source_encoding, target_encoding, input_file, output_file );
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
