/*******************************************************************************
 Copyright (c) 2015, Etienne Pierre-Doray, INRS
 Copyright (c) 2015, Leszek Szczecinski, INRS
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
 
 Declaration of LlrMetrics class
 ******************************************************************************/

#ifndef FEC_LINEAR_TABLE_H
#define FEC_LINEAR_TABLE_H

#include <cmath>
#include <vector>

//chanco
namespace fec {
  
  template <typename T>
  class LinearTable {
  public:
    template <class F>
    LinearTable(size_t lenght, F f) {
      y.resize(lenght);
      for (size_t i = 0; i < y.size(); ++i) {
        y[i] = f(i);
      }
    }
    
    T operator () (T x) const {
      size_t i = x;
      return (y[i+1] - y[i]) * (x-i) + y[i];
    }
    size_t size() {
      return y.size();
    }
    
  private:
    std::vector<T> y;
  };
  
  template <typename T>
  struct Linearlog1pexpm {
    Linearlog1pexpm(T step, size_t length) : table_(length, Impl(step)) {
      step_ = step;
    }
    struct Impl {
      Impl(T step) {
        step_ = step;
      }
      T operator()(T x) {
        return std::log(1+std::exp(-double(x)/step_));
      }
      T step_;
    };
    
    T operator()(T x) {
      x*=step_;
      if(x >= table_.size()) {
        return 0;
      }
      return table_(x/step_);
    }
    
    T step_;
    LinearTable<T> table_;
  };
  
}

#endif