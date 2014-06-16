//
//  sc_main.cpp
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//

#include <iostream>
#include <ctime> // Needed for the true randomization
#include "systemc.h"
#include "sc_main_help_functions.h"
#include <string>

#include "complete_system.h"
#include "root_complex.h"
#include "memory.h"
#include "pcie_endpoint.h"
#include "pcie_socket.h"
#include "tlp.h"
#include "pcie_switch.h"

extern int simulation_time;
extern bool plot_mrd_cpl_delay_dist;

using namespace std;

int sc_main(int argc, char* argv[]) {

	sc_set_time_resolution(1, SC_PS);

	// This will ensure a really randomized number by help of time. Used in Root Complex
	srand(time(0));

	//Set up program depending on user input with execommand, function is defined in main_help_functions.h
	InitializeProgramFromInputArguments(argc, argv);

	// The entire system is a module with several submodules defined in the CompleteSystem module
	CompleteSystem * complete_system = new CompleteSystem("COMPLETESYSTEM");

	//Run simulation,function is defined in main_help_functions.h
	RunSystemCSimulation(simulation_time, argc, argv);

	//Statistics are printed out in each submodul's destructor
	delete complete_system;	//generates out_deltas.txt

	if(plot_mrd_cpl_delay_dist){
		std::string filename = "0_plot_text_file.py";
		std::string command = "python ";
		command += filename;
		system(command.c_str());
	}

	return 0;
}
