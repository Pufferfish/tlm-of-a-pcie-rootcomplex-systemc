//
//  pcie_configuration_register_type_0.cpp
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//

#include "pcie_configuration_register_type_0.h"

bool ConfigurationRegisterType0::IsAddressInBaseAddressRegisters(int address) {
	bool is_in_this_memory = false;

	for (int i = 0; i < NUMBER_OF_BASE_ADDRESS_REGISTERS_TYPE0; i++) {
		if ((address >= base_address_registers_segment_startpoints[i])
				&& (address <= (base_address_registers_segment_limit[i]))) {
			is_in_this_memory = true;
			break;
		}
	}
	return is_in_this_memory; //Consume the packet if true, reject if not
}
