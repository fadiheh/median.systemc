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
#include <tlm.h>
#include "tlm_utils/simple_initiator_socket.h"

#include "EasyBMP/EasyBMP.h"

#include "median_tb.h"

void Median_tb::do_median_tb()
{
  // Send start signal
  start.write(true);
  cout << "median filter started\n";
  wait();
  start.write(false);

  cout << "waiting for finish signal\n";

  // wait for module to finish
  while(finish.read() != true)
  {
    // cout << "Waiting for finish \n";
    wait();
  }

  cout << "finish signal recieved\n";

  // read result image
  unsigned char img[width][height];
  read_image_uut(img);

  // read reference image
  BMP image_ref;
  image_ref.ReadFromFile("output_image_reference.bmp");
  // image_ref.ReadFromFile("input_image.bmp");

  // compare result with reference image
  for (unsigned int i = 1; i < width-1; ++i)
    for(unsigned int j = 1; j < height-1; j++)
    {
      // cout << "Checking Pixel (" << (int)i << ", " << (int)j << "): "
      //     << (int)img[i][j] << " / " << (int)image_ref.GetPixel(i, j).Red << "\n";
      if(img[i][j] == 0)
        SC_REPORT_WARNING("Image Check", "Pixel Value is 0");
      if( ( img[i][j] != image_ref.GetPixel(i, j).Red ) ||
          ( img[i][j] != image_ref.GetPixel(i, j).Blue ) || 
          ( img[i][j] != image_ref.GetPixel(i, j).Green )
        ){
//				cout<<"ref -->> "<<image_ref.GetPixel(i, j).Red<<"   "<<image_ref.GetPixel(i, j).Blue<<"  "<<image_ref.GetPixel(i, j).Green<<endl;
				cout<<"i->"<<i<<" j->"<<j<<endl;
        SC_REPORT_WARNING("Image Check", "a Pixel is wrong\n");
			}
    }

  // write result image
  BMP image_out;
  image_out.SetSize(width,height);
  unsigned int x;
  for(x = 0; x < width; x++)
  {
    unsigned int y;
    for(y = 0; y < height; y++)
    {
      image_out(x,y)->Red   = img[x][y];
      image_out(x,y)->Green = img[x][y];
      image_out(x,y)->Blue  = img[x][y];
      image_out(x,y)->Alpha = 0;
    }
  }
  image_out.WriteToFile("output.bmp");

  //stop simulation
  sc_stop();
}

void Median_tb::read_image_uut(unsigned char img[][height])
{
  tlm::tlm_generic_payload* trans = new tlm::tlm_generic_payload;
  trans->set_read();
  // trans->set_data_length(height); // write complete column at once
  trans->set_data_length(1); // write pixel for pixel

  unsigned int len;

  for (unsigned int i = 0; i < width; i++)
  {
    for (unsigned int j = 0; j < height; ++j)
    {
      // set transmission parameters
      trans->set_address(i * height + j);  // address set to positon of pixel column
      trans->set_data_ptr( &(img[j][i]) );  // copy directly into img array, switched coords

      len = mem_if->transport_dbg(*trans);
      // cout << "Reading Pixel (" << i << ", " << j << "): " << (int)img[i][j] << "\n";
    }
  }
}
