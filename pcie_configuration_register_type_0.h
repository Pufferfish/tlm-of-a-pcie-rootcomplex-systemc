//
//  pcie_configuration_register_type_0.h
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//
#ifndef __Even__pcie_configuration_register_type_0_h__
#define __Even__pcie_configuration_register_type_0_h__

#include "pcie_configuration_register.h"

#define NUMBER_OF_BASE_ADDRESS_REGISTERS_TYPE0 6

struct ConfigurationRegisterType0: ConfigurationRegister {//Configured by the RC, abstracted, set in constructor

	int
			base_address_registers_segment_startpoints[NUMBER_OF_BASE_ADDRESS_REGISTERS_TYPE0]; //
	int
			base_address_registers_segment_limit[NUMBER_OF_BASE_ADDRESS_REGISTERS_TYPE0]; //

	int busNumber;
	int deviceNumber;
	int functionNumber;
	bool useECRCWhenSending;

	bool IsAddressInBaseAddressRegisters(int);
};

#endif
