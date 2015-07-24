/*******************************************************************************
 *  \file TurboCodeImpl.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-07-22
 *  \version Last update : 2015-07-24
 *
 *  Declaration of TurboCodeImpl class
 ******************************************************************************/

#ifndef TURBO_CODE_IMPL_H
#define TURBO_CODE_IMPL_H

#include "../Code.h"
#include "../CodeStructure/TurboCodeStructure.h"
#include "../ConvolutionalCode/ConvolutionalCode.h"
#include "../ConvolutionalCode/MapDecoder/MapDecoder.h"

namespace fec {

/*******************************************************************************
 *  This class represents a map encode / decoder
 *  It offers methods encode and to decode data giving a posteriori informations 
 *  using a trellis.
 ******************************************************************************/
class TurboCodeImpl
{
  friend class TurboCode;
public:
  TurboCodeImpl(const TurboCodeStructure& codeStructure) : codeStructure_(codeStructure) {
    for (size_t i = 0; i < codeStructure_.structureCount(); ++i) {
      code_.push_back(MapDecoder::create(codeStructure_.structure(i)));
    }
  }
  virtual ~TurboCodeImpl() = default;
  
  
  size_t extrinsicSize() const {
    size_t extrinsicSize = 0;
    switch (codeStructure_.structureType()) {
      default:
      case TurboCodeStructure::Serial:
        return codeStructure_.msgSize();
        break;
        
      case TurboCodeStructure::Parallel:
        for (auto & i : codeStructure_.structures()) {
          extrinsicSize += i.msgSize();
        }
        return extrinsicSize;
        break;
    }
  }
  const CodeStructure& structure() const {return codeStructure_;}
  
protected:
  TurboCodeImpl() = default;
  
  void appDecodeNBloc(boost::container::vector<LlrType>::const_iterator parityIn, boost::container::vector<LlrType>::const_iterator extrinsicIn, boost::container::vector<LlrType>::iterator messageOut, boost::container::vector<LlrType>::iterator extrinsicOut, size_t n) const;

private:
  
  void serialDecodeBloc(boost::container::vector<LlrType>::const_iterator parityIn, boost::container::vector<LlrType>::const_iterator extrinsicIn, boost::container::vector<LlrType>::iterator messageOut, boost::container::vector<LlrType>::iterator extrinsicOut) const;
  void parallelDecodeBloc(boost::container::vector<LlrType>::const_iterator parityIn, boost::container::vector<LlrType>::const_iterator extrinsicIn, boost::container::vector<LlrType>::iterator messageOut, boost::container::vector<LlrType>::iterator extrinsicOut) const;
  
  boost::container::vector<std::unique_ptr<MapDecoder>> code_;
  TurboCodeStructure codeStructure_;
};
  
}

#endif
