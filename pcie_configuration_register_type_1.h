//
//  pcie_configuration_register_type_1.h
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//
#ifndef __Even__pcie_configuration_register_type_1_h__
#define __Even__pcie_configuration_register_h_type_1__

#include "pcie_configuration_register.h"
#include <vector>
#include <iostream>
using namespace std;

#define MAXIMUM_DIRECT_DOWNSTREAM_CONNECTIONS 10

#define NUMBER_OF_BASE_ADDRESS_REGISTERS_TYPE1 2

struct ConfigurationRegisterType1: ConfigurationRegister {//Configured by the RC, abstracted, set in constructor
	int
			base_address_registers_segment_startpoints[NUMBER_OF_BASE_ADDRESS_REGISTERS_TYPE1]; //
	int
			base_address_registers_segment_limit[NUMBER_OF_BASE_ADDRESS_REGISTERS_TYPE1]; // range= base -> base+size

	int
			port_base_address_registers_base_array[MAXIMUM_DIRECT_DOWNSTREAM_CONNECTIONS]; //from 			page 144 prefetchable vs non-prefetchable is not destinguished
	int
			port_base_address_registers_limit_array[MAXIMUM_DIRECT_DOWNSTREAM_CONNECTIONS]; //delta, range = from -> limt

	int busNumber;
	int deviceNumber;
	int functionNumber;
	bool useECRCWhenSending;

	bool IsAddressInBaseAddressRegisters(int);

	int CheckBaseAndLimitThenForwardToPortNumber(int);

	ConfigurationRegisterType1();
};

#endif
