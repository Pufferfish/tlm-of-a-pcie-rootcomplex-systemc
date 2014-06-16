//
//  memory.cpp
//  Even
//  Copyright (c) 2014 Even Låte. All rights reserved.
//

#include "memory.h"

void Memory::PrintMemory() {
	cout << "\t" << "- - -Printing RC PCIe Subsystem_Memory- - -" << endl;
	for (int i = pcie_system_memory_address_from; i
			< pcie_system_memory_address_to - pcie_system_memory_address_from; i
			= i + 30) {
		cout << "\t\t"
				<< memory_array[i] << " " << memory_array[i + 1] << " "
				<< memory_array[i + 2] << " " << memory_array[i + 3] << " "
				<< memory_array[i + 4] << " " << memory_array[i + 5] << " "
				<< memory_array[i + 6] << " " << memory_array[i + 7] << " "
				<< memory_array[i + 8] << " " << memory_array[i + 9] << "  "
				<< memory_array[i + 10] << " " << memory_array[i + 11] << " "
				<< memory_array[i + 12] << " " << memory_array[i + 13] << " "
				<< memory_array[i + 14] << " " << memory_array[i + 15] << " "
				<< memory_array[i + 16] << " " << memory_array[i + 17] << " "
				<< memory_array[i + 18] << " " << memory_array[i + 19] << "  "
				<< memory_array[i + 20] << " " << memory_array[i + 21] << " "
				<< memory_array[i + 22] << " " << memory_array[i + 23] << " "
				<< memory_array[i + 24] << " " << memory_array[i + 25] << " "
				<< memory_array[i + 26] << " " << memory_array[i + 27] << " "
				<< memory_array[i + 28] << " " << memory_array[i + 29] <<endl;
		if (i > 2000) {

			break;
		}
	}

}



void Memory::MemoryAction() {
	while (true) {
		wait();
		if (enable == 1) {
			if (print_full_simulation_information) {
				//cout << "---Memory Action @" << sc_time_stamp() << endl;
			}
			if (write_memory == 1) { //Writing	bytte ut 1 m en enum

				memory_array[address_memory] = dataToMemory;
				//wait(1,SC_NS);
				write_confirmation = !write_confirmation; //toggle write allert

				if (print_full_simulation_information) {

					cout << "---Data: " << memory_array[address_memory] << "  "
							<< dataToMemory
							<< " Is written to memory-address: "
							<< address_memory << endl;
				}

				memory_writes++;

			} else {
				dataFromMemory = -1;
				wait(1, SC_NS);
				dataFromMemory = memory_array[address_memory];

				if (print_full_simulation_information) {
					cout << "---DW at address:" << address_memory
							<< ", that is: " << memory_array[address_memory]
							<< ". Is available on the DATAFromMemory bus "
							<< endl;
				}
			}

		} else {
			if (print_full_simulation_information) {
				//cout << "---Memory is disabled" << endl;
			}
		}
	}
}
