// #param once

// #include "NvInfer.h"
// #include <cassert>
// #include <iostream>
// #include <iterator>
// #include <memory>
// #include <new>
// #include <numeric>
// #include <string>
// #include <vector>

// namespace object_detection {
//   template <typename AllocFunc, typename FreeFunc>
//   class GenericBuffer {
//   public:
//       GenericBuffer(nvinfer1::DataType type = nvinfer1::DataType::kFLOAT):
//         _size(0),
//         _capacity(0),
//         _type(type),
//         _buffer(nullptr) {}

//       GenericBuffer(size_t size, nvinfer1::DataType type):
//         _size(size),
//         _capacity(size),
//         _type(type) {
//         if(!allocFn(&_buffer, this->nbBytes())) {
//           throw std::bad_alloc();
//         }
//       }

//       GenericBuffer(GenericBuffer&& buf)
//         : _size(buf._size)
//         , _capacity(buf._capacity)
//         , _type(buf._type)
//         , _buffer(buf._buffer) {
//         buf._size = 0;
//         buf._capacity = 0;
//         buf._type = nvinfer1::DataType::kFLOAT;
//         buf._buffer = nullptr;
//       }

//       GenericBuffer& operator=(GenericBuffer&& buf) {
//         if(this != &buf) {
//           freeFn(_buffer);
//           _size = buf._size;
//           _capacity = buf._capacity;
//           _type = buf._type;
//           _buffer = buf._buffer;
//           // Reset buf.
//           buf._size = 0;
//           buf._capacity = 0;
//           buf._buffer = nullptr;
//         }
//         return *this;
//       }

//       void* data() {
//         return _buffer;
//       }

//       const void* data() const {
//         return _buffer;
//       }

//       size_t size() const {
//           return _size;
//       }

//       size_t nbBytes() const {
//         unsigned int elementSize;
//         switch(_type) {
//             case nvinfer1::DataType::kHALF: elementSize = 2; break;
//             case nvinfer1::DataType::kINT32:
//             case nvinfer1::DataType::kFLOAT: elementSize = 4; break;
//             case nvinfer1::DataType::kBOOL:
//             case nvinfer1::DataType::kINT8: elementSize = 1; break;
//             default: throw std::runtime_error("Invalid DataType.");
//         }
//         return this->size() * elementSize;
//       }

//       void resize(size_t newSize)
//       {
//         _size = newSize;
//         if (_capacity < newSize) {
//           freeFn(_buffer);
//           if (!allocFn(&_buffer, this->nbBytes())) {
//             throw std::bad_alloc{};
//           }
//           _capacity = newSize;
//         }
//       }

//       void resize(const nvinfer1::Dims& dims) {
//         return this->resize(std::accumulate(d.d, d.d + d.nbDims, 1, std::multiplies<int64_t>()));
//       }

//       ~GenericBuffer() {
//           freeFn(_buffer);
//       }

//   private:
//       size_t _size{0}, _capacity{0};
//       nvinfer1::DataType _type;
//       void* _buffer;
//       AllocFunc allocFn;
//       FreeFunc freeFn;
//   };

//   class DeviceAllocator {
//   public:
//     bool operator()(void** ptr, size_t size) const {
//         return cudaMalloc(ptr, size) == cudaSuccess;
//     }
//   };

//   class DeviceFree {
//   public:
//     void operator()(void* ptr) const {
//       cudaFree(ptr);
//     }
//   };

//   class HostAllocator {
//   public:
//     bool operator()(void** ptr, size_t size) const {
//       *ptr = malloc(size);
//       return *ptr != nullptr;
//     }
//   };

//   class HostFree {
//   public:
//     void operator()(void* ptr) const {
//       free(ptr);
//     }
//   };

//   using DeviceBuffer = GenericBuffer<DeviceAllocator, DeviceFree>;
//   using HostBuffer = GenericBuffer<HostAllocator, HostFree>;

//   class ManagedBuffer {
//   public:
//     DeviceBuffer deviceBuffer;
//     HostBuffer hostBuffer;
//   };

//   class BufferManager {
//   public:
//     static const size_t kINVALID_SIZE_VALUE = ~size_t(0);

