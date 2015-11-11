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

#ifndef FEC_CONVOLUTIONAL_H
#define FEC_CONVOLUTIONAL_H

#include "Codec.h"
#include "Trellis.h"
#include "Permutation.h"

namespace fec {
  
  /**
   *  This class represents a convolutional encode / decoder.
   *  It offers methods encode and to decode data given a Structure.
   *
   *  The structure of the parity bits generated by a ConvolutionalCodec object is as follows
   *
   *    | parity1 | parity2 | ... | tail1 | tail2 ... |
   *
   *  where parityX is the output symbol sequence of the branch used at stage X
   *  in the trellis, tailX is are the output symbol sequence of the tail branch
   *  at stage X.
   *
   *  The structure of the extrinsic information
   *
   *    | msg | systTail |
   *
   *  where msg are the extrinsic msg L-values generated by the map decoder
   *  and systTail are the tail bit added to the message for trellis termination.
   */
  class Convolutional : public Codec
  {
    friend class boost::serialization::access;
  public:
    /**
     *  Trellis termination types.
     *  This specifies the type of termination at the end of each bloc.
     */
    enum Termination {
      Tail, /**< The state is brought to zero by implicitly adding new msg bit */
      Truncate /**< The state is forced to zero by truncating the trellis */
    };
    
    struct EncoderOptions {
    public:
      EncoderOptions(const Trellis& trellis, size_t length) {trellis_ = trellis; length_ = length;}
      EncoderOptions& termination(Termination type) {termination_ = type; return *this;}
      
      Trellis trellis_;
      size_t length_;
      Termination termination_ = Truncate;
    };
    struct DecoderOptions {
    public:
      DecoderOptions() = default;
      
      DecoderOptions& algorithm(DecoderAlgorithm algorithm) {algorithm_ = algorithm; return *this;}
      DecoderOptions& scalingFactor(double scalingFactor) {scalingFactor_ = scalingFactor; return *this;}
      
      DecoderAlgorithm algorithm_ = Approximate;
      double scalingFactor_ = 1.0;
    };
    struct PunctureOptions {
    public:
      PunctureOptions() = default;
      
      PunctureOptions& mask(std::vector<bool> mask) {mask_ = mask; return *this;}
      PunctureOptions& tailMask(std::vector<bool> mask) {tailMask_ = mask; return *this;}
      
      std::vector<bool> mask_;
      std::vector<bool> tailMask_;
    };
    struct Options : public EncoderOptions, DecoderOptions
    {
    public:
      Options(const Trellis& trellis, size_t length) : EncoderOptions(trellis, length) {}
    };
    
    struct detail {
      /**
       *  This class represents a convolutional codec structure.
       *  It provides a usefull interface to store and acces the structure information.
       */
      class Structure : public Codec::detail::Structure {
        friend class ::boost::serialization::access;
      public:
        Structure() = default;
        Structure(const Options& options);
        Structure(const EncoderOptions& encoder, const DecoderOptions& decoder);
        Structure(const EncoderOptions& encoder);
        virtual ~Structure() = default;
        
        virtual const char * get_key() const;
        
        void setDecoderOptions(const DecoderOptions& decoder);
        DecoderOptions getDecoderOptions() const;
        
        inline size_t length() const {return length_;}
        inline size_t tailSize() const {return tailSize_;}
        inline size_t systTailSize() const {return tailSize_ * trellis().inputSize();}
        inline Termination termination() const {return termination_;}
        inline const Trellis& trellis() const {return trellis_;}
        
        fec::LlrType scalingFactor() const {return scalingFactor_;} /**< Access the scalingFactor value used in decoder. */
        void setScalingFactor(fec::LlrType factor) {scalingFactor_ = factor;} /**< Modify the scalingFactor value used in decoder. */
        
        virtual bool check(std::vector<BitField<size_t>>::const_iterator parity) const;
        virtual void encode(std::vector<BitField<size_t>>::const_iterator msg, std::vector<BitField<size_t>>::iterator parity) const;
        void encode(std::vector<BitField<size_t>>::const_iterator msg, std::vector<BitField<size_t>>::iterator parity, std::vector<BitField<size_t>>::iterator tail) const;
        
        Permutation puncturing(const PunctureOptions& options) const;
        
      protected:
        void setEncoderOptions(const EncoderOptions& encoder);
        
      private:
        template <typename Archive>
        void serialize(Archive & ar, const unsigned int version) {
          using namespace boost::serialization;
          ar & ::BOOST_SERIALIZATION_BASE_OBJECT_NVP(Codec::detail::Structure);
          ar & ::BOOST_SERIALIZATION_NVP(trellis_);
          ar & ::BOOST_SERIALIZATION_NVP(termination_);
          ar & ::BOOST_SERIALIZATION_NVP(tailSize_);
          ar & ::BOOST_SERIALIZATION_NVP(length_);
          ar & ::BOOST_SERIALIZATION_NVP(scalingFactor_);
        }
        
        Trellis trellis_;
        size_t length_;
        Termination termination_;
        size_t tailSize_;
        fec::LlrType scalingFactor_;
      };
    };
    
    Convolutional() = default;
    Convolutional(const Options& options, int workGroupSize = 8);
    Convolutional(const detail::Structure& structure, int workGroupSize = 8);
    Convolutional(const EncoderOptions& encoder, const DecoderOptions& decoder, int workGroupSize = 8);
    Convolutional(const EncoderOptions& encoder, int workGroupSize = 8);
    Convolutional(const Convolutional& other) {*this = other;}
    virtual ~Convolutional() = default;
    Convolutional& operator=(const Convolutional& other) {Codec::operator=(other); structure_ = std::unique_ptr<detail::Structure>(new detail::Structure(other.structure())); return *this;}
    
    virtual const char * get_key() const;
    
    void setDecoderOptions(const DecoderOptions& decoder) {structure().setDecoderOptions(decoder);}
    DecoderOptions getDecoderOptions() const {return structure().getDecoderOptions();}
    
    Permutation puncturing(const PunctureOptions& options) {return structure().puncturing(options);}
    
  protected:
    Convolutional(std::unique_ptr<detail::Structure>&& structure, int workGroupSize = 4) : Codec(std::move(structure), workGroupSize) {}
    
    inline const detail::Structure& structure() const {return dynamic_cast<const detail::Structure&>(Codec::structure());}
    inline detail::Structure& structure() {return dynamic_cast<detail::Structure&>(Codec::structure());}
    
    virtual void decodeBlocks(std::vector<LlrType>::const_iterator parity, std::vector<BitField<size_t>>::iterator msg, size_t n) const;
    virtual void soDecodeBlocks(Codec::detail::InputIterator input, Codec::detail::OutputIterator output, size_t n) const;
    
  private:
    template <typename Archive>
    void serialize(Archive & ar, const unsigned int version) {
      using namespace boost::serialization;
      ar.template register_type<detail::Structure>();
      ar & ::BOOST_SERIALIZATION_BASE_OBJECT_NVP(Codec);
    }
  };
  
}

BOOST_CLASS_TYPE_INFO(fec::Convolutional,extended_type_info_no_rtti<fec::Convolutional>);
BOOST_CLASS_EXPORT_KEY(fec::Convolutional);
BOOST_CLASS_TYPE_INFO(fec::Convolutional::detail::Structure,extended_type_info_no_rtti<fec::Convolutional::detail::Structure>);
BOOST_CLASS_EXPORT_KEY(fec::Convolutional::detail::Structure);

#endif