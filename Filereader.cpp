#include "Filereader.hpp"

using namespace std;

FileReader::FileReader(const char * filename)
	: file(filename), line(), isValid_(true)
	{};

FileReader::FileReader(const string &filename)
	: file(filename.c_str()), line(), isValid_(true)
	{};

	bool FileReader::isValid() const
	{
		if ( ! file ) return false;
		else return true;
	}

	bool FileReader::nextLine()
	{
		getline(file, line);
		if (file.eof()) return false;
		else return true;
	}
	
	string FileReader::nextTxtLine()
	{
		do
		{
			getline(file, line);
			if (file.eof()) return (string)("");
		}
		while( line != "" );
			
		return line;
	}

	void FileReader::skip_fields(istringstream& ist, const int n)
	{
		if (n < 1) return;
		string tmp;
		for(int i = 1; i <= n; ++i) ist >> tmp;
	}