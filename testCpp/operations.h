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
 ******************************************************************************/

#include <vector>
#include <random>
#include <memory>
#include <cstdint>

#include "Structure/Serialization.h"
#include "Codec.h"
#include "Convolutional/Convolutional.h"
#include "Turbo/Turbo.h"
#include "Ldpc/Ldpc.h"

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& a)
{
  for (uint64_t i = 0; i < a.size(); i++) {
    os << float(a[i]) << " ";
  }
  return os;
}

template <typename T>
std::vector<fec::LlrType> distort(const std::vector<fec::BitField<T>>& input, double snrdb)
{
  const int8_t bpsk[2] = {-1, 1};
  
  double snr = pow(10.0, snrdb/10.0);
  
  uint64_t seed = 0;
  std::minstd_rand0 randomGenerator;
  randomGenerator.seed(uint32_t(seed));
  std::normal_distribution<double> normalDistribution(snr*4.0, 4.0*sqrt(snr/2.0));
  
  std::vector<fec::LlrType> llr(input.size());
  for (int i = 0; i < input.size(); i++) {
    llr[i] = bpsk[input[i]] * normalDistribution(randomGenerator);
  }
  return llr;
}

void test_encodeBlock(const fec::Codec::Structure& structure)
{
  std::vector<fec::BitField<bool>> msg0(structure.msgSize(), 0);
  std::vector<fec::BitField<uint8_t>> parity(structure.paritySize());
  structure.encode(msg0.begin(), parity.begin());
  BOOST_CHECK(structure.check(parity.begin()));
  
  std::vector<fec::BitField<bool>> msg1(structure.msgSize(), 1);
  structure.encode(msg1.begin(), parity.begin());
  BOOST_CHECK(structure.check(parity.begin()));
}

void test_encode(const fec::Codec& code, size_t n)
{
  std::vector<fec::BitField<bool>> msg(code.msgSize()*n, 1);
  std::vector<fec::BitField<uint8_t>> parity;
  code.encode(msg, parity);
  BOOST_CHECK(code.check(parity));
}

void test_encode_badMsgSize(const fec::Codec& code)
{
  std::vector<fec::BitField<bool>> msg(code.msgSize()+1);
  std::vector<fec::BitField<uint8_t>> parity;
  try {
    code.encode(msg, parity);
  } catch (std::exception& e) {
    return;
  }
  BOOST_ERROR("Exception not thrown");
}

void test_saveLoad(const fec::Codec& code) {
  DerivedTypeHolder<fec::Convolutional, fec::Turbo, fec::Ldpc> derived;
  std::vector<char> archive(archiveSize(&code, derived));
  save(&code, &archive[0], archive.size(), derived);
  auto recover = load<fec::Codec>(&archive[0], archive.size(), derived);
}

void test_decode(const fec::Codec& code, double snr, size_t n = 1)
{
  std::vector<fec::BitField<bool>> msg(code.msgSize()*n, 1);
  std::vector<fec::BitField<uint8_t>> parity(code.paritySize());
  code.encode(msg, parity);
  
  std::vector<fec::LlrType> parityIn = distort(parity, snr);
  std::vector<fec::BitField<bool>> msgOut;
  code.decode(parityIn, msgOut);
  
  BOOST_REQUIRE(msgOut.size() == code.msgSize()*n);
  for (size_t i = 0; i < msg.size(); ++i) {
    BOOST_REQUIRE(msg[i] == msgOut[i]);
  }
}

void test_decode_badParitySize(const fec::Codec& code)
{
  std::vector<fec::LlrType> parityIn(code.paritySize()+1);
  std::vector<fec::BitField<bool>> msgOut(code.msgSize());
  try {
    code.decode(parityIn, msgOut);
  } catch (std::exception& e) {
    return;
  }
  BOOST_ERROR("Exception not thrown");
}

