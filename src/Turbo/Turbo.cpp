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

#include "Turbo.h"
#include "TurboDecoder/TurboDecoder.h"

using namespace fec;

BOOST_CLASS_EXPORT_IMPLEMENT(Turbo);
BOOST_CLASS_EXPORT_IMPLEMENT(Turbo::Structure);

const char * Turbo::get_key() const {
  return boost::serialization::type_info_implementation<Turbo>::type::get_const_instance().get_key();
}

const char * Turbo::Structure::get_key() const {
  return boost::serialization::type_info_implementation<Turbo::Structure>::type::get_const_instance().get_key();
}

/*******************************************************************************
 *  Turbo constructor
 *  \param  codeStructure Codec structure used for encoding and decoding
 *  \param  workGroupSize Number of thread used for decoding
 ******************************************************************************/
Turbo::Turbo(const Structure& structure,  int workGroupSize) :
structure_(structure),
Codec(&structure_, workGroupSize)
{
}
Turbo::Turbo(const EncoderOptions& encoder, const DecoderOptions& decoder, int workGroupSize) :
structure_(encoder, decoder),
Codec(&structure_, workGroupSize)
{
}
Turbo::Turbo(const EncoderOptions& encoder, int workGroupSize) :
structure_(encoder),
Codec(&structure_, workGroupSize)
{
}

void Turbo::decodeBlocks(std::vector<LlrType>::const_iterator parity, std::vector<BitField<size_t>>::iterator msg, size_t n) const
{
  auto worker = TurboDecoder::create(structure_);
  worker->decodeBlocks(parity, msg, n);
}

void Turbo::soDecodeBlocks(InputIterator input, OutputIterator output, size_t n) const
{
  auto worker = TurboDecoder::create(structure_);
  worker->soDecodeBlocks(input, output, n);
}


Turbo::Structure::Structure(const EncoderOptions& encoder, const DecoderOptions& decoder)
{
  setEncoderOptions(encoder);
  setDecoderOptions(decoder);
}
Turbo::Structure::Structure(const EncoderOptions& encoder)
{
  setEncoderOptions(encoder);
  setDecoderOptions(DecoderOptions());
}

void Turbo::Structure::setEncoderOptions(const fec::Turbo::EncoderOptions &encoder)
{
  interleaver_ = encoder.interleaver_;
  if (encoder.trellis_.size() != 1 && encoder.trellis_.size() != interleaver_.size()) {
    throw std::invalid_argument("Trellis and Permutation count don't match");
  }
  if (encoder.termination_.size() != 1 && encoder.termination_.size() != interleaver_.size()) {
    throw std::invalid_argument("Termination and Permutation count don't match");
  }
  
  msgSize_ = 0;
  for (size_t i = 0; i < interleaver_.size(); ++i) {
    if (interleaver_[i].inputSize() > msgSize()) {
      msgSize_ = interleaver_[i].inputSize();
    }
  }
  
  constituents_.clear();
  for (size_t i = 0; i < interleaver_.size(); ++i) {
    if (interleaver_[i].outputSize() == 0) {
      std::vector<size_t> tmp(msgSize());
      for (size_t j = 0; j < tmp.size(); ++j) {
        tmp[j] = j;
      }
      interleaver_[i] = tmp;
    }
    size_t j = i;
    if (encoder.trellis_.size() == 1) {
      j = 0;
    }
    size_t length = interleaver_[i].outputSize() / encoder.trellis_[j].inputSize();
    if (length * encoder.trellis_[j].inputSize() != interleaver_[i].outputSize()) {
      throw std::invalid_argument("Invalid size for interleaver");
    }
    auto encoderConstituentOptions = Convolutional::EncoderOptions(encoder.trellis_[j], length);
    if (encoder.termination_.size() == 1) {
      encoderConstituentOptions.termination(encoder.termination_[0]);
    }
    else {
      encoderConstituentOptions.termination(encoder.termination_[i]);
    }
    auto decoderConstituentOptions = Convolutional::DecoderOptions().algorithm(decoderAlgorithm_);
    constituents_.push_back(Convolutional::Structure(encoderConstituentOptions, decoderConstituentOptions));
  }
  
  systSize_ = 0;
  paritySize_ = 0;
  tailSize_ = 0;
  stateSize_ = 0;
  for (auto & i : constituents()) {
    tailSize_ += i.systTailSize();
    paritySize_ += i.paritySize();
    stateSize_ += i.systSize();
  }
  systSize_ = msgSize_ + systTailSize();
  paritySize_ += systSize();
}

