//
//  pcie_configuration_register.h
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//
#ifndef __Even__pcie_configuration_register_h__
#define __Even__pcie_configuration_register_h__

struct ConfigurationRegister {//Configured by the RC, abstracted, set in constructor

	int busNumber;
	int deviceNumber;
	int functionNumber;
	bool useECRCWhenSending;
};

#endif