void test_soDecode(const fec::Codec& code, double snr, size_t n = 1)
{
  std::vector<fec::BitField<bool>> msg(code.msgSize()*n, 1);
  std::vector<fec::BitField<uint8_t>> parity;
  code.encode(msg, parity);
  
  std::vector<fec::LlrType> parityIn = distort(parity, snr);
  
  std::vector<fec::LlrType> msgOut;
  code.soDecode(fec::Codec::Input<>().parity(parityIn), fec::Codec::Output<>().msg(msgOut));
  
  std::vector<fec::BitField<bool>> msgDec;
  code.decode(parityIn, msgDec);
  
  BOOST_REQUIRE(msgOut.size() == code.msgSize()*n);
  for (size_t i = 0; i < msg.size(); ++i) {
    BOOST_REQUIRE(msgDec[i] == (msgOut[i]>0));
  }
}

void test_soDecode_parityOut(const fec::Codec& code, double snr, size_t n = 1)
{
  std::vector<fec::BitField<bool>> msg(code.msgSize()*n, 1);
  std::vector<fec::BitField<uint8_t>> parity;
  code.encode(msg, parity);
  
  std::vector<fec::LlrType> parityIn = distort(parity, snr);
  std::vector<fec::LlrType> msgOut;
  std::vector<fec::LlrType> parityOut;
  code.soDecode(fec::Codec::Input<>().parity(parityIn), fec::Codec::Output<>().msg(msgOut).parity(parityOut));
  
  BOOST_REQUIRE(msgOut.size() == code.msgSize()*n);
  BOOST_REQUIRE(parityOut.size() == code.paritySize()*n);
  for (size_t i = 0; i < msg.size(); ++i) {
    BOOST_REQUIRE(msg[i] == (msgOut[i]>0));
  }
  for (size_t i = 0; i < parity.size(); ++i) {
    /*if (parity[i] != ((parityIn[i]+parityOut[i])>0)) {
      std::cout << i << std::endl;
      std::cout << parity[i] << std::endl;
      std::cout << parityIn[i] << std::endl;
      std::cout << parityOut[i] << std::endl;
    }*/
    BOOST_REQUIRE(parity[i] == ((parityIn[i]+parityOut[i])>0));
  }
}

void test_soDecode_systIn(const fec::Codec& code, double snr, size_t n = 1)
{
  std::vector<fec::BitField<bool>> msg(code.msgSize()*n, 1);
  std::vector<fec::BitField<uint8_t>> parity;
  code.encode(msg, parity);
  
  std::vector<fec::LlrType> parityIn = distort(parity, snr);
  std::vector<fec::LlrType> systIn = distort(msg, snr);
  systIn.resize(code.systSize());
  std::vector<fec::LlrType> msgOut;
  std::vector<fec::LlrType> parityOut;
  code.soDecode(fec::Codec::Input<>().parity(parityIn), fec::Codec::Output<>().msg(msgOut).parity(parityOut));
  
  size_t msgErrorCount1 = 0;
  for (size_t i = 0; i < msg.size(); ++i) {
    msgErrorCount1 += (msg[i] != msgOut[i]);
  }
  
  size_t parityErrorCount1 = 0;
  for (size_t i = 0; i < parity.size(); ++i) {
    parityErrorCount1 += (parity[i] != parityOut[i]);
  }
  
  code.soDecode(fec::Codec::Input<>().parity(parityIn).syst(systIn), fec::Codec::Output<>().msg(msgOut).parity(parityOut));
  
  size_t msgErrorCount2 = 0;
  for (size_t i = 0; i < msg.size()*n; ++i) {
    msgErrorCount2 += (msg[i] != msgOut[i]);
  }
  
  size_t parityErrorCount2 = 0;
  for (size_t i = 0; i < parity.size()*n; ++i) {
    parityErrorCount2 += (parity[i] != parityOut[i]);
  }
  
  BOOST_CHECK(msgErrorCount2 <=  msgErrorCount1);
  BOOST_CHECK(parityErrorCount2 <=  parityErrorCount1);
}

