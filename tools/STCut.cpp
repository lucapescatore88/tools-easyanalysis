//author: Guido Andreassi <guido.andreassi@cern.ch>
//20.02.2018

#include "STCut.hpp"

//constructor with TCut
STCut::STCut (TCut& cut) {
	c=cut;
}

//constructor with string
STCut::STCut (const char *title) {
	c = TCut(title);
}



//"get" functions
const TCut* STCut::GetTCut() const {
	return &c;
}

const std::vector<STCut*>* STCut::GetMembers(bool deep) const { //if deep, it goes down the hierarchy of members, returning the base ones

	if (!deep) return &members; //"normal" behavious
	else {
		std::vector<STCut*>* basic_members = new std::vector<STCut*>;
		STCut::fill_basic_members(basic_members, &members);

	return basic_members;
	}
 }

void STCut::fill_basic_members (std::vector<STCut*>* basic_members, const std::vector<STCut*>* input_members) const {
	for (auto member : *input_members) { //loop on members to find the deepest ones
		const std::vector<STCut*>* members_of_member = member->GetMembers();
		if (members_of_member->empty()) basic_members->emplace_back(member);
		else STCut::fill_basic_members(basic_members, members_of_member);
	}
}


//"get" functions to emulate TCut's behaviour
const char* STCut::GetTitle() const {
	return c.GetTitle();
}

const char* STCut::GetName() const {
	return c.GetName();
}


//operators
 STCut& STCut::operator+=(const char *rhs) {
 	STCut* this_copy = new STCut(*this);
 	
 	members.clear();
 	members.emplace_back(this_copy); //add itself as a member, as it is now (to keep structure)
 	c+=(rhs); //just call TCut's +=
 	STCut* rhsc = new STCut(rhs);
 	members.emplace_back(rhsc);
 	return *this;
 }

  STCut& STCut::operator+=(const STCut& stcut) {
 	return STCut::operator+=(stcut.GetTCut()->GetTitle());
 }



//friend functions
  STCut operator+(const STCut& lhs, const char *rhs) {
  	return STCut(lhs) += rhs;
 }

  STCut operator+(const char *lhs, const STCut& rhs) {
  	return STCut(lhs) += rhs;
 }

  STCut operator+(const STCut& lhs, const STCut& rhs) {
  	return STCut(lhs) += rhs;
 }
