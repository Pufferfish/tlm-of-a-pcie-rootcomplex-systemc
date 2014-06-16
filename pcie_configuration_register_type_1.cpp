//
//  pcie_configuration_register_type_1.cpp
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//
#include "pcie_configuration_register_type_1.h"

bool ConfigurationRegisterType1::IsAddressInBaseAddressRegisters(int address) {
	bool is_in_this_memory = false;

	for (int i = 0; i < NUMBER_OF_BASE_ADDRESS_REGISTERS_TYPE1; i++) {
		if ((address >= base_address_registers_segment_startpoints[i])
				&& (address <= base_address_registers_segment_limit[i])) {
			is_in_this_memory = true;
			break;
		}
	}
	return is_in_this_memory; //Consume the packet if true, reject if not
}

int ConfigurationRegisterType1::CheckBaseAndLimitThenForwardToPortNumber(
		int address) {
	int port_number = -1;
	for (unsigned i = 0; i < MAXIMUM_DIRECT_DOWNSTREAM_CONNECTIONS; i++) {
		if ((address >= port_base_address_registers_base_array[i]) && (address
				<= (port_base_address_registers_limit_array[i]))) {
			port_number = i;

			//cout << "port_number is "<< i << endl ;
			break;
		}
	}
	return port_number;
}

ConfigurationRegisterType1::ConfigurationRegisterType1() {
	for (int i = 0; i < NUMBER_OF_BASE_ADDRESS_REGISTERS_TYPE1; i++) {
		base_address_registers_segment_startpoints[i] = -1;
		base_address_registers_segment_limit[i] = -1;
	}

	for (int i = 0; i < MAXIMUM_DIRECT_DOWNSTREAM_CONNECTIONS; i++) {
		port_base_address_registers_base_array[i] = -1; //from 			page 144 prefetchable vs non-prefetchable is not destinguished
		port_base_address_registers_limit_array[i] = -1; //delta, range = from -> limt
	}
}
