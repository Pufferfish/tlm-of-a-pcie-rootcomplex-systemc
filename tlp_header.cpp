//
//  tlp_header.cpp
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//

#include "tlp_header.h"

void TLPHeader::SetTLPHeader(int slength, bool snoSnoop, bool srelaxedOrdering,
		bool sEP, bool sTD, int sTC, PacketType stype, Format sfmt) {
	length = slength;
	noSnoop = snoSnoop;
	relaxedOrdering = srelaxedOrdering;
	EP = sEP;
	TD = sTD;
	TC = sTC;
	type = stype;
	fmt = sfmt;
}
