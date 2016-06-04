//----------------------------------------------------------------------
//  Copyright (c) 2007-2008 by Doulos Ltd.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//----------------------------------------------------------------------

// Version 2  16-June-2008 - updated for TLM-2.0
// Version 3  18-June-2008 - updated for TLM-2.0
// Version 4  12-Jan-2009  - fix bug in transport_dbg
// Version 5  26-Sep-2009  - fix bug with set_end_address

// Getting Started with TLM-2.0, Tutorial Example 1 - 3

// For a full description, see http://www.doulos.com/knowhow/systemc/tlm2

//----------------------------------------------------------------------
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

void Memory::do_memory()
{
  // Register callback for incoming b_transport interface method call
  socket.register_b_transport(this, &Memory::b_transport);
  socket_dbg.register_transport_dbg(this, &Memory::transport_dbg);

  // initialize memory with image contents
//  unsigned char tmp;
  for (unsigned int i = 0; i < width; i++)
    for (unsigned int j = 0; j < height; j++)
      mem[i*height+j] = IMAGE[i][j];
}

// TLM-2 blocking transport method
void Memory::b_transport( tlm::tlm_generic_payload& trans, sc_time& delay )
{
	cout<<"b_transport first"<<endl;
  tlm::tlm_command cmd = trans.get_command();
  sc_dt::uint64    adr = trans.get_address();
  unsigned char*   ptr = trans.get_data_ptr();
  unsigned int     len = trans.get_data_length();
  unsigned char*   byt = trans.get_byte_enable_ptr();
  unsigned int     wid = trans.get_streaming_width();

  // Obliged to check address range and check for unsupported features,
  //   i.e. byte enables, streaming, and bursts
  // Can ignore DMI hint and extensions
  // Using the SystemC report handler is an acceptable way of signalling an error
	cout<<"wid  "<<wid<<"len   "<<len<<"adr "<<adr<<endl;
//  if (adr >= sc_dt::uint64(width*height) || byt != 0 || len > 1 || wid < len)
//    SC_REPORT_ERROR("TLM-2", "Target does not support given generic payload transaction");
  if (adr >= sc_dt::uint64(width*height))
    SC_REPORT_ERROR("TLM-2", "Target does not support given generic payload transaction adr");
  if (byt != 0)
    SC_REPORT_ERROR("TLM-2", "Target does not support given generic payload transaction byt");
  if (len > 1)
    SC_REPORT_WARNING("TLM-2", "Target does not support given generic payload transaction len");
  if (wid < len)
    SC_REPORT_WARNING("TLM-2", "Target does not support given generic payload transaction wid");



  // Obliged to implement read and write commands
  if ( cmd == tlm::TLM_READ_COMMAND )
  {
    // cout << "Mem: Reading Cell " << +(unsigned int)adr << ": " << +(unsigned char)mem[adr] << "\n";
		cout<<"b_transport read before"<<endl;
    memcpy(ptr, &mem[adr], len);
		cout<<"b_transport read"<<endl;
  }
  else if ( cmd == tlm::TLM_WRITE_COMMAND )
  {
    // cout << "Mem: Writing Cell " << +(unsigned int)adr << ": " << +(unsigned char)(*ptr) << "\n";
    memcpy(&mem[adr], ptr, len);
		cout<<"b_transport write"<<endl;
  }
	
  // Obliged to set response status to indicate successful completion
  trans.set_response_status( tlm::TLM_OK_RESPONSE );
}

unsigned int Memory::transport_dbg(tlm::tlm_generic_payload& trans)
{
  tlm::tlm_command cmd = trans.get_command();
  sc_dt::uint64    adr = trans.get_address();
  unsigned char*   ptr = trans.get_data_ptr();
  unsigned int     len = trans.get_data_length();

  // Calculate the number of bytes to be actually copied
  unsigned int num_bytes = (len < (width*height) - adr ) ? len : (SIZE - adr);

  if ( cmd == tlm::TLM_READ_COMMAND )
  {
    // cout << "Mem(dbg): Reading Cell " << +(unsigned int)adr << ": " << +(unsigned char)mem[adr] << "\n";
    memcpy(ptr, &mem[adr], num_bytes);
  }
  else if ( cmd == tlm::TLM_WRITE_COMMAND )
    // memcpy(&mem[adr], ptr, num_bytes);
    SC_REPORT_ERROR("TLM-2", "Target does not allow writing through transport_dbg");

  return num_bytes;
}