//     BufferManager(std::shared_ptr<nvinfer1::ICudaEngine> engine, const int& batchSize, 
//       const nvinfer1::IExecutionContext* context = nullptr): 
//       mEngine(engine),
//       mBatchSize(batchSize) {
//       // Create host and device buffers
//       for(int i = 0; i < mEngine->getNbBindings(); i++) {
//         auto dims = context ? context->getBindingDimensions(i) : mEngine->getBindingDimensions(i);
//         size_t vol = context ? 1 : static_cast<size_t>(mBatchSize);
//         nvinfer1::DataType type = mEngine->getBindingDataType(i);
//         int vecDim = mEngine->getBindingVectorizedDim(i);
//         if (-1 != vecDim) // i.e., 0 != lgScalarsPerVector
//         {
//             int scalarsPerVec = mEngine->getBindingComponentsPerElement(i);
//             dims.d[vecDim] = divUp(dims.d[vecDim], scalarsPerVec);
//             vol *= scalarsPerVec;
//         }
//         vol *= samplesCommon::volume(dims);
//         std::unique_ptr<ManagedBuffer> manBuf{new ManagedBuffer()};
//         manBuf->deviceBuffer = DeviceBuffer(vol, type);
//         manBuf->hostBuffer = HostBuffer(vol, type);
//         mDeviceBindings.emplace_back(manBuf->deviceBuffer.data());
//         mManagedBuffers.emplace_back(std::move(manBuf));
//       }
//     }

//       //!
//       //! \brief Returns a vector of device buffers that you can use directly as
//       //!        bindings for the execute and enqueue methods of IExecutionContext.
//       //!
//       std::vector<void*>& getDeviceBindings()
//       {
//           return mDeviceBindings;
//       }

//       //!
//       //! \brief Returns a vector of device buffers.
//       //!
//       const std::vector<void*>& getDeviceBindings() const
//       {
//           return mDeviceBindings;
//       }

//       //!
//       //! \brief Returns the device buffer corresponding to tensorName.
//       //!        Returns nullptr if no such tensor can be found.
//       //!
//       void* getDeviceBuffer(const std::string& tensorName) const
//       {
//           return getBuffer(false, tensorName);
//       }

//       //!
//       //! \brief Returns the host buffer corresponding to tensorName.
//       //!        Returns nullptr if no such tensor can be found.
//       //!
//       void* getHostBuffer(const std::string& tensorName) const
//       {
//           return getBuffer(true, tensorName);
//       }

//       //!
//       //! \brief Returns the size of the host and device buffers that correspond to tensorName.
//       //!        Returns kINVALID_SIZE_VALUE if no such tensor can be found.
//       //!
//       size_t size(const std::string& tensorName) const
//       {
//           int index = mEngine->getBindingIndex(tensorName.c_str());
//           if (index == -1)
//               return kINVALID_SIZE_VALUE;
//           return mManagedBuffers[index]->hostBuffer.nbBytes();
//       }

//       //!
//       //! \brief Dump host buffer with specified tensorName to ostream.
//       //!        Prints error message to std::ostream if no such tensor can be found.
//       //!
//       void dumpBuffer(std::ostream& os, const std::string& tensorName)
//       {
//           int index = mEngine->getBindingIndex(tensorName.c_str());
//           if (index == -1)
//           {
//               os << "Invalid tensor name" << std::endl;
//               return;
//           }
//           void* buf = mManagedBuffers[index]->hostBuffer.data();
//           size_t bufSize = mManagedBuffers[index]->hostBuffer.nbBytes();
//           nvinfer1::Dims bufDims = mEngine->getBindingDimensions(index);
//           size_t rowCount = static_cast<size_t>(bufDims.nbDims >= 1 ? bufDims.d[bufDims.nbDims - 1] : mBatchSize);

//           os << "[" << mBatchSize;
//           for (int i = 0; i < bufDims.nbDims; i++)
//               os << ", " << bufDims.d[i];
//           os << "]" << std::endl;
//           switch (mEngine->getBindingDataType(index))
//           {
//           case nvinfer1::DataType::kINT32: print<int32_t>(os, buf, bufSize, rowCount); break;
//           case nvinfer1::DataType::kFLOAT: print<float>(os, buf, bufSize, rowCount); break;
//           case nvinfer1::DataType::kHALF: print<half_float::half>(os, buf, bufSize, rowCount); break;
//           case nvinfer1::DataType::kINT8: assert(0 && "Int8 network-level input and output is not supported"); break;
//           case nvinfer1::DataType::kBOOL: assert(0 && "Bool network-level input and output are not supported"); break;
//           }
//       }

