//
//  tlp_header_memory_request.cpp
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//

#include "tlp_header_memory_request.h"

void MemoryRequestHeader::SetMemoryRequestHeader(int slength, bool snoSnoop,
		bool srelaxedOrdering, bool sEP, bool sTD, int sTC, PacketType stype,
		Format sfmt, bool(&sfirstDWBE)[4], bool slastDWBE[4], int stag,
		int sbusNR, int sdevNR, int sfuncNR, int saddress1, int saddress2) {
	length = slength;
	noSnoop = snoSnoop;
	relaxedOrdering = srelaxedOrdering;
	EP = sEP;
	TD = sTD;
	TC = sTC;
	type = stype;
	fmt = sfmt;

	memcpy(firstDWBE, sfirstDWBE, 4);
	memcpy(lastDWBE, slastDWBE, 4);
	tag = stag;
	reqIDBusNr = sbusNR;
	reqIDDevNr = sdevNR;
	reqIDFuncNr = sfuncNR;

	address1 = saddress1;
	address2 = saddress2;
}
