//
//  software_data.h
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//

#include "tlp_header_packet_type.h"
#include "tlp_header_completion_status.h"

#ifndef Even_pcie_socket_software_layer_data_h
#define Even_pcie_socket_software_layer_data_h

struct SoftwareData { //TODO	Divide in several childs
	int simulation_TLP_number;
	int address1;
	int address2;
	PacketType type;
	int length;
	int trafficClass;
	bool firstDWBE[4];
	bool lastDWBE[4];
	bool noSnoop;
	bool relaxedOrdering;
	bool EP;
	std::vector<int> data;
	int byteCount;
	bool bcm;
	CompletionStatus cplStatus;
	int lowerAddress;
	int tag;
	int reqIDBusNr;
	int reqIDDevNr;
	int reqIDFuncNr;

	///CONSTRUCTOR///
	SoftwareData() {
		simulation_TLP_number=0;

		address1 = 0;
		address2 = -1; // Constructed w/o address part 2
		type = MRd;
		length = 0;
		trafficClass = 0;
		firstDWBE[0] = 0;
		firstDWBE[1] = 0;
		firstDWBE[2] = 0;
		firstDWBE[3] = 0;
		lastDWBE[0] = 0;
		lastDWBE[1] = 0;
		lastDWBE[2] = 0;
		lastDWBE[3] = 0;
		noSnoop = 0;
		relaxedOrdering = 0;
		EP = 0;
		byteCount = 0;
		bcm = 0;
		cplStatus = COMPLETION_ABORT;
		lowerAddress = 0;
		tag = 0;
		reqIDBusNr = 0;
		reqIDDevNr = 0;
		reqIDFuncNr = 0;

	}
	virtual ~SoftwareData() {
	}

	inline bool operator ==(const SoftwareData& rhs) const {//TODO if data == data
		return (rhs.address1 == address1 && rhs.address2 == address2
				&& rhs.type == type && rhs.length == length && rhs.trafficClass
				== trafficClass && rhs.firstDWBE == firstDWBE && rhs.lastDWBE
				== lastDWBE && rhs.noSnoop == noSnoop && rhs.relaxedOrdering
				== relaxedOrdering && rhs.byteCount == byteCount && rhs.bcm
				== bcm && rhs.cplStatus == cplStatus && rhs.lowerAddress
				== lowerAddress && rhs.tag == tag && rhs.reqIDBusNr
				== reqIDBusNr && rhs.reqIDDevNr == reqIDDevNr
				&& rhs.reqIDFuncNr == reqIDFuncNr && rhs.reqIDFuncNr==simulation_TLP_number);
	}

	inline SoftwareData& operator =(const SoftwareData& rhs) {
		simulation_TLP_number = rhs.simulation_TLP_number;
		address1 = rhs.address1;
		address2 = rhs.address2;
		type = rhs.type;
		length = rhs.length;
		trafficClass = rhs.trafficClass;
		memcpy(firstDWBE, rhs.firstDWBE, 4);
		memcpy(lastDWBE, rhs.lastDWBE, 4);
		noSnoop = rhs.noSnoop;
		relaxedOrdering = rhs.relaxedOrdering;
		EP = rhs.EP;
		data = rhs.data;
		//Completion Specific
		byteCount = rhs.byteCount;
		bcm = rhs.bcm;
		cplStatus = rhs.cplStatus;
		lowerAddress = rhs.lowerAddress;
		tag = rhs.tag;
		reqIDBusNr = rhs.reqIDBusNr;
		reqIDDevNr = rhs.reqIDDevNr;
		reqIDFuncNr = rhs.reqIDFuncNr;
		return *this;
	}

	inline friend void sc_trace(sc_trace_file *tf, const SoftwareData & v,
			const std::string& NAME) {
		cout << "Trying to trace DataFromSWLayer" << endl;
	}

	inline friend ostream& operator <<(ostream& os, SoftwareData const & v) { //Todo data and cpl specific, maby one version for Cpl and another for Req if else
		cout
				<< "|______________________________DATA______________________________________| \t TLP System Number: "<<v.simulation_TLP_number
				<< endl;
		os << "| Type:   " << v.type << "\t\t" << "Address1:" << v.address1
				<< '\t' << '\t' << "Address2: " << v.address2 << "\t\t" << " |"
				<< endl;
		os << "| Length: " << v.length << '\t' << '\t' << "TrafficClass"
				<< v.trafficClass << "\t\t" << "FirstDWBE: ";
		for (int i = 0; i < 4; i++)
			os << v.firstDWBE[i];
		os << '\t' << '\t' << " |" << endl;
		os << "| LastDWBE: ";
		for (int i = 0; i < 4; i++)
			os << v.lastDWBE[i];
		os << '\t' << "NoSnoop: " << v.noSnoop << "\t\t" << "Relaxed Ordering:"
				<< v.relaxedOrdering << '\t' << " |" << endl;
		os << "| EP: " << v.EP << '\t' << '\t' << '\t' << '\t' << '\t' << '\t'
				<< '\t' << '\t' << '\t' << " |" << endl;
		os << "| [ TLP Data =" << '\t' << "< ";
		for (std::vector<int>::const_iterator i = v.data.begin(); i
				!= v.data.end(); ++i) {
			os << *i << ' ';
		}
		os << "> ]" << '\t' << '\t' << '\t' << '\t' << '\t' << '\t' << '\t'
				<< " " << endl;//|"<<endl;
		os << "| EP: " << v.EP << '\t' << "byteCount:" << v.byteCount << '\t'
				<< "bcm:" << v.bcm << "\t\t" << "CPL status:" << v.cplStatus
				<< "\t\t\t |" << endl;

		os << "| LowerAddress: " << v.lowerAddress << '\t' << "tag:" << v.tag
				<< "\t\t" << "ReqID:" << v.reqIDBusNr << " " << v.reqIDDevNr
				<< " " << v.reqIDFuncNr << "\t\t\t |" << endl;

		cout
				<< "|________________________________________________________________________|";

		return os;
	}
};

#endif
