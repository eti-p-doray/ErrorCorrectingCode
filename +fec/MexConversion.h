/*******************************************************************************
 *  \file MexConversion.h
 *  \author Etienne Pierre-Doray
 *  \since 2015-05-27
 *  \version Last update : 2015-07-15
 *
 *  Definition of general conversion to and from mxArray
 ******************************************************************************/

#ifndef MEX_CONVERSION
#define MEX_CONVERSION

#include <memory>
#include <cstring>
#include <type_traits>
#include <vector>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <mex.h>

#include "MexAllocator.h"

template <typename T> mxClassID mexClassId() {return mxUNKNOWN_CLASS;}
template <> mxClassID mexClassId<double>() {return mxDOUBLE_CLASS;}
template <> mxClassID mexClassId<float>() {return mxSINGLE_CLASS;}
template <> mxClassID mexClassId<bool>() {return mxLOGICAL_CLASS;}
template <> mxClassID mexClassId<int8_t>() {return mxINT8_CLASS;}
template <> mxClassID mexClassId<uint8_t>() {return mxUINT8_CLASS;}
template <> mxClassID mexClassId<int16_t>() {return mxINT16_CLASS;}
template <> mxClassID mexClassId<uint16_t>() {return mxUINT16_CLASS;}
template <> mxClassID mexClassId<int32_t>() {return mxINT32_CLASS;}
template <> mxClassID mexClassId<uint32_t>() {return mxUINT32_CLASS;}
template <> mxClassID mexClassId<int64_t>() {return mxINT64_CLASS;}
template <> mxClassID mexClassId<uint64_t>() {return mxUINT64_CLASS;}


template <class T, class Enable = void>
class mxArrayTo {
public:
  static T f(const mxArray* in) {
    if (in == nullptr) {
      throw std::invalid_argument("Null mxArray");
    }
    if (mxIsComplex(in) || mxGetNumberOfElements(in) != 1 || mxGetData(in) == nullptr) {
      throw std::invalid_argument("Invalid data");
    }
    switch (mxGetClassID(in))
    {
      case mxINT8_CLASS:
        return *reinterpret_cast<int8_t*>(mxGetData(in));
        break;
        
      case mxUINT8_CLASS:
        return *reinterpret_cast<uint8_t*>(mxGetData(in));
        break;
        
      case mxINT16_CLASS:
        return *reinterpret_cast<int16_t*>(mxGetData(in));
        break;
        
      case mxUINT16_CLASS:
        return *reinterpret_cast<uint16_t*>(mxGetData(in));
        break;
        
      case mxINT32_CLASS:
        return *reinterpret_cast<int32_t*>(mxGetData(in));
        break;
        
      case mxUINT32_CLASS:
        return *reinterpret_cast<uint32_t*>(mxGetData(in));
        break;
        
      case mxINT64_CLASS:
        return *reinterpret_cast<int64_t*>(mxGetData(in));
        break;
        
      case mxUINT64_CLASS:
        return *reinterpret_cast<uint64_t*>(mxGetData(in));
        break;
        
      case mxSINGLE_CLASS:
        return *reinterpret_cast<float*>(mxGetData(in));
        break;
        
      case mxDOUBLE_CLASS:
        return *reinterpret_cast<double*>(mxGetData(in));
        break;
        
      default:
        throw std::invalid_argument("Unknown class");
        break;
    }
  }
};

template <class T>
mxArray* toMxArray(const T& in) {
  mxArray* out = mxCreateDoubleScalar(in);
  return out;
}

template <class T>
class mxCellArrayTo {
public:
  static std::vector<T> f(const mxArray* in) {
    if (in == nullptr) {
      throw std::invalid_argument("Null mxArray");
    }
    if (!mxIsCell(in)) {
      throw std::invalid_argument("Invalid data");
    }
    
    std::vector<T> out(mxGetNumberOfElements(in));
    for (size_t i = 0; i < out.size(); ++i) {
      out[i] = mxArrayTo<T>::f(mxGetCell(in, i));
    }
    return out;
  }
};

