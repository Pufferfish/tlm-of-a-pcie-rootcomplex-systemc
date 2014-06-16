//
//  tlp_header_memory_request.cpp
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//

#ifndef __Even__memory_request_header__
#define __Even__memory_request_header__

#include "tlp_header.h"

struct MemoryRequestHeader: public TLPHeader {
	//MemReqHeader Variables//
	bool firstDWBE[4]; //Non-Generic			//1 for enable writing or reading of byt in first dw between	rules at page 168
	bool lastDWBE[4]; //Non-Generic-field Mrd rq			//1 for enable writing or reading of byt in last dw
	int tag; //Non-Generic-field Mrd rq	//between 0 and 31, identify outstanding reqs by the requester
	int reqIDBusNr; //TODO		//Non-Generic-field Mrd rq	//0-255	max256
	int reqIDDevNr; //TODO		//Non-Generic-field Mrd rq	//0-31	max32
	int reqIDFuncNr; //TODO		//Non-Generic-field Mrd rq	//0-7	max8

	int address1; //Non-Generic-field Mrd rq	//30 bits instead of 32 makes adressing Dw-alligned max2³⁰, 1073741824
	int address2; //Non-Generic-field Mrd rq	// address2

	MemoryRequestHeader() :
		TLPHeader() {
		type = MRd;//parent variable

		firstDWBE[0] = 0;
		firstDWBE[1] = 0;
		firstDWBE[2] = 0;
		firstDWBE[3] = 0;
		lastDWBE[0] = 0;
		lastDWBE[1] = 0;
		lastDWBE[2] = 0;
		lastDWBE[3] = 0;
		tag = 0;
		reqIDBusNr = 0;
		reqIDDevNr = 0;
		reqIDFuncNr = 0;
		address1 = 0;
		address2 = -1; //TODO -1 or 0
	}

	MemoryRequestHeader(const MemoryRequestHeader &obj) {
		length = obj.length;
		noSnoop = obj.noSnoop;
		relaxedOrdering = obj.relaxedOrdering;
		EP = obj.EP;
		TD = obj.TD;
		TC = obj.TC;
		type = obj.type;
		fmt = obj.fmt;

		memcpy(lastDWBE, obj.lastDWBE, 4);
		memcpy(firstDWBE, obj.firstDWBE, 4);
		tag = obj.tag;
		reqIDBusNr = obj.reqIDBusNr;
		reqIDDevNr = obj.reqIDDevNr;
		reqIDFuncNr = obj.reqIDFuncNr;

		address1 = obj.address1;
		address2 = obj.address2;
	}

	virtual ~MemoryRequestHeader() {
	}
	;

	void SetMemoryRequestHeader(int slength, bool snoSnoop,
			bool srelaxedOrdering, bool sEP, bool sTD, int sTC,
			PacketType stype, Format sfmt, bool(&sfirstDWBE)[4],
			bool slastDWBE[4], int stag, int sbusNr, int sdevNr, int sfuncNr,
			int saddress1, int saddress2);

	//Functions Required by systemC to use User-Defined Types in//

	// Defining isEqual(==) for MemReqHeader
	inline bool operator ==(const MemoryRequestHeader& rhs) const {

		bool firstDWBEBool = true;
		bool lastDWBEBool = true;
		for (int i = 0; i < 4; i++) {
			if (firstDWBE[i] != rhs.firstDWBE[i]) {
				firstDWBEBool = false;
			}
			if (lastDWBE[i] != rhs.lastDWBE[i]) {
				lastDWBEBool = false;
			}
		}
		return (firstDWBEBool && lastDWBEBool && rhs.address1 == address1
				&& rhs.address2 == address2 && rhs.reqIDFuncNr == reqIDFuncNr
				&& rhs.reqIDDevNr == reqIDDevNr && rhs.reqIDBusNr == reqIDBusNr
				&& rhs.tag == tag && rhs.fmt == fmt && rhs.type == type
				&& rhs.TC == TC && rhs.TD == TD && rhs.EP == EP
				&& rhs.relaxedOrdering == relaxedOrdering && rhs.noSnoop
				== noSnoop && rhs.length == length);//&& rhs.address_memory == address_memory && rhs.type == type && rhs.length==length && rhs.traficClass==traficClass && rhs.firstDWBE==firstDWBE && rhs.lastDWBE==lastDWBE && rhs.noSnoop==noSnoop && rhs.relaxedOrdering==relaxedOrdering );
	}

	// Defining Assignment(=) for MemReqHeader
	inline MemoryRequestHeader& operator =(const MemoryRequestHeader& rhs) { //RightHandSide
		length = rhs.length;
		noSnoop = rhs.noSnoop;
		relaxedOrdering = rhs.relaxedOrdering;
		EP = rhs.EP;
		TD = rhs.TD;
		TC = rhs.TC;
		type = rhs.type;
		fmt = rhs.fmt;

		memcpy(lastDWBE, rhs.lastDWBE, 4);
		memcpy(firstDWBE, rhs.firstDWBE, 4);
		tag = rhs.tag;
		reqIDBusNr = rhs.reqIDBusNr;
		reqIDDevNr = rhs.reqIDDevNr;
		reqIDFuncNr = rhs.reqIDFuncNr;

		address1 = rhs.address1;
		address2 = rhs.address2;
		return *this;
	}

	// Defining SC_Trace for MemReqHeader
	inline friend void sc_trace(sc_trace_file *tf,
			const MemoryRequestHeader & v, const std::string& NAME) {
		cout << "Trying to trace the TLP Header" << endl;
	}

	// Defining Cout for MemReqHeader
	virtual void PrintMyself(ostream& os) const {
		os << endl
				<< "|___________________________MemReqHEADER_________________________________|"
				<< endl;
		os << "|| " << "(Fmt=" << this->fmt << ", Type=" << this->type
				<< ", TC=" << this->TC << ", TD=" << this->TD << ", EP="
				<< this->EP << ", Attr=" << this->relaxedOrdering
				<< this->noSnoop << ", length=" << this->length << ")         "
				<< '\t' << "||" << endl;
		os << "|| " << "(RequesterID=" << this->reqIDBusNr << this->reqIDDevNr
				<< this->reqIDFuncNr << ", Tag=" << this->tag << ", firstDWBE="
				<< this->firstDWBE[0] << this->firstDWBE[1]
				<< this->firstDWBE[2] << this->firstDWBE[3] << ", lastDWBE="
				<< this->lastDWBE[0] << this->lastDWBE[1] << this->lastDWBE[2]
				<< this->lastDWBE[3] << ")" << '\t' << '\t' << "||" << endl;
		os << "|| " << "(Address1=" << this->address1 << ")" << '\t' << '\t'
				<< '\t' << '\t' << '\t' << '\t' << '\t' << '\t' << "  " << endl;
		os << "|| " << "(Address2=" << this->address2 << ")" << '\t' << '\t'
				<< '\t' << '\t' << '\t' << '\t' << '\t' << "||" << endl;
		os
				<< "||______________________________________________________________________||";
	}

};

#endif
