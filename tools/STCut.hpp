//author: Guido Andreassi <guido.andreassi@cern.ch>
//20.02.2018

#ifndef STCUT_HPP
#define STCUT_HPP

#include <vector>

#include "TCut.h"
#include <iostream>

class STCut {

  public:

		const TCut* GetTCut() const;
		const char* GetTitle() const;
		const char* GetName() const;
		const std::vector<STCut>* GetMembers(bool deep=false) const;

		virtual ~STCut(); //destructor

		STCut(const TCut&); //constructor from TCut
		STCut(const char*); //constructor from char*
		STCut(const STCut&); //copy constructor
		STCut() = default; //default empty constructor
		STCut& operator=(const STCut&) = default;

		//+= operators
		STCut& operator+=(const char*);
		STCut& operator+=(STCut &);

		friend STCut operator+(const STCut, const STCut); //I cannot pass the arguments by reference, because this would require (for this specific implementation)
//to distinguish rvalues and lvalues, and this breaks when imported in python
/*		friend STCut operator+(STCut&, STCut&);
		friend STCut operator+(STCut&&, STCut&);
		friend STCut operator+(STCut&, STCut&&);
		friend STCut operator+(STCut&&, STCut&&);*/

		/*
    	friend STCut operator+(const STCut&, const char*);
    	friend STCut operator+(const char*, const STCut&);
    	friend STCut operator+(const STCut&, const STCut&);
		*/

		//STCut* operator&() & = default;


		std::vector<STCut*> pointed_by;
		std::vector<STCut*> ptrs_to_members; //cannot use pointers to STCuts here. It's unsafe, if they point to a temporary object for example

	private:
		TCut c;
		std::vector<STCut> members; //FIX THIS LATER////cannot use pointers to STCuts here. It's unsafe, if they point to a temporary object for example

		void fill_basic_members (std::vector<STCut>*, const std::vector<STCut>*) const;
};



#endif