void Turbo::Structure::setDecoderOptions(const fec::Turbo::DecoderOptions &decoder)
{
  iterations_ = decoder.iterations_;
  scheduling_ = decoder.scheduling_;
  decoderAlgorithm_ = decoder.algorithm_;
  for (size_t i = 0; i < interleaver_.size(); ++i) {
    auto constituentOptions = Convolutional::DecoderOptions().algorithm(decoder.algorithm_);
    constituents_[i].setDecoderOptions(constituentOptions);
  }
}

Turbo::DecoderOptions Turbo::Structure::getDecoderOptions() const
{
  return DecoderOptions().iterations(iterations()).scheduling(scheduling()).algorithm(decoderAlgorithm());
}

void Turbo::Structure::encode(std::vector<BitField<size_t>>::const_iterator msg, std::vector<BitField<size_t>>::iterator parity) const
{
  std::vector<BitField<size_t>> messageInterl;
  std::vector<BitField<size_t>> parityOut;
  std::vector<BitField<size_t>>::iterator parityOutIt;
  parityOutIt = parity;
  std::copy(msg, msg + msgSize(), parityOutIt);
  auto systTail = parityOutIt + msgSize();
  parityOutIt += systSize();
  for (size_t i = 0; i < constituentCount(); ++i) {
    messageInterl.resize(constituent(i).msgSize());
    interleaver(i).permuteBlock<BitField<size_t>>(msg, messageInterl.begin());
    constituent(i).encode(messageInterl.begin(), parityOutIt, systTail);
    systTail += constituent(i).systTailSize();
    parityOutIt += constituent(i).paritySize();
  }
}

bool Turbo::Structure::check(std::vector<BitField<size_t>>::const_iterator parity) const
{
  std::vector<BitField<size_t>> messageInterl;
  std::vector<BitField<size_t>> parityTest;
  std::vector<BitField<size_t>> tailTest;
  std::vector<BitField<size_t>> parityIn;
  std::vector<BitField<size_t>>::const_iterator parityInIt;
  parityInIt = parity;
  auto tailIt = parityInIt + msgSize();
  auto systIt = parityInIt;
  parityInIt += systSize();
  for (size_t i = 0; i < constituentCount(); ++i) {
    messageInterl.resize(constituent(i).msgSize());
    parityTest.resize(constituent(i).paritySize());
    tailTest.resize(constituent(i).systTailSize());
    interleaver(i).permuteBlock<BitField<size_t>,BitField<size_t>>(systIt, messageInterl.begin());
    constituent(i).encode(messageInterl.begin(), parityTest.begin(), tailTest.begin());
    if (!std::equal(parityTest.begin(), parityTest.end(), parityInIt)) {
      return false;
    }
    if (!std::equal(tailTest.begin(), tailTest.end(), tailIt)) {
      return false;
    }
    parityInIt += constituent(i).paritySize();
    tailIt += constituent(i).systTailSize();
  }
  return true;
}

