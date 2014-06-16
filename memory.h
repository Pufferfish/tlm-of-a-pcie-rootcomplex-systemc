//
//  memory.h
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//

#ifndef __Even__memory__
#define __Even__memory__

#include "systemc.h"


#define NROFDOUBLEWORDS 10000				//MEMORY size could be larger than its part of the system memory.
extern bool print_full_simulation_information;
extern bool print_full_and_system_memory;
extern bool ep_is_reader__init_mem_var;


using namespace std;
SC_MODULE(Memory) {
	sc_in<int> dataToMemory; //writes
	sc_out<bool> write_confirmation;
	sc_out<int> dataFromMemory; //reads
	sc_in<int> address_memory; //Number between  0 and 1073741823 // 4294967296-1, 0 to 99 for now
	sc_in<bool> write_memory;
	sc_in<bool> enable;

	int memory_array[NROFDOUBLEWORDS]; //4bytes=1 int= doubleword
	int pcie_system_memory_address_from = 0;
	int pcie_system_memory_address_to = 0;

	int memory_writes = 0;

	void PrintMemory();

	void MemoryAction();


	SC_HAS_PROCESS(Memory);
	Memory(sc_module_name name_, int system_memory_address_from_,
			int system_memory_address_to_) {
		pcie_system_memory_address_from = system_memory_address_from_;
		pcie_system_memory_address_to = system_memory_address_to_;

		SC_THREAD(MemoryAction)
		sensitive << enable << write_memory << address_memory << dataToMemory;

		for(int i =0 ;i<NROFDOUBLEWORDS ;i++){
			if(ep_is_reader__init_mem_var){
				memory_array[i]=i;
			}else{
				memory_array[i]=0;
			}
		}
	}

	~Memory() {
		if (print_full_simulation_information) {
			if (print_full_and_system_memory) {
				cout << "--RC Memory Destructor--" << endl;

				PrintMemory();
			}
			cout << endl << endl;
		}
	}
};
#endif

