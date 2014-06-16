//
//  tlp_header_completion.cpp
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//

#include "tlp_header_completion.h"

void CompletionHeader::SetCompletionHeader(int slength, bool snoSnoop,
		bool srelaxedOrdering, bool sEP, bool sTD, int sTC, PacketType stype,
		Format sfmt, int sbyteCount, bool sbcm, CompletionStatus scplStatus,
		int scplIDBusNr, int scplIDDevNr, int scplIDFuncNr, int slowerAddress,
		int stag, int sreqIDBusNr, int sreqIDDevNr, int sreqIDFuncNr) {

	length = slength;
	noSnoop = snoSnoop;
	relaxedOrdering = srelaxedOrdering;
	EP = sEP;
	TD = sTD;
	TC = sTC;
	type = stype;
	fmt = sfmt;

	byteCount = sbyteCount; //Cpl Specific
	bcm = sbcm;
	cplStatus = scplStatus;
	cplIDBusNr = scplIDBusNr;
	cplIDDevNr = scplIDDevNr;
	cplIDFuncNr = scplIDFuncNr;
	lowerAddress = slowerAddress;
	tag = stag;
	reqIDBusNr = sreqIDBusNr;
	reqIDDevNr = sreqIDDevNr;
	reqIDFuncNr = sreqIDFuncNr;

}
