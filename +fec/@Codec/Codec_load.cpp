/*******************************************************************************
 This file is part of FeCl.
 
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 Copyright (c) 2015, Leszek Szczecinski, INRS
 All rights reserved.
 
 FeCl is free software: you can redistribute it and/or modify
 it under the terms of the Lesser General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 FeCl is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the Lesser General Public License
 along with C3rel.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#include <mex.h>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include "Codec.h"
#include "Convolutional/Convolutional.h"
#include "Turbo/Turbo.h"
#include "Ldpc/Ldpc.h"
#include "../util/MexConversion.h"
#include "../util/MexSaveLoad.h"

using namespace fec;

const int inputCount = 1;
const int outputCount = 1;

void Codec_load( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
{
  DerivedTypeHolder<Convolutional,Turbo,Ldpc> derived;
  MexHandle<Codec> codec = load<fec::Codec>(prhs[0], derived);
  plhs[0] = toMxArray(std::move(codec));
}