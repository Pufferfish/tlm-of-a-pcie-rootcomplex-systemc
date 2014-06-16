//
//  tlp.h
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//
#ifndef __Even_tlp_h__
#define __Even_tlp_h__

#include <iostream>
#include <vector>
#include <string.h>

#include "systemc.h"
#include "tlp_header.h"
#include "tlp_header_memory_request.h"
#include "tlp_header_completion.h"

extern bool only_ep_sends;

using namespace std;

class TLP {

private:
	TLPHeader* header;
	std::vector<int> data;
	std::vector<int> ecrc;

public:
	int simulation_TLP_number;
	TLP() :
		header(0),simulation_TLP_number(0){
	}
	;

	~TLP() {
		delete header;
	}

	TLP(const TLP &obj) {
		if (dynamic_cast<MemoryRequestHeader*> (obj.header) != 0) {
			MemoryRequestHeader* memoryRequestHeader = new MemoryRequestHeader;
			*memoryRequestHeader
					= *static_cast<MemoryRequestHeader*> (obj.header);
			header = memoryRequestHeader;
		} else { // Must be CplHeader
			CompletionHeader* cplHeader = new CompletionHeader;
			*cplHeader = *static_cast<CompletionHeader*> (obj.header);
			header = cplHeader;
		}
		data = obj.data;
		ecrc = obj.ecrc;
		simulation_TLP_number=obj.simulation_TLP_number;
	}

	//Functions To access TLP-Privates//
	void set_header(TLPHeader * headerObject);
	TLPHeader * get_header();

	void set_data(std::vector<int> data);
	std::vector<int> get_data();

	void set_ecrc(std::vector<int> ecrc);
	std::vector<int> get_ecrc();

	//Functions Required by systemC to use User-Defined Types in//

	// Defining isEqual(==) for TLP
	inline bool operator ==(const TLP& rhs) const {
		return (rhs.header == header && rhs.data == data && rhs.ecrc == rhs.ecrc && simulation_TLP_number==rhs.simulation_TLP_number);
	}

	// Defining Assignment(=) for TLP
	inline TLP& operator =(const TLP& rhs) {
		data = rhs.data;
		ecrc = rhs.ecrc;
		simulation_TLP_number=rhs.simulation_TLP_number;
		//		cout <<"ASSIGNMENT OPERATOR TLP"<<endl;
		if (dynamic_cast<MemoryRequestHeader*> (rhs.header) != 0) {
			if (header == 0) {
				MemoryRequestHeader* memReqHeader = new MemoryRequestHeader;
				*memReqHeader = *static_cast<MemoryRequestHeader*> (rhs.header);
				header = memReqHeader;

			} else {
				if(only_ep_sends){	//leak fix
					*header = *rhs.header;
				}else{
					MemoryRequestHeader* memReqHeader = new MemoryRequestHeader;
					*memReqHeader = *static_cast<MemoryRequestHeader*> (rhs.header);
					header = memReqHeader;
					//delete rhs.header;
					//				*dynamic_cast<MemoryRequestHeader*>(header) = *dynamic_cast<MemoryRequestHeader*> (rhs.header);		//segfault
				}
			}
		} else if (dynamic_cast<CompletionHeader*> (rhs.header) != 0) { // Must be CplHeader
			if (header == 0) {
				CompletionHeader* cplHeader = new CompletionHeader;
				*cplHeader = *static_cast<CompletionHeader*> (rhs.header);
				header = cplHeader;

			} else {
				if(only_ep_sends){	//leak fix
					*header=*rhs.header;
				}else{ //should suffice with top one, however this crashes *header = *rhs.header

					CompletionHeader* cplHeader = new CompletionHeader;
					*cplHeader = *static_cast<CompletionHeader*> (rhs.header);
					header = cplHeader;
					 //delete rhs.header;	//to fix memory leak
					//           	*dynamic_cast<CompletionHeader*>(header) = *dynamic_cast<CompletionHeader*> (rhs.header);
				}
			}

		} else {

			cout
					<< "Packet does not correspond to either of the decleared packet types"
					<< endl;
		}
		return *this;
	}

	// Defining Tracer for TLP	
	inline friend void sc_trace(sc_trace_file *tf, const TLP & v,
			const std::string& NAME) {
		cout << "Trying to create Trace from TLP" << endl;
	}

	// Defining Cout for TLP
	inline friend ostream& operator <<(ostream& os, TLP const & v) {
		os<< "|______________________________TLP_______________________________________| \t TLP System Number: "<<v.simulation_TLP_number;
				os<<*(v.header) << endl;

		//printout data
		os << "| [ TLP Data =" << "\t\t" << "< ";
		int counter=0;
		for (std::vector<int>::const_iterator i = v.data.begin(); i
				!= v.data.end(); ++i) {
			os << *i << ' ';
			counter++;
			if(((counter % 10)==0) && (counter!=0)){
				os <<endl<<"\t\t";
			}

		}
		os << "> ]" << '\t' << '\t' << '\t' << '\t' << '\t' << '\t' << '\t'
				<< " " << endl;

		//printout ecrc
		os << "| [ TLP ECRC =" << '\t' << "< ";

		for (std::vector<int>::const_iterator i = v.ecrc.begin(); i
				!= v.ecrc.end(); ++i) {
			os << *i << ' ';
		}
		os << "> ]" << '\t' << '\t' << '\t' << '\t' << '\t' << '\t' << '\t'
				<< " |" << endl;
		cout
				<< "|________________________________________________________________________|"
				<< endl << endl;
		return os;
	}
};

#endif
