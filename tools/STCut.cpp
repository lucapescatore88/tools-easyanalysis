//author: Guido Andreassi <guido.andreassi@cern.ch>
//20.02.2018

#include "STCut.hpp"
#include <iostream>
#include <algorithm>

//constructor with TCut
STCut::STCut (const TCut& cut) {
	c=cut;
}

//constructor with string
STCut::STCut (const char* title) {
	c = TCut(title);
}

//copy constructor
STCut::STCut (const STCut& stcut) {
	std::cout<<"Copy constructor called for "<<stcut.GetTitle()<<std::endl;
	c = stcut.c;
	members = stcut.members;
	ptrs_to_members=stcut.ptrs_to_members;
	pointed_by=stcut.pointed_by;

	//add itself in pointed_by of the objects it points to
	for (auto &member : ptrs_to_members){
		if (member != NULL){
			auto &v = member->pointed_by;
			v.emplace_back(this);
			std::cout<<"Spingo "<<this<<" in "<<member<<std::endl;
		}
	}
}



//"get" functions
const TCut* STCut::GetTCut() const {
	return &c;
}

const std::vector<STCut>* STCut::GetMembers(bool deep) const { //if deep, it goes down the hierarchy of members, returning the base ones
	std::vector<STCut>* final_members = new std::vector<STCut>;
	int i(0);
	for (auto v : ptrs_to_members) {
		if (v==NULL) {//it means the object does not exist anymore, so it has to be considered as its parts
			std::cout<<"proviamo "<<members[i].GetTitle()<<std::endl;
			if (members[i].GetMembers()->empty()){
				final_members->emplace_back(members[i]);
			} else {
				for (auto &member : *members[i].GetMembers()){
					final_members->emplace_back(member);
				}
			}	
		} else { //it means the object still exists, so it has to be considered as a whole
			final_members->emplace_back(*v);
		}
	i++;
	}
	return final_members;
 }

/*
void STCut::fill_basic_members (std::vector<STCut>* basic_members, const std::vector<STCut>* input_members) const {
	for (auto member : *input_members) { //loop on members to find the deepest ones
		const std::vector<STCut>* members_of_member = member.GetMembers();
		if (members_of_member->empty()) basic_members->emplace_back(member);
		else STCut::fill_basic_members(basic_members, members_of_member);
	}
}
*/

//"get" functions to emulate TCut's behaviour
const char* STCut::GetTitle() const {
	return c.GetTitle();
}

const char* STCut::GetName() const {
	return c.GetName();
}


//operators

 STCut& STCut::operator+=(const char *rhs) {
	STCut this_copy = STCut(*this); //copy the current instance before altering it
 	members.clear();
 	members.emplace_back(this_copy); //add itself as a member, as it is now (to keep structure)
 	c+=(rhs); //just call TCut's +=
 	STCut rhsc = STCut(rhs);
 	members.emplace_back(rhsc);
 	return *this;
 }

  STCut& STCut::operator+=(STCut& rhs) {
  	std::cout<<"operator+= called"<<std::endl;
  	c+=(*rhs.GetTCut());
  	/*
	STCut this_copy = STCut(*this); //copy the current instance before altering it
 	members.clear();
 	members.emplace_back(this_copy); //add itself as a member, as it is now (to keep structure)
 	ptrs_to_members.emplace_back(this);
   	pointed_by.emplace_back(&ptrs_to_members.back());
 	members.emplace_back(rhs);
  	ptrs_to_members.emplace_back(&rhs);
    rhs.pointed_by.emplace_back(&ptrs_to_members.back());*/
 	return *this;
 }



 //destructor
 STCut::~STCut(){
 	std::cout<<"Destructor called on "<<this->GetTitle()<<"  "<<this<<std::endl;

 	//remove itself from pointed_by of other instances
 	for (auto &member : ptrs_to_members){
 		std::cout<<"---going in member "<<member<<std::endl;
 		if (member != NULL){
 			std::cout<<"...member: "<<member->GetTitle()<<"  "<<member<<std::endl;
 			auto v = member->pointed_by;
 			for (auto i: v) std::cout << i << ' ';
 			std::cout<<std::endl;
 			v.erase(std::find(v.begin(), v.end(), this));
 			for (auto i: v) std::cout << i << ' ';
 			std::cout<<std::endl;
 		}
 	}
 	//replace itself by NULL where it is pointed
 	for (STCut* pointing_stcut : this->pointed_by){
 		auto &v = pointing_stcut->ptrs_to_members;
 		for (auto &pointer : v) {
 			if (pointer == this){
 				std::cout<<"...replaced "<<pointer<<" with NULL in "<<pointing_stcut<<std::endl;
 				pointer = NULL;
 			}
 		}
 	}
 }


//friends

  STCut operator+(const STCut lhs, const STCut rhs) {
 	std::cout<<"operator+ (friend) called"<<std::endl;
 	STCut result;
 	result.c=(*lhs.GetTCut()+*rhs.GetTCut());

 	result.members.emplace_back(lhs); //add itself as a member, as it is now (to keep structure)
 	result.ptrs_to_members.emplace_back(&lhs);
 	std::cout<<"+Spingo "<<&lhs<<" in "<<&result<<std::endl;
   	lhs.pointed_by.emplace_back(&result);

 	result.members.emplace_back(rhs);
  	result.ptrs_to_members.emplace_back(&rhs);
  	std::cout<<"+Spingo "<<&rhs<<" in "<<&result<<std::endl;
    rhs.pointed_by.emplace_back(&result);

 	return result;
 }


 /*
 STCut operator+(STCut& lhs, STCut& rhs) {
 	std::cout<<"operator+ (friend) called"<<std::endl;
 	STCut result;
 	result.c=(*lhs.GetTCut()+*rhs.GetTCut());

 	result.members.emplace_back(lhs); //add itself as a member, as it is now (to keep structure)
 	result.ptrs_to_members.emplace_back(&lhs);
 	std::cout<<"+Spingo "<<&lhs<<" in "<<&result<<std::endl;
   	lhs.pointed_by.emplace_back(&result);

 	result.members.emplace_back(rhs);
  	result.ptrs_to_members.emplace_back(&rhs);
  	std::cout<<"+Spingo "<<&rhs<<" in "<<&result<<std::endl;
    rhs.pointed_by.emplace_back(&result);

 	return result;
 }


  STCut operator+(STCut&& lhs, STCut& rhs) {
  	STCut lhs_lvalue = STCut(lhs);
  	return operator+(lhs_lvalue,rhs);
 }

  STCut operator+(STCut& lhs, STCut&& rhs) {
  	STCut rhs_lvalue = STCut(rhs);
  	return operator+(lhs,rhs_lvalue);
 }

   STCut operator+(STCut&& lhs, STCut&& rhs) {
  	STCut lhs_lvalue = STCut(lhs);
   	STCut rhs_lvalue = STCut(rhs);
  	return operator+(lhs_lvalue,rhs_lvalue);
 }
*/
//friend functions
 /*
  STCut operator+(const STCut& lhs, const char *rhs) {
  	return STCut(lhs) += rhs;
 }

  STCut operator+(const char *lhs, const STCut& rhs) {
  	return STCut(lhs) += rhs;
 }

  STCut operator+(const STCut& lhs, const STCut& rhs) {
  	return STCut(lhs) += rhs;
 }
 */
