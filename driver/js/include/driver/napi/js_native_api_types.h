/*
 *
 * Tencent is pleased to support the open source community by making
 * Hippy available.
 *
 * Copyright (C) 2019 THL A29 Limited, a Tencent company.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include "dom/dom_argument.h"
#include "dom/dom_event.h"
#include "driver/base/common.h"
#include "driver/base/js_value_wrapper.h"
#include "footstone/hippy_value.h"
#include "footstone/logging.h"


class Scope;

namespace hippy {
namespace napi {

constexpr char kErrorHandlerJSName[] = "ExceptionHandle.js";
constexpr char kHippyErrorHandlerName[] = "HippyExceptionHandler";

enum PropertyAttribute {
  /** None. **/
  None = 0,
  /** ReadOnly, i.e., not writable. **/
  ReadOnly = 1 << 0,
  /** DontEnum, i.e., not enumerable. **/
  DontEnum = 1 << 1,
  /** DontDelete, i.e., not configurable. **/
  DontDelete = 1 << 2
};

class CallbackInfo;
using JsCallback = std::function<void(const CallbackInfo& info)>;

// Map: FunctionName -> Callback (e.g. "Log" -> ConsoleModule::Log)
using ModuleClass =
    std::unordered_map<footstone::stringview::unicode_string_view, hippy::napi::JsCallback>;

// Map: ClassName -> ModuleClass (e.g. "ConsoleModule" -> [ModuleClass])
using ModuleClassMap =
    std::unordered_map<footstone::stringview::unicode_string_view, ModuleClass>;

enum Encoding {
  UNKNOWN_ENCODING,
  ONE_BYTE_ENCODING,
  TWO_BYTE_ENCODING,
  UTF8_ENCODING
};

class CtxValue;
class CBCtxValueTuple {
 public:
  CBCtxValueTuple(const void *data,
                  const std::shared_ptr<CtxValue> arguments[],
                  size_t count)
      : data_(data), arguments_(arguments), count_(count) {}
  const void *data_;
  const std::shared_ptr<CtxValue> *arguments_;
  size_t count_;
};

class CtxValue {
 public:
  CtxValue() {}
  virtual ~CtxValue() {}
};

template <typename T>
using GetterCallback = std::function<std::shared_ptr<CtxValue>(T* thiz)>;

template <typename T>
using SetterCallback = std::function<void(T* thiz, const std::shared_ptr<CtxValue>& value)>;

template <typename T>
using FunctionCallback = std::function<std::shared_ptr<CtxValue>(
    T* thiz,
    size_t argument_count,
    const std::shared_ptr<CtxValue> arguments[])>;

template <typename T>
using InstanceConstructor = std::function<std::shared_ptr<T>(size_t argument_count,
                                                             const std::shared_ptr<CtxValue> arguments[])>;

template <typename T>
struct PropertyDefine {
  using unicode_string_view = footstone::stringview::unicode_string_view;

  GetterCallback<T> getter;
  SetterCallback<T> setter;
  unicode_string_view name;
};

template <typename T>
struct FunctionDefine {
  using unicode_string_view = footstone::stringview::unicode_string_view;

  FunctionCallback<T> cb;
  unicode_string_view name;
};

template <typename T>
struct InstanceDefine {
  using unicode_string_view = footstone::stringview::unicode_string_view;

  InstanceConstructor<T> constructor;
  std::vector<PropertyDefine<T>> properties{};
  std::vector<FunctionDefine<T>> functions{};
  unicode_string_view name;
  std::unordered_map<void*, std::shared_ptr<T>> holder;
};

class Ctx {
 public:
  using JSValueWrapper = hippy::base::JSValueWrapper;
  using unicode_string_view = footstone::stringview::unicode_string_view;
  using HippyValue = footstone::value::HippyValue;
  using NativeFunction = std::function<std::shared_ptr<hippy::napi::CtxValue>(void *)>;

