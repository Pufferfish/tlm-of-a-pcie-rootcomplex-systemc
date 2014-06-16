//
//  tlp_header.h
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//

#include "tlp_header_packet_type.h"
#include "tlp_header_format.h"
#include "tlp_header_completion_status.h"
#include "systemc.h"

#ifndef __Even__tlp_header__
#define __Even__tlp_header__

/**
 * @class TLPHeader
 * http://teledynelecroy.com/protocolanalyzer/protocolstandard.aspx?standardid=3
 * Parent of MemoryRequestHeader and CompletionHeader
 * TODO implement IORequests, Configuration Requests, Message Requests
 */
struct TLPHeader {

	int length; //Generic	//Between 1 and 1024
	bool noSnoop; //Generic Attr	//Snooping is off if true
	bool relaxedOrdering; //Generic Attr	//Strict ordering is off if true
	bool EP; //Generic	//Active High data poisoned
	bool TD; //Generic	//Active HIgh, ecrc follows
	int TC; //Generic	//Trafic-class, number between 0 and 7, including endpoints
	PacketType type; //Generic	//ENUM	packettype page	165								type 0 0000 write, 0 0001 read
	Format fmt; //Generic	//ENUM	format	READ: 00(0) 3dw  01(1) 4dw		Write : 10(2) 3dw 11(3) 4dw

	TLPHeader() { //Constructor
		length = 0;
		noSnoop = false;
		relaxedOrdering = false;
		EP = false;
		TD = false;
		TC = 0;
		type = MRd;
		fmt = DW_3_WO_Data;
	}

	virtual ~TLPHeader() {
	}
	;

	//TLPHeader Functions//
	void SetTLPHeader(int slength, bool snoSnoop, bool srelaxedOrdering,
			bool sEP, bool sTD, int sTC, PacketType stype, Format sfmt);//Changed , int stag


	//Functions Required by systemC to use User-Defined Types in//
	virtual inline bool operator ==(const TLPHeader& rhs) const { //rhs = RightHandSide of logical operator
		return (rhs.fmt == fmt && rhs.type == type && rhs.TC == TC && rhs.TD
				== TD && rhs.EP == EP && rhs.relaxedOrdering == relaxedOrdering
				&& rhs.noSnoop == noSnoop && rhs.length == length);
	}

	virtual inline TLPHeader& operator =(const TLPHeader& rhs) {
		length = rhs.length;
		noSnoop = rhs.noSnoop;
		relaxedOrdering = rhs.relaxedOrdering;
		EP = rhs.EP;
		TD = rhs.TD;
		TC = rhs.TC;
		type = rhs.type;
		fmt = rhs.fmt;
		return *this;
	}
	// Defining SC_Trace for TLPheader
	inline friend void sc_trace(sc_trace_file *tf, const TLPHeader & v,
			const std::string& NAME) {
		cout << "Trying to trace the TLP Header" << endl;
	}
	// Defining Cout for TLPheader
	inline friend ostream& operator<<(ostream& os, TLPHeader const & v) {
		v.PrintMyself(os); //Virtual function to allow for friend function cout to be virtual and overloadable by childs
		return os;
	}

	virtual void PrintMyself(ostream& os) const = 0;
};

#endif
