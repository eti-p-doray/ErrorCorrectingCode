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
 along with FeCl.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/

#ifndef FEC_LDPC_H
#define FEC_LDPC_H

#include <thread>
#include <random>
#include <chrono>
#include <algorithm>
#include <vector>
#include <unordered_map>

#include <boost/serialization/export.hpp>

#include "detail/Ldpc.h"
#include "Codec.h"
#include "BitMatrix.h"
#include "Permutation.h"

namespace fec {
  
  /**
   *  This class represents an ldpc encode / decoder.
   *  It offers methods encode and to decode data given an Structure.
   *
   *  The structure of the parity bits generated by an Ldpc object is as follow.
   *
   *    | syst | parity |
   *
   *  where syst are the systematic msg bits and parity are the added parity required
   *  to create a consistent bloc.
   *
   *  The structure of the extrinsic information is the case
   *
   *    | check1 | check2 | ... |
   *
   *  where checkX are the messages at the last iteration from the check node X
   *  that would be transmitted to the connected bit nodes at the next iteration.
   */
  class Ldpc : public Codec
  {
    friend class boost::serialization::access;
  public:
    struct Gallager {
      static SparseBitMatrix matrix(size_t n, size_t wc, size_t wr, uint64_t seed = 0);
    };
    struct DvbS2 {
    public:
      static SparseBitMatrix matrix(size_t n, double rate);
      
    private:
      static const std::array<size_t, 2> length_;
      static const std::vector<std::vector<double>> rate_;
      static const std::vector<std::vector<size_t>> parameter_;
      static const std::vector<std::vector<std::vector<std::vector<size_t>>>> index_;
    };
    
    using EncoderOptions = detail::Ldpc::EncoderOptions;
    using DecoderOptions = detail::Ldpc::DecoderOptions;
    using PunctureOptions = detail::Ldpc::PunctureOptions;
    
    Ldpc() = default;
    Ldpc(const detail::Ldpc::Structure& structure, int workGroupSize = 8);
    Ldpc(const EncoderOptions& encoder, const DecoderOptions& decoder, int workGroupSize = 8);
    Ldpc(const EncoderOptions& encoder, int workGroupSize = 8);
    Ldpc(const Ldpc& other) {*this = other;}
    virtual ~Ldpc() = default;
    Ldpc& operator=(const Ldpc& other) {Codec::operator=(other); structure_ = std::unique_ptr<detail::Ldpc::Structure>(new detail::Ldpc::Structure(other.structure())); return *this;}
    
    virtual const char * get_key() const;
    
    void setDecoderOptions(const DecoderOptions& decoder) {structure().setDecoderOptions(decoder);}
    DecoderOptions getDecoderOptions() const {return structure().getDecoderOptions();}
    
    Permutation puncturing(const PunctureOptions& options) {return structure().puncturing(options);}
    
  protected:
    Ldpc(std::unique_ptr<detail::Ldpc::Structure>&& structure, int workGroupSize = 4) : Codec(std::move(structure), workGroupSize) {}
    
    inline const detail::Ldpc::Structure& structure() const {return dynamic_cast<const detail::Ldpc::Structure&>(Codec::structure());}
    inline detail::Ldpc::Structure& structure() {return dynamic_cast<detail::Ldpc::Structure&>(Codec::structure());}
    
    void decodeBlocks(detail::Codec::const_iterator<double> first, detail::Codec::const_iterator<double> last, detail::Codec::iterator<BitField<size_t>> output) const override;
    void soDecodeBlocks(detail::Codec::const_iterator<double> first, detail::Codec::const_iterator<double> last, detail::Codec::iterator<double> output) const override;
    
  private:
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version);
  };
  
}

BOOST_CLASS_EXPORT_KEY(fec::Ldpc);
BOOST_CLASS_TYPE_INFO(fec::Ldpc,extended_type_info_no_rtti<fec::Ldpc>);


template <typename Archive>
void fec::Ldpc::serialize(Archive & ar, const unsigned int version) {
  using namespace boost::serialization;
  ar.template register_type<detail::Ldpc::Structure>();
  ar & ::BOOST_SERIALIZATION_BASE_OBJECT_NVP(Codec);
}

#endif
