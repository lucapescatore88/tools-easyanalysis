//author: Guido Andreassi <guido.andreassi@cern.ch>
//20.02.2018

#ifndef STCUT_HPP
#define STCUT_HPP

#include <vector>

#include "TCut.h"

class STCut {

  public:


		const TCut* GetTCut() const;
		const char* GetTitle() const;
		const char* GetName() const;
		const std::vector<STCut*>* GetMembers(bool deep=false) const;

		STCut(TCut&); //constructor from TCut
		STCut(const char*); //constructor from char*


		//+= operators
		STCut& operator+=(const char*);
		STCut& operator+=(const STCut &);

    	friend STCut operator+(const STCut&, const char*);
    	friend STCut operator+(const char*, const STCut&);
    	friend STCut operator+(const STCut&, const STCut&);

	private:
		TCut c;
		std::vector<STCut*> members;
		void fill_basic_members (std::vector<STCut*>*, const std::vector<STCut*>*) const;
};



#endif
