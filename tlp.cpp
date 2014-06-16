//
//  tlp.cpp
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//

#include "tlp.h"

void TLP::set_header(TLPHeader * headerObject) {
	header = headerObject;
}

TLPHeader* TLP::get_header() {
	return header;
}

void TLP::set_data(std::vector<int> dataVector) {//int length, bool noSnoop, bool relaxedOrdering, bool EP, bool TD, int TC, PacketType type, Format fmt, bool firstDWBE[4], bool lastDWBE[4], int tag, int busNR, int funcNR, int DWAddress
	data = dataVector;
}

std::vector<int> TLP::get_data() {
	return data;
}

void TLP::set_ecrc(std::vector<int> ecrcVector) {//int length, bool noSnoop, bool relaxedOrdering, bool EP, bool TD, int TC, PacketType type, Format fmt, bool firstDWBE[4], bool lastDWBE[4], int tag, int busNR, int funcNR, int DWAddress
	ecrc = ecrcVector;
}

std::vector<int> TLP::get_ecrc() {
	return ecrc;
}

