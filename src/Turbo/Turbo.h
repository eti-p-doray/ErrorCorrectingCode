/*******************************************************************************
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 
 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 Declaration of Turbo class
 ******************************************************************************/

#ifndef TURBO_H
#define TURBO_H

#include <boost/serialization/export.hpp>

#include "../Codec.h"
#include "../Convolutional/Convolutional.h"
#include "../Structure/Interleaver.h"

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
  enum SchedulingType {
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
    Pack,
  };
  
  class Structure;
  class EncoderOptions {
    friend class Structure;
  public:
    EncoderOptions(const std::vector<Trellis>& trellis, const std::vector<Interleaver>& interleaver) {trellis_ = trellis; interleaver_ = interleaver;}
    EncoderOptions& termination(Convolutional::TerminationType type) {terminationType_ = std::vector<Convolutional::TerminationType>(1,type); return *this;}
    EncoderOptions& termination(std::vector<Convolutional::TerminationType> type) {terminationType_ = type; return *this;}
    
  private:
    std::vector<Trellis> trellis_;
    std::vector<Interleaver> interleaver_;
    std::vector<Convolutional::TerminationType> terminationType_ = std::vector<Convolutional::TerminationType>(1,Convolutional::Tail);
  };
  class DecoderOptions {
    friend class Structure;
  public:
    DecoderOptions() = default;
    
    DecoderOptions& iterations(size_t count) {iterationCount_ = count; return *this;}
    DecoderOptions& scheduling(SchedulingType type) {schedulingType_ = type; return *this;}
    DecoderOptions& decoderType(Codec::DecoderType type) {decoderType_ = type; return *this;}
    
  private:
    size_t iterationCount_ = 6;
    SchedulingType schedulingType_ = Serial;
    Codec::DecoderType decoderType_ = Approximate;
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
    virtual ~Structure() = default;
    
    virtual const char * get_key() const;
    virtual Codec::Structure::Type type() const {return Codec::Structure::Turbo;}
    
    /**
     *  Access the size of added msg bit for the trellis termination.
     *  This is zero in the cas of trunction.
     *  \return Tail size
     */
    inline size_t msgTailSize() const {return tailSize_;}
    inline size_t constituentCount() const {return constituents_.size();}
    inline const std::vector<Convolutional::Structure>& constituents() const {return constituents_;}
    inline const std::vector<Interleaver>& interleavers() const {return interleaver_;}
    inline const Convolutional::Structure& constituent(size_t i) const {return constituents_[i];}
    inline const Interleaver& interleaver(size_t i) const {return interleaver_[i];}
    
    inline size_t iterationCount() const {return iterationCount_;}
    inline BitOrdering bitOrdering() const {return bitOrdering_;}
    inline SchedulingType scheduling() const {return schedulingType_;}
    
    virtual bool check(std::vector<BitField<uint8_t>>::const_iterator parity) const;
    virtual void encode(std::vector<BitField<bool>>::const_iterator msg, std::vector<BitField<uint8_t>>::iterator parity) const;
    
    template <typename T>
    void alternate(typename std::vector<T>::const_iterator parityIn, typename std::vector<T>::iterator parityOut);
    
    template <typename T>
    void pack(typename std::vector<T>::const_iterator parityIn, typename std::vector<T>::iterator parityOut);
    
  private:
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version) {
      using namespace boost::serialization;
      ar & ::BOOST_SERIALIZATION_BASE_OBJECT_NVP(Codec::Structure);
      ar & ::BOOST_SERIALIZATION_NVP(constituents_);
      ar & ::BOOST_SERIALIZATION_NVP(interleaver_);
      ar & ::BOOST_SERIALIZATION_NVP(tailSize_);
      ar & ::BOOST_SERIALIZATION_NVP(bitOrdering_);
      ar & ::BOOST_SERIALIZATION_NVP(schedulingType_);
    }
    
    std::vector<Convolutional::Structure> constituents_;
    std::vector<Interleaver> interleaver_;
    size_t tailSize_;
    size_t iterationCount_;
    BitOrdering bitOrdering_;
    SchedulingType schedulingType_;
  };
  
  Turbo(const Structure& structure, int workGroupSize = 4);
  virtual ~Turbo() = default;
  
  virtual const char * get_key() const;
  
  inline const Structure& structure() const {return structure_;}
  
protected:
  Turbo() = default;
  
  virtual void decodeBlocks(std::vector<LlrType>::const_iterator parity, std::vector<BitField<bool>>::iterator msg, size_t n) const;
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