  Ctx() {}
  virtual ~Ctx() { FOOTSTONE_DLOG(INFO) << "~Ctx"; }
  virtual bool RegisterGlobalInJs() = 0;
  virtual void RegisterClasses(std::weak_ptr<Scope> scope) = 0;
  virtual void RegisterDomEvent(std::weak_ptr<Scope> scope, const std::shared_ptr<CtxValue> callback, std::shared_ptr<DomEvent>& dom_event) = 0;
  virtual bool SetGlobalJsonVar(const unicode_string_view& name,
                                const unicode_string_view& json) = 0;
  virtual bool SetGlobalStrVar(const unicode_string_view& name,
                               const unicode_string_view& str) = 0;
  virtual bool SetGlobalObjVar(const unicode_string_view& name,
                               const std::shared_ptr<CtxValue>& obj,
                               const PropertyAttribute& attr) = 0;
  virtual std::shared_ptr<CtxValue> GetGlobalStrVar(
      const unicode_string_view& name) = 0;
  virtual std::shared_ptr<CtxValue> GetGlobalObjVar(
      const unicode_string_view& name) = 0;
  virtual bool SetProperty(const std::shared_ptr<CtxValue>& object,
                           const unicode_string_view& prop_key,
                           const std::shared_ptr<CtxValue>& value,
                           const PropertyAttribute& attr) = 0;
  virtual std::shared_ptr<CtxValue> GetProperty(
      const std::shared_ptr<CtxValue>& object,
      const unicode_string_view& name) = 0;
  virtual bool DeleteProperty(const std::shared_ptr<CtxValue>& object,
                              const unicode_string_view& name) = 0;

  virtual void RegisterGlobalModule(const std::shared_ptr<Scope>& scope,
                                    const ModuleClassMap& modules) = 0;
  virtual void RegisterNativeBinding(const unicode_string_view& name,
                                     hippy::base::RegisterFunction fn,
                                     void* data) = 0;
  virtual void RegisterNativeBinding(const unicode_string_view& name,
                                     NativeFunction fn,
                                     void* data) = 0;

  virtual std::shared_ptr<CtxValue> CreateNumber(double number) = 0;
  virtual std::shared_ptr<CtxValue> CreateBoolean(bool b) = 0;
  virtual std::shared_ptr<CtxValue> CreateString(
      const unicode_string_view& string) = 0;
  virtual std::shared_ptr<CtxValue> CreateUndefined() = 0;
  virtual std::shared_ptr<CtxValue> CreateNull() = 0;
  virtual std::shared_ptr<CtxValue> ParseJson(const unicode_string_view& json) = 0;
  virtual std::shared_ptr<CtxValue> CreateObject(const std::unordered_map<
      unicode_string_view,
      std::shared_ptr<CtxValue>>& object) = 0;
  virtual std::shared_ptr<CtxValue> CreateObject(const std::unordered_map<
      std::shared_ptr<CtxValue>,
      std::shared_ptr<CtxValue>>& object) = 0;
  virtual std::shared_ptr<CtxValue> CreateMap(const std::map<
      std::shared_ptr<CtxValue>,
      std::shared_ptr<CtxValue>>& map) = 0;
  virtual std::shared_ptr<CtxValue> CreateArray(
      size_t count,
      std::shared_ptr<CtxValue> value[]) = 0;
  virtual std::shared_ptr<CtxValue> CreateError(
      const unicode_string_view& msg) = 0;
  virtual std::shared_ptr<CtxValue> CreateByteBuffer(
      const void* buffer, size_t length) = 0;

  // Get From Value
  virtual std::shared_ptr<CtxValue> CallFunction(
      const std::shared_ptr<CtxValue>& function,
      size_t argument_count,
      const std::shared_ptr<CtxValue> arguments[]) = 0;

  virtual bool GetValueNumber(const std::shared_ptr<CtxValue>& value,
                              double* result) = 0;
  virtual bool GetValueNumber(const std::shared_ptr<CtxValue>& value,
                              int32_t* result) = 0;
  virtual bool GetValueBoolean(const std::shared_ptr<CtxValue>& value,
                               bool* result) = 0;
  virtual bool GetValueString(const std::shared_ptr<CtxValue>& value,
                              unicode_string_view* result) = 0;
  virtual bool GetValueJson(const std::shared_ptr<CtxValue>& value,
                            unicode_string_view* result) = 0;
  virtual bool IsNullOrUndefined(const std::shared_ptr<CtxValue>& value) = 0;

  virtual bool IsMap(const std::shared_ptr<CtxValue>& value) = 0;

  virtual bool IsString(const std::shared_ptr<CtxValue>& value) = 0;

  virtual bool IsNumber(const std::shared_ptr<CtxValue>& value) = 0;

