//
//  tlp_header_packet_type.h
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//

#ifndef __Even_tlp_header_packet_type__
#define __Even_tlp_header_packet_type__

enum PacketType {
	MRd = 0,
	MRdLk,
	MWr,
	IORd,
	IOWr,
	CfgRd0,
	CfgRd1,
	CfgWr0,
	CfgWr1,
	Msg,
	MsgD,
	Cpl,
	CplD,
	CplLk,
	CplDLk
}; //Page 57

#endif
