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

#ifndef FEC_INTERLEAVER_H
#define FEC_INTERLEAVER_H

#include <stdint.h>

#include <vector>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

namespace fec {

/**
  * This class represents an interleaver.
  * An interleaver generates a sequence of output data where each element 
  * is picked at a specific index from the input sequence. 
  * The index is defined by the index sequence given at the construction.
  * The interleaver can interleave many independant sequences at once.
  * An interleaver is most usefull in concatenated codes such as turbo code.
  */
class Interleaver {
  friend class boost::serialization::access;
public:
  Interleaver() = default;
  /**
   * Interleaver constructor.
   *  The size of the input and output sequence is automatically detected 
   *  from the given index sequence
   *  \param  sequence  Index sequence specifying the source index of each output element.
   */
  Interleaver(const std::vector<size_t>& sequence) {
    if (sequence.size() == 0) {
      return;
    }
    sequence_ = sequence;
    inputSize_ = *std::max_element(sequence_.begin(), sequence_.end()) + 1;
    outputSize_ = sequence_.size();
  }
  /**
   * Interleaver constructor.
   *  You can specify a custom input and output seuqence size.
   *  Be carefull specifying these size because an invalid index in the index sequence
   *  will result in segfault.
   *  \param  sequence  Index sequence specifying the source index of each output element.
   *  \param  srcSize Length of the input sequence.
   *  \param  dstSize Length of the output sequence.
   */
  Interleaver(::std::vector<size_t> sequence, size_t inputSize, size_t outputSize) {
    sequence_ = sequence;
    outputSize_ = outputSize;
    inputSize_ = inputSize;
  }
  
  size_t& outputSize() {return outputSize_;}
  size_t& inputSize() {return inputSize_;}
  size_t inputSize() const {return inputSize_;}
  size_t outputSize() const {return outputSize_;}
  
  size_t operator[] (size_t i) const {return sequence_[i];}
  
  template <typename T1, typename T2=T1> void interleave(const std::vector<T1>& input, std::vector<T2>& output) const;
  template <typename T1, typename T2=T1> void deInterleave(const std::vector<T1>& input, std::vector<T2>& output) const;
  
  template <typename T1, typename T2=T1> std::vector<T1> interleave(const std::vector<T2>& input) const {
    std::vector<T2> output;
    interleave(input, output);
    return output;
  }
  template <typename T1, typename T2=T1> std::vector<T1> deInterleave(const std::vector<T2>& input) const {
    std::vector<T2> output;
    deInterleave<T1,T2>(input, output);
    return output;
  }
  
  template <typename T1, typename T2=T1> void interleaveBlocks(typename std::vector<T1>::const_iterator input, typename std::vector<T2>::iterator output, size_t n) const
  {
    for (size_t i = 0; i < n; i++) {
      interleaveBlock<T1,T2>(input, output);
      input += inputSize();
      output += outputSize();
    }
  }
  
  template <typename T1, typename T2=T1> void deInterleaveBlocks(typename std::vector<T1>::const_iterator input, typename std::vector<T2>::iterator output, size_t n) const
  {
    for (size_t i = 0; i < n; i++) {
      deInterleaveBlock<T1,T2>(input, output);
      input += outputSize();
      output += inputSize();
    }
  }
  
  template <typename T1, typename T2=T1> void interleaveBlock(typename std::vector<T1>::const_iterator input, typename std::vector<T2>::iterator output) const;
  template <typename T1, typename T2=T1> void deInterleaveBlock(typename std::vector<T1>::const_iterator input, typename std::vector<T2>::iterator output) const;
  
private:
  template <typename Archive>
  void serialize(Archive & ar, const unsigned int version) {
    using namespace boost::serialization;
    ar & BOOST_SERIALIZATION_NVP(sequence_);
  }
  
  std::vector<size_t> sequence_;
  size_t outputSize_ = 0;
  size_t inputSize_ = 0;
};
  
}

template <typename T1, typename T2>
void fec::Interleaver::interleave(const std::vector<T1>& input, std::vector<T2>& output) const
{
  output.resize(input.size() / inputSize() * outputSize());
  
  auto inputIt = input.begin();
  auto outputIt = output.begin();
  for (; inputIt < input.end(); inputIt += sequence_.size(), outputIt += sequence_.size()) {
    interleaveBlock<T1,T2>(inputIt, outputIt);
  }
}

template <typename T1, typename T2>
void fec::Interleaver::deInterleave(const std::vector<T1>& input, std::vector<T2>& output) const
{
  output.resize(input.size() / inputSize() * outputSize());
  
  auto inputIt = input.begin();
  auto outputIt = output.begin();
  for (; inputIt < input.end(); inputIt += sequence_.size(), outputIt += sequence_.size()) {
    deInterleaveBlock<T1,T2>(inputIt, outputIt);
  }
}

template <typename T1, typename T2>
void fec::Interleaver::interleaveBlock(typename std::vector<T1>::const_iterator input, typename std::vector<T2>::iterator output) const
{
  for (size_t i = 0; i < sequence_.size(); i++) {
    output[i] = input[sequence_[i]];
  }
}

template <typename T1, typename T2>
void fec::Interleaver::deInterleaveBlock(typename std::vector<T1>::const_iterator input, typename std::vector<T2>::iterator output) const
{
  for (size_t i = 0; i < sequence_.size(); i++) {
    output[sequence_[i]] = input[i];
  }
}

#endif