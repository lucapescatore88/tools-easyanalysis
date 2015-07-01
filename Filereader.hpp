/***
* class FileReader
*
* author: Luca Pescatore
* email : luca.pescatore@cern.ch
* 
* date  : 01/07/2015
***/


#ifndef FILEREADER_HPP
#define FILEREADER_HPP

#include <fstream>
#include <string>
#include <sstream>


using namespace std;

class FileReader {
	public:
		FileReader(const char* filename);
		FileReader(const string &filename);

		bool nextLine();
  
		string nextTxtLine();

		template < typename T > T getField(const int n)
		{
			isValid_ = true;
			istringstream ist(line);
			this->skip_fields(ist, n-1);
			T rval;
			ist >> rval;
    
			if (ist.fail()) 
				isValid_= false;

			return rval;
		}
  
		bool inputFailed() const { return !isValid_; };
		bool isValid() const;

	private:
		void skip_fields(istringstream& ist, const int n);
		ifstream file;
		string line;
		bool isValid_;
};

#endif