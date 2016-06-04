// Copyright (C) 2015 University of Kaiserslautern
// Microelectronic System Design Research Group
//
// This file is part of the FPGAHS Course
// de.uni-kl.eit.course.fpgahs
//
// Matthias Jung <jungma@eit.uni-kl.de>
// Christian De Schryver <schryver@eit.uni-kl.de>
// Markus Steffes <steffesm@rhrk.uni-kl.de>

// Needed for the simple_target_socket
#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "systemc.h"
#include <iostream>
#include <tlm.h>
#include "tlm_utils/simple_target_socket.h"

#include "memory.h"
#include "image.h"
#include "median_module.h"

void medianFilter(const unsigned char input[][HEIGHT], unsigned char output[][HEIGHT], unsigned int width, unsigned int height)
{

    unsigned int x;
    for(x = 1; x < (width - 1); x++)
    {
        unsigned int y;
        for(y = 1; y < (height - 1); y++)
        {
            // Extract Neighbourhood
            unsigned char SORT[9];
            SORT[0] = input[y-1][x-1];
            SORT[1] = input[y-1][x  ];
            SORT[2] = input[y-1][x+1];
            SORT[3] = input[y  ][x-1];
            SORT[4] = input[y  ][x  ];
            SORT[5] = input[y  ][x+1];
            SORT[6] = input[y+1][x-1];
            SORT[7] = input[y+1][x  ];
            SORT[8] = input[y+1][x+1];

            // Sort it
            int min;
            int b;
            int i;
            for ( i = 0; i < 5; i++)
            {
                b = i;
                min = SORT[b];

                int j;
                for (j = i; j < 9; j++)
                {
                    if (SORT[j] < min)
                    {
                        b = j;
                        min = SORT[b];
                    }
                }
                SORT[b] = SORT[i];
                SORT[i] = min;
            }

            // Writeback of the Median:
            output[x][y] =SORT[4];
        }
    }
    return;
}

void median_module::do_median()
{
  // TODO: Your code here
	unsigned char tlm_buffer[WIDTH*HEIGHT];
	unsigned char input[WIDTH][HEIGHT];
	unsigned char output[WIDTH][HEIGHT];
	tlm::tlm_generic_payload *trans = new tlm::tlm_generic_payload;
	sc_time delay = sc_time(0, SC_NS);
	tlm::tlm_command cmd;
	while(true){
		cout<<"medianloop"<<endl;
		if(start.read() == true) {
			cout<<"filter started"<<endl;
			cmd = tlm::TLM_READ_COMMAND;
			trans->set_command(cmd);
			trans->set_address(0);
			trans->set_data_length(WIDTH*HEIGHT);
			trans->set_data_ptr(tlm_buffer);
			trans->set_streaming_width(1);
			trans->set_byte_enable_ptr(0);
			trans->set_dmi_allowed(false);
			trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
			socket->b_transport(*trans, delay);
			for(unsigned int i = 0; i < WIDTH; i++){
				for(unsigned int j = 0; j < HEIGHT; j++){
					input[i][j] = tlm_buffer[i*WIDTH+j];
				}
			}
			medianFilter(input, output, WIDTH, HEIGHT);
			for(unsigned int i = 0; i < WIDTH; i++){
				for(unsigned int j = 0; j < HEIGHT; j++){
					tlm_buffer[i*WIDTH+j] = output[i][j];
				}
			}
			cmd = tlm::TLM_WRITE_COMMAND;
			trans->set_command(cmd);
			trans->set_address(0);
			trans->set_data_length(WIDTH*HEIGHT);
			trans->set_data_ptr(tlm_buffer);
			trans->set_streaming_width(1);
			trans->set_byte_enable_ptr(0);
			trans->set_dmi_allowed(false);
			trans->set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);
			socket->b_transport(*trans, delay);
			finish.write(true);
		}
		else{
			finish.write(false);
		}
		wait();		
	}
} // do_median()