void test_soDecode_0systIn(const fec::Codec& code, size_t n = 1)
{
  double snr = -5.0;
  
  std::vector<fec::BitField<bool>> msg(code.msgSize()*n, 1);
  std::vector<fec::BitField<uint8_t>> parity;
  code.encode(msg, parity);
  
  std::vector<fec::LlrType> parityIn = distort(parity, snr);
  std::vector<fec::LlrType> msgOut1;
  std::vector<fec::LlrType> systOut1;
  std::vector<fec::LlrType> parityOut1;
  code.soDecode(fec::Codec::Input<>().parity(parityIn), fec::Codec::Output<>().msg(msgOut1).syst(systOut1).parity(parityOut1));
  
  std::vector<fec::LlrType> syst(code.systSize()*n, 0);
  std::vector<fec::LlrType> msgOut2;
  std::vector<fec::LlrType> systOut2;
  std::vector<fec::LlrType> parityOut2;
  code.soDecode(fec::Codec::Input<>().parity(parityIn).syst(syst), fec::Codec::Output<>().msg(msgOut2).syst(systOut2).parity(parityOut2));
  
  BOOST_REQUIRE(msgOut1.size() == msgOut2.size());
  BOOST_REQUIRE(systOut1.size() == systOut2.size());
  BOOST_REQUIRE(parityOut1.size() == parityOut2.size());
  for (size_t i = 0; i < msgOut1.size(); ++i) {
    BOOST_REQUIRE(msgOut1[i] == msgOut2[i]);
  }
  for (size_t i = 0; i < systOut1.size(); ++i) {
    BOOST_REQUIRE(systOut1[i] == systOut2[i]);
  }
  for (size_t i = 0; i < parityOut1.size(); ++i) {
    BOOST_REQUIRE(parityOut1[i] == parityOut2[i]);
  }
}

void test_soDecode_0stateIn(const fec::Codec& code, size_t n = 1)
{
  double snr = -5.0;
  
  std::vector<fec::BitField<bool>> msg(code.msgSize()*n, 1);
  std::vector<fec::BitField<uint8_t>> parity;
  code.encode(msg, parity);
  
  std::vector<fec::LlrType> parityIn = distort(parity, snr);
  std::vector<fec::LlrType> msgOut1;
  std::vector<fec::LlrType> systOut1;
  std::vector<fec::LlrType> parityOut1;
  code.soDecode(fec::Codec::Input<>().parity(parityIn), fec::Codec::Output<>().msg(msgOut1).syst(systOut1).parity(parityOut1));
  
  std::vector<fec::LlrType> state(code.stateSize()*n, 0);
  std::vector<fec::LlrType> msgOut2;
  std::vector<fec::LlrType> systOut2;
  std::vector<fec::LlrType> parityOut2;
  code.soDecode(fec::Codec::Input<>().parity(parityIn).state(state), fec::Codec::Output<>().msg(msgOut2).syst(systOut2).parity(parityOut2));
  
  BOOST_REQUIRE(msgOut1.size() == msgOut2.size());
  BOOST_REQUIRE(systOut1.size() == systOut2.size());
  BOOST_REQUIRE(parityOut1.size() == parityOut2.size());
  for (size_t i = 0; i < msgOut1.size(); ++i) {
    BOOST_REQUIRE(msgOut1[i] == msgOut2[i]);
  }
  for (size_t i = 0; i < systOut1.size(); ++i) {
    BOOST_REQUIRE(systOut1[i] == systOut2[i]);
  }
  for (size_t i = 0; i < parityOut1.size(); ++i) {
    BOOST_REQUIRE(parityOut1[i] == parityOut2[i]);
  }
}