template <class T, class A, template <class, class> class V>
class mxArrayTo<V<T,A>> {
public:
  static V<T,A> f(const mxArray* in) {
    if (in == nullptr) {
      throw std::invalid_argument("Null mxArray");
    }
    if (mxIsComplex(in) || mxGetData(in) == nullptr) {
      throw std::invalid_argument("Invalid data");
    }
    V<T,A> vec(mxGetNumberOfElements(in));
    switch (mxGetClassID(in))
    {
      case mxINT8_CLASS:
        std::copy(reinterpret_cast<int8_t*>(mxGetData(in)), reinterpret_cast<int8_t*>(mxGetData(in))+vec.size(), vec.begin());
        break;
        
      case mxUINT8_CLASS:
        std::copy(reinterpret_cast<uint8_t*>(mxGetData(in)), reinterpret_cast<uint8_t*>(mxGetData(in))+vec.size(), vec.begin());
        break;
        
      case mxINT16_CLASS:
        std::copy(reinterpret_cast<int16_t*>(mxGetData(in)), reinterpret_cast<int16_t*>(mxGetData(in))+vec.size(), vec.begin());
        break;
        
      case mxUINT16_CLASS:
        std::copy(reinterpret_cast<uint16_t*>(mxGetData(in)), reinterpret_cast<uint16_t*>(mxGetData(in))+vec.size(), vec.begin());
        break;
        
      case mxINT32_CLASS:
        std::copy(reinterpret_cast<int32_t*>(mxGetData(in)), reinterpret_cast<int32_t*>(mxGetData(in))+vec.size(), vec.begin());
        break;
        
      case mxUINT32_CLASS:
        std::copy(reinterpret_cast<uint32_t*>(mxGetData(in)), reinterpret_cast<uint32_t*>(mxGetData(in))+vec.size(), vec.begin());
        break;
        
      case mxINT64_CLASS:
        std::copy(reinterpret_cast<int64_t*>(mxGetData(in)), reinterpret_cast<int64_t*>(mxGetData(in))+vec.size(), vec.begin());
        break;
        
      case mxUINT64_CLASS:
        std::copy(reinterpret_cast<uint64_t*>(mxGetData(in)), reinterpret_cast<uint64_t*>(mxGetData(in))+vec.size(), vec.begin());
        break;
        
      case mxSINGLE_CLASS:
        std::copy(reinterpret_cast<float*>(mxGetData(in)), reinterpret_cast<float*>(mxGetData(in))+vec.size(), vec.begin());
        break;
        
      case mxDOUBLE_CLASS:
        std::copy(reinterpret_cast<double*>(mxGetData(in)), reinterpret_cast<double*>(mxGetData(in))+vec.size(), vec.begin());
        break;
        
      default:
        throw std::invalid_argument("Unknown class");
        break;
    }
    return vec;
  }
};

template <class T, template <class, class> class V>
class mxArrayTo<V<T, MexAllocator<T>>> {
public:
  static V<T, MexAllocator<T>> f(const mxArray* in) {
    if (in == nullptr) {
      throw std::invalid_argument("Null mxArray");
    }
    if (mxIsComplex(in) || mxGetData(in) == nullptr || mexClassId<T>() != mxGetClassID(in)) {
      throw std::invalid_argument("Unknown class");
    }
    V<T, MexAllocator<T>> vec((MexAllocator<T>(in)));
    vec.resize(mxGetNumberOfElements(in));
    return vec;
  }
};

template <class T, template <class> class A, template <class, class> class V>
mxArray* toMxArray(const V<T, A<T>>& vec) {
  auto all = vec.get_allocator();
  mxArray* out = mxCreateNumericMatrix(vec.size(), 1, mexClassId<T>(), mxREAL);
  std::copy(vec.begin(), vec.end(), static_cast<T*>(mxGetData(out)));
  
  return out;
}

template <class T, template <class, class> class V>
mxArray* toMxArray(const V<T, MexAllocator<T>>& vec) {
  auto all = vec.get_allocator();
  mxArray* out = mxCreateNumericMatrix(0, 0, mexClassId<T>(), mxREAL);
  mxSetData(out, all.ptr());
  mxSetM(out, all.size());
  mxSetN(out, 1);
  
  return out;
}