  //buffer
  virtual bool IsByteBuffer(const std::shared_ptr<CtxValue>& value) = 0;
  virtual bool GetByteBuffer(const std::shared_ptr<CtxValue>& value,
                             void** out_data,
                             size_t& out_length,
                             uint32_t& out_type) = 0;

  // Array Helpers
  virtual bool IsArray(const std::shared_ptr<CtxValue>& value) = 0;
  virtual uint32_t GetArrayLength(const std::shared_ptr<CtxValue>& value) = 0;
  virtual std::shared_ptr<CtxValue> CopyArrayElement(const std::shared_ptr<CtxValue>& value,
                                                     uint32_t index) = 0;

  // Object Helpers
  virtual bool IsObject(const std::shared_ptr<CtxValue>& value) = 0;
  //Currently, we only support the case where the 'key' is string type.
  virtual bool GetEntriesFromObject(const std::shared_ptr<CtxValue>& value,
                                    std::unordered_map<unicode_string_view, std::shared_ptr<CtxValue>> &map) = 0;

  virtual bool HasNamedProperty(const std::shared_ptr<CtxValue>& value,
                                const unicode_string_view& name) = 0;
  virtual std::shared_ptr<CtxValue> CopyNamedProperty(
      const std::shared_ptr<CtxValue>& value,
      const unicode_string_view& name) = 0;
  // Function Helpers

  virtual bool IsFunction(const std::shared_ptr<CtxValue>& value) = 0;
  virtual unicode_string_view CopyFunctionName(
      const std::shared_ptr<CtxValue>& value) = 0;

  virtual std::shared_ptr<CtxValue> RunScript(
      const unicode_string_view& data,
      const unicode_string_view& file_name) = 0;
  virtual std::shared_ptr<CtxValue> GetJsFn(
      const unicode_string_view& name) = 0;

  virtual void ThrowException(const std::shared_ptr<CtxValue> &exception) = 0;
  virtual void ThrowException(const unicode_string_view& exception) = 0;
  virtual void HandleUncaughtException(const std::shared_ptr<CtxValue>& exception) = 0;

  virtual std::shared_ptr<JSValueWrapper> ToJsValueWrapper(
      const std::shared_ptr<CtxValue>& value) = 0;
  virtual std::shared_ptr<CtxValue> CreateCtxValue(
      const std::shared_ptr<JSValueWrapper>& wrapper) = 0;

  virtual std::shared_ptr<HippyValue> ToDomValue(
      const std::shared_ptr<CtxValue>& value) = 0;
  virtual std::shared_ptr<DomArgument> ToDomArgument(
      const std::shared_ptr<CtxValue>& value) = 0;
  virtual std::shared_ptr<CtxValue> CreateCtxValue(
      const std::shared_ptr<HippyValue>& value) = 0;

  virtual bool Equals(const std::shared_ptr<CtxValue>& lhs, const std::shared_ptr<CtxValue>& rhs) = 0;
};

struct VMInitParam {};

class VM {
 public:
  VM(std::shared_ptr<VMInitParam> param = nullptr){}
  virtual ~VM() { FOOTSTONE_DLOG(INFO) << "~VM"; }

  virtual std::shared_ptr<Ctx> CreateContext() = 0;
};

class TryCatch {
 public:
  explicit TryCatch(bool enable = false, std::shared_ptr<Ctx> ctx = nullptr)
      : enable_(enable), ctx_(ctx) {}
  virtual ~TryCatch() {}
  virtual void ReThrow() = 0;
  virtual bool HasCaught() = 0;
  virtual bool CanContinue() = 0;
  virtual bool HasTerminated() = 0;
  virtual bool IsVerbose() = 0;
  virtual void SetVerbose(bool verbose) = 0;
  virtual std::shared_ptr<CtxValue> Exception() = 0;
  virtual footstone::stringview::unicode_string_view GetExceptionMsg() = 0;

 protected:
  bool enable_;
  std::shared_ptr<Ctx> ctx_;
};

class BindingData {
 public:
  std::weak_ptr<Scope> scope_;
  ModuleClassMap map_;

  BindingData(std::weak_ptr<Scope> scope, ModuleClassMap map)
      : scope_(scope), map_(map) {}
};

class FunctionData {
 public:
  std::weak_ptr<Scope> scope_;
  JsCallback callback_;

  FunctionData(std::weak_ptr<Scope> scope, JsCallback callback)
      : scope_(scope), callback_(callback) {}
};

}  // namespace napi
}  // namespace hippy