void test_soDecode_2phases(const fec::Codec& code1, const fec::Codec& code2, size_t n = 1)
{
  double snr = -5.0;
  
  std::vector<fec::BitField<bool>> msg(code1.msgSize()*n, 1);
  std::vector<fec::BitField<uint8_t>> parity1;
  std::vector<fec::BitField<uint8_t>> parity2;
  code1.encode(msg, parity1);
  code2.encode(msg, parity2);
  BOOST_REQUIRE(parity1.size() == parity2.size());
  for (size_t i = 0; i < parity1.size(); ++i) {
    BOOST_REQUIRE(parity1[i] == parity2[i]);
  }
  
  std::vector<fec::LlrType> parityIn = distort(parity1, snr);
  std::vector<fec::LlrType> msgOut1;
  std::vector<fec::LlrType> systOut1;
  std::vector<fec::LlrType> parityOut1;
  std::vector<fec::LlrType> state;
  code1.soDecode(fec::Codec::Input<>().parity(parityIn), fec::Codec::Output<>().state(state));
  code1.soDecode(fec::Codec::Input<>().parity(parityIn).state(state), fec::Codec::Output<>().msg(msgOut1).syst(systOut1).parity(parityOut1));
  
  std::vector<fec::LlrType> msgOut2;
  std::vector<fec::LlrType> systOut2;
  std::vector<fec::LlrType> parityOut2;
  code2.soDecode(fec::Codec::Input<>().parity(parityIn), fec::Codec::Output<>().msg(msgOut2).syst(systOut2).parity(parityOut2));
  
  BOOST_REQUIRE(msgOut1.size() == msgOut2.size());
  BOOST_REQUIRE(systOut1.size() == systOut2.size());
  BOOST_REQUIRE(parityOut1.size() == parityOut2.size());
  for (size_t i = 0; i < msgOut1.size(); ++i) {
    BOOST_REQUIRE(msgOut1[i] == msgOut2[i]);
  }
  for (size_t i = 0; i < systOut1.size(); ++i) {
    BOOST_REQUIRE(systOut1[i] == systOut2[i]);
  }
  for (size_t i = 0; i < parityOut1.size(); ++i) {
    BOOST_REQUIRE(parityOut1[i] == parityOut2[i]);
  }
}

void test_soDecode_badParitySize(const fec::Codec& code)
{
  std::vector<fec::LlrType> parityIn(code.paritySize()+1);
  std::vector<fec::LlrType> syst;
  try {
    code.soDecode(fec::Codec::Input<>().parity(parityIn), fec::Codec::Output<>().syst(syst));
  } catch (std::exception& e) {
    return;
  }
  BOOST_ERROR("Exception not thrown");
}

void test_soDecode_badSystSize(const fec::Codec& code)
{
  std::vector<fec::LlrType> parityIn(code.paritySize());
  std::vector<fec::LlrType> systIn(code.systSize()+1);
  std::vector<fec::LlrType> syst;
  try {
    code.soDecode(fec::Codec::Input<>().syst(systIn), fec::Codec::Output<>().syst(syst));
  } catch (std::exception& e) {
    return;
  }
  BOOST_ERROR("Exception not thrown");
}

void test_soDecode_badStateSize(const fec::Codec& code)
{
  std::vector<fec::LlrType> parityIn(code.paritySize());
  std::vector<fec::LlrType> stateIn(code.stateSize()+1);
  std::vector<fec::LlrType> syst;
  try {
    code.soDecode(fec::Codec::Input<>().state(stateIn), fec::Codec::Output<>().syst(syst));
  } catch (std::exception& e) {
    return;
  }
  BOOST_ERROR("Exception not thrown");
}

void test_soDecode_noParity(const fec::Codec& code)
{
  std::vector<fec::LlrType> syst;
  try {
    code.soDecode(fec::Codec::Input<>(), fec::Codec::Output<>().syst(syst));
  } catch (std::exception& e) {
    return;
  }
  BOOST_ERROR("Exception not thrown");
}