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

#ifndef FEC_TURBO_H
#define FEC_TURBO_H

#include <boost/serialization/export.hpp>

#include "../Codec.h"
#include "../Convolutional/Convolutional.h"
#include "../Structure/Permutation.h"

namespace fec {
  
  /**
   *  This class represents a turbo encode / decoder.
   *  It offers methods encode and to decode data given a Turbo::Structure.
   *
   *  The structure of the parity bits generated by a Turbo object is as follow
   *
   *    | syst | systTail | convOutput1 | tailOutpu1 | convOutput2 | tailOutpu2 | ... |
   *
   *  where syst are the systematic bits, systTail are the tail bit
   *  added to the msg for termination of the constituents 1, 2, ..., respectively,
   *  convOutputX and tailOutputX are the output parity of the  msg and the tail
   *  generated by the constituent convolutional code X.
   *
   *  The structure of the extrinsic information is the case of serial decoding
   *
   *    | msg | systTail |
   *
   *  where msg are the extrinsic msg L-values generated by the last constituent
   *  code involved in each msg bit.
   *
   *  And in the case of parallel decoding
   *
   *    | msg1 | systTail1 | msg2 | systTail2 | ... |
   *
   *  where msgX are the extrinsic msg L-values generated by the constituent code X
   *  and systTailX are the tail bit added to the msg
   *  for termination of the constituent X.
   */
  class Turbo : public Codec
  {
    friend class boost::serialization::access;
  public:
    
    /**
     *  Algorithm used in decoding.
     *  This defines the scheduling of extrinsic communication between code
     *    constituents.
     */
    enum Scheduling {
      Serial,/**< Each constituent tries to decode and gives its extrinsic
              information to the next constituent in a serial behavior. */
      Parallel,/**< Each constituent tries to decode in parallel.
                The extrinsic information is then combined and shared to every
                constituents similar to the Belief Propagation algorithm used in ldpc. */
    };
    /**
     *  Algorithm used in decoding.
     *  This defines the scheduling of extrinsic communication between code
     *    constituents.
     */
    enum BitOrdering {
      Alternate,
      Group,
    };
    
    struct EncoderOptions {
      friend class Structure;
    public:
      EncoderOptions(const std::vector<Trellis>& trellis, const std::vector<Permutation>& interleaver) {trellis_ = trellis; interleaver_ = interleaver;}
      EncoderOptions(const Trellis& trellis, const std::vector<Permutation>& interleaver) {trellis_ = {trellis}; interleaver_ = interleaver;}
      EncoderOptions& termination(Convolutional::Termination type) {termination_ = {type}; return *this;}
      EncoderOptions& termination(std::vector<Convolutional::Termination> type) {termination_ = type; return *this;}
      
      std::vector<Trellis> trellis_;
      std::vector<Permutation> interleaver_;
      std::vector<Convolutional::Termination> termination_ = std::vector<Convolutional::Termination>(1,Convolutional::Tail);
    };
    struct DecoderOptions {
      friend class Structure;
    public:
      DecoderOptions() = default;
      
      DecoderOptions& iterations(size_t count) {iterations_ = count; return *this;}
      DecoderOptions& scheduling(Scheduling type) {scheduling_ = type; return *this;}
      DecoderOptions& algorithm(Codec::DecoderAlgorithm algorithm) {algorithm_ = algorithm; return *this;}
      
      size_t iterations_ = 6;
      Scheduling scheduling_ = Serial;
      Codec::DecoderAlgorithm algorithm_ = Approximate;
    };
    struct PermuteOptions {
    public:
      PermuteOptions() = default;
      
      PermuteOptions& systMask(std::vector<bool> mask) {systMask_ = mask; return *this;}
      PermuteOptions& systTailMask(std::vector<bool> mask) {systTailMask_ = {mask}; return *this;}
      PermuteOptions& systTailMask(std::vector<std::vector<bool>> mask) {systTailMask_ = mask; return *this;}
      PermuteOptions& parityMask(std::vector<bool> mask) {parityMask_ = {mask}; return *this;}
      PermuteOptions& parityMask(std::vector<std::vector<bool>> mask) {parityMask_ = mask; return *this;}
      PermuteOptions& tailMask(std::vector<bool> mask) {tailMask_ = {mask}; return *this;}
      PermuteOptions& tailMask(std::vector<std::vector<bool>> mask) {tailMask_ = mask; return *this;}
      PermuteOptions& bitOrdering(BitOrdering ordering) {bitOrdering_ = ordering; return *this;}
      