Permutation Turbo::Structure::createPermutation(const PermuteOptions& options) const
{
  std::vector<bool> systMask_ = options.systMask_;
  if (systMask_.size() == 0) {
    systMask_ = {true};
  }
  std::vector<std::vector<bool>> systTailMask_ = options.systTailMask_;
  if (systTailMask_.size() == 0) {
    
  } else if (systTailMask_.size() == 1) {
    systTailMask_.resize(constituentCount(), systTailMask_[0]);
  } else if (systTailMask_.size() == constituentCount()) {
    for (size_t i = 0; i < constituentCount(); ++i) {
      if (systTailMask_[i].size() == 0) {
        systTailMask_[i] = {true};
      }
    }
  } else {
    throw std::invalid_argument("Invalid size for systematic tail mask");
  }
  
  std::vector<std::vector<bool>> parityMask_ = options.parityMask_;
  if (parityMask_.size() == 0) {
    parityMask_.resize(constituentCount(), {true});
  } else if (parityMask_.size() == 1) {
    parityMask_.resize(constituentCount(), parityMask_[0]);
  } else if (parityMask_.size() == constituentCount()) {
    for (size_t i = 0; i < constituentCount(); ++i) {
      if (parityMask_[i].size() == 0) {
        parityMask_[i] = {true};
      }
    }
  } else {
    throw std::invalid_argument("Invalid size for parity mask");
  }
  std::vector<std::vector<bool>> tailMask_ = options.tailMask_;
  if (tailMask_.size() == 0) {
    
  } else if (tailMask_.size() == 1) {
    tailMask_.resize(constituentCount(), tailMask_[0]);
  } else if (tailMask_.size() == constituentCount()) {
    for (size_t i = 0; i < constituentCount(); ++i) {
      if (tailMask_[i].size() == 0) {
        tailMask_[i] = {true};
      }
    }
  } else {
    throw std::invalid_argument("Invalid size for tail mask");
  }
  
  
  std::vector<size_t> perms;
  switch (options.bitOrdering_) {
    case Alternate: {
      size_t systIdx = 0;
      for (size_t i = 0; i < msgSize(); ++i) {
        if (options.systMask_[systIdx % options.systMask_.size()]) {
          perms.push_back(systIdx);
        }
        ++systIdx;
        size_t parityBaseIdx = systSize();
        for (size_t j = 0; j < constituentCount(); ++j) {
          if (i < constituent(j).length()) {
            for (size_t k = 0; k < constituent(j).trellis().outputSize(); ++k) {
              size_t parityIdx = i*constituent(j).trellis().outputSize()+k;
              if (options.parityMask_[j][parityIdx % options.parityMask_[j].size()]) {
                perms.push_back(parityBaseIdx+parityIdx);
              }
            }
          }
          parityBaseIdx += constituent(j).paritySize();
        }
      }
      size_t parityIdx = systSize();
      for (size_t i = 0; i < constituentCount(); ++i) {
        parityIdx += constituent(i).paritySize() - constituent(i).tailSize() * constituent(i).trellis().outputSize();
        size_t tailIdx = 0;
        size_t systTailIdx = 0;
        for (size_t j = 0; j < constituent(i).tailSize(); ++j) {
          for (size_t k = 0; k < constituent(i).trellis().inputSize(); ++k) {
            if ((options.systTailMask_.size() == 0 && (options.systMask_[systIdx % options.systMask_.size()])) ||
                (options.systTailMask_.size() != 0 && (options.systTailMask_[i][systTailIdx % options.systTailMask_.size()]))) {
              perms.push_back(systIdx);
            }
            ++systIdx;
            ++systTailIdx;
          }
          for (size_t k = 0; k < constituent(i).trellis().outputSize(); ++k) {
            if ((options.tailMask_.size() == 0 && (options.parityMask_[i][parityIdx % options.systMask_.size()])) ||
                (options.tailMask_.size() != 0 && (options.tailMask_[i][tailIdx % options.systTailMask_.size()]))) {
              perms.push_back(parityIdx);
            }
            ++parityIdx;
            ++tailIdx;
          }
        }
      }
      break;
    }
      
    case Group: {
      size_t idx = 0;
      for (size_t i = 0; i < msgSize(); ++i) {
        if (options.systMask_[idx % options.systMask_.size()]) {
          perms.push_back(idx);
        }
        ++idx;
      }
      for (size_t i = 0; i < constituentCount(); ++i) {
        size_t tailIdx = 0;
        for (size_t j = 0; j < constituent(i).tailSize() * constituent(i).trellis().inputSize(); ++j) {
            if ((options.systTailMask_.size() == 0 && (options.systMask_[idx % options.systMask_.size()])) ||
                (options.systTailMask_.size() != 0 && (options.systTailMask_[i][tailIdx % options.systTailMask_.size()]))) {
              perms.push_back(idx);
            }
            ++idx;
            ++tailIdx;
        }
      }
      for (size_t i = 0; i < constituentCount(); ++i) {
        size_t parityIdx = 0;
        for (size_t j = 0; j < constituent(i).paritySize() * constituent(i).trellis().outputSize(); ++j) {
            if (options.parityMask_[j][parityIdx % options.parityMask_[j].size()]) {
              perms.push_back(idx);
            }
            ++idx;
        }
        size_t tailIdx = 0;
        for (size_t j = 0; j < constituent(i).tailSize() * constituent(i).trellis().outputSize(); ++j) {
          if ((options.tailMask_.size() == 0 && (options.parityMask_[i][parityIdx % options.systMask_.size()])) ||
              (options.tailMask_.size() != 0 && (options.tailMask_[i][tailIdx % options.systTailMask_.size()]))) {
            perms.push_back(idx);
          }
          ++idx;
          ++parityIdx;
          ++tailIdx;
        }
      }
      break;
    }
      
    default:
      break;
  }
  
  return Permutation(perms, paritySize());
}