//       //!
//       //! \brief Templated print function that dumps buffers of arbitrary type to std::ostream.
//       //!        rowCount parameter controls how many elements are on each line.
//       //!        A rowCount of 1 means that there is only 1 element on each line.
//       //!
//       template <typename T>
//       void print(std::ostream& os, void* buf, size_t bufSize, size_t rowCount)
//       {
//           assert(rowCount != 0);
//           assert(bufSize % sizeof(T) == 0);
//           T* typedBuf = static_cast<T*>(buf);
//           size_t numItems = bufSize / sizeof(T);
//           for (int i = 0; i < static_cast<int>(numItems); i++)
//           {
//               // Handle rowCount == 1 case
//               if (rowCount == 1 && i != static_cast<int>(numItems) - 1)
//                   os << typedBuf[i] << std::endl;
//               else if (rowCount == 1)
//                   os << typedBuf[i];
//               // Handle rowCount > 1 case
//               else if (i % rowCount == 0)
//                   os << typedBuf[i];
//               else if (i % rowCount == rowCount - 1)
//                   os << " " << typedBuf[i] << std::endl;
//               else
//                   os << " " << typedBuf[i];
//           }
//       }

//       //!
//       //! \brief Copy the contents of input host buffers to input device buffers synchronously.
//       //!
//       void copyInputToDevice()
//       {
//           memcpyBuffers(true, false, false);
//       }

//       //!
//       //! \brief Copy the contents of output device buffers to output host buffers synchronously.
//       //!
//       void copyOutputToHost()
//       {
//           memcpyBuffers(false, true, false);
//       }

//       //!
//       //! \brief Copy the contents of input host buffers to input device buffers asynchronously.
//       //!
//       void copyInputToDeviceAsync(const cudaStream_t& stream = 0)
//       {
//           memcpyBuffers(true, false, true, stream);
//       }

//       //!
//       //! \brief Copy the contents of output device buffers to output host buffers asynchronously.
//       //!
//       void copyOutputToHostAsync(const cudaStream_t& stream = 0)
//       {
//           memcpyBuffers(false, true, true, stream);
//       }

//       ~BufferManager() = default;

//   private:
//       void* getBuffer(const bool isHost, const std::string& tensorName) const
//       {
//           int index = mEngine->getBindingIndex(tensorName.c_str());
//           if (index == -1)
//               return nullptr;
//           return (isHost ? mManagedBuffers[index]->hostBuffer.data() : mManagedBuffers[index]->deviceBuffer.data());
//       }

//       void memcpyBuffers(const bool copyInput, const bool deviceToHost, const bool async, const cudaStream_t& stream = 0)
//       {
//           for (int i = 0; i < mEngine->getNbBindings(); i++)
//           {
//               void* dstPtr
//                   = deviceToHost ? mManagedBuffers[i]->hostBuffer.data() : mManagedBuffers[i]->deviceBuffer.data();
//               const void* srcPtr
//                   = deviceToHost ? mManagedBuffers[i]->deviceBuffer.data() : mManagedBuffers[i]->hostBuffer.data();
//               const size_t byteSize = mManagedBuffers[i]->hostBuffer.nbBytes();
//               const cudaMemcpyKind memcpyType = deviceToHost ? cudaMemcpyDeviceToHost : cudaMemcpyHostToDevice;
//               if ((copyInput && mEngine->bindingIsInput(i)) || (!copyInput && !mEngine->bindingIsInput(i)))
//               {
//                   if (async)
//                       CHECK(cudaMemcpyAsync(dstPtr, srcPtr, byteSize, memcpyType, stream));
//                   else
//                       CHECK(cudaMemcpy(dstPtr, srcPtr, byteSize, memcpyType));
//               }
//           }
//       }

//       std::shared_ptr<nvinfer1::ICudaEngine> mEngine;              //!< The pointer to the engine
//       int mBatchSize;                                              //!< The batch size
//       std::vector<std::unique_ptr<ManagedBuffer>> mManagedBuffers; //!< The vector of pointers to managed buffers
//       std::vector<void*> mDeviceBindings; //!< The vector of device buffers needed for engine execution
//   };
// }