      std::vector<bool> systMask_;
      std::vector<std::vector<bool>> systTailMask_;
      std::vector<std::vector<bool>> parityMask_;
      std::vector<std::vector<bool>> tailMask_;
      BitOrdering bitOrdering_ = Alternate;
    };
    /**
     *  This class represents a convolutional code structure.
     *  It provides a usefull interface to store and acces the structure information.
     */
    class Structure : public Codec::Structure {
      friend class ::boost::serialization::access;
    public:
      Structure() = default;
      Structure(const EncoderOptions&, const DecoderOptions&);
      Structure(const EncoderOptions&);
      virtual ~Structure() = default;
      
      virtual const char * get_key() const;
      virtual Codec::Structure::Type type() const {return Codec::Structure::Turbo;}
      
      void setDecoderOptions(const DecoderOptions& decoder);
      void setEncoderOptions(const EncoderOptions& encoder);
      DecoderOptions getDecoderOptions() const;
      
      /**
       *  Access the size of added msg bit for the trellis termination.
       *  This is zero in the cas of trunction.
       *  \return Tail size
       */
      inline size_t systTailSize() const {return tailSize_;}
      inline size_t constituentCount() const {return constituents_.size();}
      inline const std::vector<Convolutional::Structure>& constituents() const {return constituents_;}
      inline const std::vector<Permutation>& interleavers() const {return interleaver_;}
      inline const Convolutional::Structure& constituent(size_t i) const {return constituents_[i];}
      inline const Permutation& interleaver(size_t i) const {return interleaver_[i];}
      
      inline size_t iterations() const {return iterations_;}
      inline Scheduling scheduling() const {return scheduling_;}
      
      virtual bool check(std::vector<BitField<size_t>>::const_iterator parity) const;
      virtual void encode(std::vector<BitField<size_t>>::const_iterator msg, std::vector<BitField<size_t>>::iterator parity) const;
      
      Permutation createPermutation(const PermuteOptions& options) const;
      
    private:
      template <typename Archive>
      void serialize(Archive & ar, const unsigned int version) {
        using namespace boost::serialization;
        ar & ::BOOST_SERIALIZATION_BASE_OBJECT_NVP(Codec::Structure);
        ar & ::BOOST_SERIALIZATION_NVP(constituents_);
        ar & ::BOOST_SERIALIZATION_NVP(interleaver_);
        ar & ::BOOST_SERIALIZATION_NVP(tailSize_);
        ar & ::BOOST_SERIALIZATION_NVP(iterations_);
        ar & ::BOOST_SERIALIZATION_NVP(scheduling_);
      }
      
      std::vector<Convolutional::Structure> constituents_;
      std::vector<Permutation> interleaver_;
      size_t tailSize_;
      size_t iterations_;
      Scheduling scheduling_;
    };
    
    Turbo(const Structure& structure, int workGroupSize = 8);
    Turbo(const EncoderOptions& encoder, const DecoderOptions& decoder, int workGroupSize = 8);
    Turbo(const EncoderOptions& encoder, int workGroupSize = 8);
    Turbo(const Turbo& other) : Codec(&structure_) {*this = other;}
    virtual ~Turbo() = default;
    
    virtual const char * get_key() const;
    
    inline const Structure& structure() const {return structure_;}
    void setDecoderOptions(const DecoderOptions& decoder) {structure_.setDecoderOptions(decoder);}
    void setEncoderOptions(const EncoderOptions& encoder) {structure_.setEncoderOptions(encoder);}
    DecoderOptions getDecoderOptions() const {return structure_.getDecoderOptions();}
    
    Permutation createPermutation(const PermuteOptions& options) {return structure_.createPermutation(options);}
    
  protected:
    Turbo() = default;
    
    virtual void decodeBlocks(std::vector<LlrType>::const_iterator parity, std::vector<BitField<size_t>>::iterator msg, size_t n) const;
    virtual void soDecodeBlocks(InputIterator input, OutputIterator output, size_t n) const;
    
  private:
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version) {
      using namespace boost::serialization;
      ar & ::BOOST_SERIALIZATION_NVP(structure_);
      ar.template register_type<Structure>();
      ar & ::BOOST_SERIALIZATION_BASE_OBJECT_NVP(Codec);
    }
    
    Structure structure_;
  };
  
}

BOOST_CLASS_EXPORT_KEY(fec::Turbo);
BOOST_CLASS_TYPE_INFO(fec::Turbo,extended_type_info_no_rtti<fec::Turbo>);
BOOST_CLASS_EXPORT_KEY(fec::Turbo::Structure);
BOOST_CLASS_TYPE_INFO(fec::Turbo::Structure,extended_type_info_no_rtti<fec::Turbo::Structure>);


#endif