template <class T>
class mxArrayTo<std::unique_ptr<T>> {
public:
  static std::unique_ptr<T> f(const mxArray* in) {
    if (in == nullptr) {
      throw std::invalid_argument("Null mxArray");
    }
    if (mxGetProperty(in, 0, "mexHandle_") == nullptr) {
      throw std::invalid_argument("wrong arg class");
    }
    if (mxGetData(mxGetProperty(in, 0, "mexHandle_")) == nullptr) {
      throw std::invalid_argument("Invalid data");
    }
    T* ptr = reinterpret_cast<T*>(*((uint64_t *)mxGetData(mxGetProperty(in, 0, "mexHandle_"))));
    ptr = dynamic_cast<T*>(ptr);
    if (ptr == nullptr) {
      throw std::invalid_argument("null ptr received");
    }
    return std::unique_ptr<T>(ptr);
  }
};

template <class T>
mxArray* toMxArray(std::unique_ptr<T>&& u) {
  mxArray* out = mxCreateNumericMatrix(1, 1, mxUINT64_CLASS, mxREAL);
  *((uint64_t *)mxGetData(out)) = reinterpret_cast<uint64_t>(u.release());
  
  return out;
}


template <typename T>
class mxArrayTo<T, typename std::enable_if<std::is_enum<T>::value>::type> {
public:
  static T f(const mxArray* in, const char* const enumeration[], size_t count) {
    if (in == nullptr) {
      throw std::invalid_argument("Null mxArray");
    }
    if (!mxIsChar(in)) {
      throw std::invalid_argument("Invalid data");
    }
    for (size_t i = 0; i < count; i++) {
      if (!strcmp(enumeration[i], mxArrayToString(in))) {
        return static_cast<T>(i);
      }
    }
    throw std::invalid_argument("Unknown enum");
    return static_cast<T>(0);
  }
};


class sink_counter : public boost::iostreams::sink {
public:
  sink_counter() = default;
  sink_counter(const sink_counter&) = default;
  sink_counter(size_t& count) {count_ = &count; *count_ = 0;}
  
  std::streamsize write(const char* s, std::streamsize n) {*count_ += n; return n;}
  size_t size() const {return *count_;}
private:
  size_t* count_ = nullptr;
};

const char* saveStructFieldNames[] = {"data"};

template <class T>
mxArray* save(const std::unique_ptr<T>& u)
{
  
  mxArray* save = mxCreateStructMatrix(1, 1, 1, {saveStructFieldNames});
  const T* base_pointer = u.get();
  
  size_t serialSize = 0;;
  sink_counter countSr(serialSize);
  boost::iostreams::stream< sink_counter > countSource(countSr);
  boost::archive::binary_oarchive countOa(countSource);
  
  countOa & BOOST_SERIALIZATION_NVP(base_pointer);
  
  mxArray* data = mxCreateNumericMatrix(serialSize + 4096, 1, mexClassId<uint8_t>(), mxREAL);
  
  boost::iostreams::basic_array_sink<char> sr(reinterpret_cast<char*>(mxGetData(data)), serialSize + 4096);
  boost::iostreams::stream< boost::iostreams::basic_array_sink<char> > source(sr);
  boost::archive::binary_oarchive oa(source);
  oa & BOOST_SERIALIZATION_NVP(base_pointer);
  
  mxSetField(save, 0, "data", data);
  
  return save;
}

template <class T>
std::unique_ptr<T> load(const mxArray* in)
{
  mxArray* data = mxGetField(in, 0, "data");
  if (data == nullptr) {
    std::invalid_argument("Null mxArray");
  }
  if (mxGetData(data) == nullptr) {
    std::invalid_argument("Invalid data");
  }
  size_t serialSize = mxGetNumberOfElements(data);
  
  boost::iostreams::basic_array_source<char> sr(reinterpret_cast<char*>(mxGetData(data)), serialSize);
  boost::iostreams::stream< boost::iostreams::basic_array_source<char> > source(sr);
  boost::archive::binary_iarchive ia(source);
  
  std::unique_ptr<T> u(nullptr);
  T* base_ptr;
  ia & BOOST_SERIALIZATION_NVP(base_ptr);
  u.reset(base_ptr);
  return u;
}

#endif