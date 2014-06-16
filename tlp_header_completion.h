//
//  tlp_header_completion.h
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//

#ifndef __Even__completion_header__
#define __Even__completion_header__

#include "tlp_header.h"

struct CompletionHeader: public TLPHeader {
	//Completions
	int byteCount; //Non-Generic-field CPL 	TODO ?
	bool bcm; //Non-Generic-field CPL 	Take PCI-X completers into account if any
	CompletionStatus cplStatus; //Non-Generic-field CPL 	Status of the completion packet
	int cplIDBusNr; //Non-Generic-field CPL 	//Completer-ID for debugging bustraffic	//0-255	max256
	int cplIDDevNr; //Non-Generic-field CPL 	//Completer-ID 0-31	max32
	int cplIDFuncNr; //Non-Generic-field CPL 	//Completer-ID 0-7	max8
	int lowerAddress; //Non-Generic-field CPL 	TODO?
	int tag; //requesttag for comparison
	int reqIDBusNr; //Non-Generic-field CPL 	The ID of the requester that is to receive the package. For routing
	int reqIDDevNr; //Non-Generic-field CPL 	For routing
	int reqIDFuncNr; //Non-Generic-field CPL 	For routing

	CompletionHeader() {
		type = Cpl; //parent variable

		byteCount = 0;
		bcm = false;
		cplStatus = COMPLETION_ABORT;
		cplIDBusNr = 0;
		cplIDDevNr = 0;
		cplIDFuncNr = 0;
		lowerAddress = 0;
		tag = 0;
		reqIDBusNr = 0;
		reqIDDevNr = 0;
		reqIDFuncNr = 0;
	}

	virtual ~CompletionHeader() {
	}
	;

	//void setCplHeader(int slength, bool snoSnoop, bool srelaxedOrdering, bool sEP, bool sTD, int sTC, PacketType stype, Format sfmt, int stag, int byteCount, bool bcm,int cplStatus, int cplIDBusNr,int cplIDDevNr,int cplIDuncNr,int lowerAddress,int tag,int reqIDBusNr,int reqIDDevNr,int reqIDFuncNr);//Changed
	void SetCompletionHeader(int slength, bool snoSnoop, bool srelaxedOrdering,
			bool sEP, bool sTD, int sTC, PacketType stype, Format sfmt,
			int sbyteCount, bool sbcm, CompletionStatus scplStatus,
			int scplIDBusNr, int scplIDDevNr, int scplIDuncNr,
			int slowerAddress, int stag, int sreqIDBusNr, int sreqIDDevNr,
			int sreqIDFuncNr);

	virtual inline bool operator ==(const CompletionHeader& rhs) const {

		return (rhs.byteCount == byteCount && rhs.bcm == bcm && rhs.cplStatus
				== cplStatus && rhs.cplIDBusNr == cplIDBusNr && rhs.cplIDDevNr
				== cplIDDevNr && rhs.cplIDFuncNr == cplIDFuncNr
				&& rhs.lowerAddress == lowerAddress && rhs.tag == tag
				&& rhs.reqIDBusNr == reqIDBusNr && rhs.reqIDDevNr == reqIDDevNr
				&& rhs.reqIDFuncNr == reqIDFuncNr && rhs.fmt == fmt && rhs.type
				== type && rhs.TC == TC && rhs.TD == TD && rhs.EP == EP
				&& rhs.relaxedOrdering == relaxedOrdering && rhs.noSnoop
				== noSnoop && rhs.length == length);//&&firstDWBEBool && lastDWBEBool  );//&& rhs.address_memory == address_memory && rhs.type == type && rhs.length==length && rhs.traficClass==traficClass && rhs.firstDWBE==firstDWBE && rhs.lastDWBE==lastDWBE && rhs.noSnoop==noSnoop && rhs.relaxedOrdering==relaxedOrdering );
	}

	// Defining Assignment(=) for TLPheader
	virtual inline CompletionHeader& operator =(const CompletionHeader& rhs) { //RightHandSide

		length = rhs.length; //Generic	TODO possible to merge this operator overload with TLPHeader's?
		noSnoop = rhs.noSnoop;
		relaxedOrdering = rhs.relaxedOrdering;
		EP = rhs.EP;
		TD = rhs.TD;
		TC = rhs.TC;
		type = rhs.type;
		fmt = rhs.fmt;

		byteCount = rhs.byteCount; //Cpl Specific
		bcm = rhs.bcm;
		cplStatus = rhs.cplStatus;
		cplIDBusNr = rhs.cplIDBusNr;
		cplIDDevNr = rhs.cplIDDevNr;
		cplIDFuncNr = rhs.cplIDFuncNr;
		lowerAddress = rhs.lowerAddress;
		tag = rhs.tag;
		reqIDBusNr = rhs.reqIDBusNr;
		reqIDDevNr = rhs.reqIDDevNr;
		reqIDFuncNr = rhs.reqIDFuncNr;

		return *this;
	}

	// Defining SC_Trace for Cplheader
	inline friend void sc_trace(sc_trace_file *tf, const CompletionHeader & v,
			const std::string& NAME) {
		cout << "Trying to trace the Cpl Header" << endl;
	}

	// Defining Cout for Cplheader
	virtual void PrintMyself(ostream& os) const {
		//TODO fix print allignment
		os << endl
				<< "|___________________________CplHEADER____________________________________|"
				<< endl;
		os << "|| " << "(Fmt=" << this->fmt << ", Type=" << this->type
				<< ", TC=" << this->TC << ", TD=" << this->TD << ", EP="
				<< this->EP << ", Attr=" << this->relaxedOrdering
				<< this->noSnoop << ", length=" << this->length << ")         "
				<< '\t' << "||" << endl;
		os << "|| " << "(Completer ID=" << this->cplIDBusNr << this->cplIDDevNr
				<< this->cplIDFuncNr << ", ComplStatus=" << this->cplStatus
				<< ", BCM=" << this->bcm << ", byteCount=" << this->byteCount
				<< ")         " << '\t' << "||" << endl;
		os << "|| " << "(Requester ID=" << this->reqIDBusNr << this->reqIDDevNr
				<< this->reqIDFuncNr << ", Tag=" << this->tag
				<< ", LowerAddress=" << this->lowerAddress << ")         "
				<< "\t\t\t" << "||" << endl;
		os
				<< "||______________________________________________________________________||";
	}
};

#endif
