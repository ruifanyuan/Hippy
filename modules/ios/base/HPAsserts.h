/*!
 * iOS SDK
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
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if __OBJC__
#import <Foundation/Foundation.h>
#endif
#import "MacroDefines.h"
#import "HPDriverStackFrame.h"
#import "HPToolUtils.h"

NS_ASSUME_NONNULL_BEGIN

/**
 * This is the main assert macro that you should use. Asserts should be compiled out
 * in production builds. You can customize the assert behaviour by setting a custom
 * assert handler through `HPSetAssertFunction`.
 */
#ifndef NS_BLOCK_ASSERTIONS
#define HPAssert(condition, ...)                                                                                           \
    do {                                                                                                                      \
        if ((condition) == 0) {                                                                                               \
            _HPAssertFormat(#condition, __FILE__, __LINE__, __func__, __VA_ARGS__);                                        \
            if (HP_NSASSERT) {                                                                                             \
                [[NSAssertionHandler currentHandler] handleFailureInFunction:@(__func__) file:@(__FILE__) lineNumber:__LINE__ \
                                                                 description:__VA_ARGS__];                                    \
            }                                                                                                                 \
        }                                                                                                                     \
    } while (false)
#else
#define HPAssert(condition, ...) \
    do {                            \
    } while (false)
#endif
HP_EXTERN void _HPAssertFormat(const char *, const char *, int, const char *, NSString *, ...) NS_FORMAT_FUNCTION(5, 6);

#define HPAssertUnimplemented()                             \
  do {                                                      \
    HPLogError(@"Not implemented in: %s", __FUNCTION__)     \
    abort();                                                \
  } while (0)

/**
 * Report a fatal condition when executing. These calls will _NOT_ be compiled out
 * in production, and crash the app by default. You can customize the fatal behaviour
 * by setting a custom fatal handler through `HPSetFatalHandler`.
 */
HP_EXTERN void HPFatal(NSError *error, NSDictionary *__nullable userInfo);

HP_EXTERN void HPHandleException(NSException *exception, NSDictionary *userInfo);

/**
 * The default error domain to be used for HP errors.
 */
HP_EXTERN NSString *const HPErrorDomain;

/**
 * JS Stack trace provided as part of an NSError's userInfo
 */
HP_EXTERN NSString *const HPJSStackTraceKey;

/**
 * Raw JS Stack trace string provided as part of an NSError's userInfo
 */
HP_EXTERN NSString *const HPJSRawStackTraceKey;

/**
 * Name of fatal exceptions generated by HPFatal
 */
HP_EXTERN NSString *const HPFatalExceptionName;

/**
 * Module Name of fatal exceptions generated by HPFatal
 */
HP_EXTERN NSString *const HPFatalModuleName;

/**
 * A block signature to be used for custom assertion handling.
 */
typedef void (^HPAssertFunction)(NSString *condition, NSString *fileName, NSNumber *lineNumber, NSString *function, NSString *message);

typedef void (^HPFatalHandler)(NSError *error, NSDictionary *userInfo);

typedef void (^HPExceptionHandler)(NSException *e);

/**
 * Convenience macro for asserting that a parameter is non-nil/non-zero.
 */
#define HPAssertParam(name) HPAssert(name, @"'%s' is a required parameter", #name)

/**
 * Convenience macro for asserting that we're running on main queue.
 */
#define HPAssertMainQueue() HPAssert(HPIsMainQueue(), @"This function must be called on the main thread")

/**
 * Convenience macro for asserting that we're running off the main queue.
 */
#define HPAssertNotMainQueue() HPAssert(!HPIsMainQueue(), @"This function must not be called on the main thread")

/**
 * These methods get and set the current assert function called by the HPAssert
 * macros. You can use these to replace the standard behavior with custom assert
 * functionality.
 */
HP_EXTERN void HPSetAssertFunction(HPAssertFunction assertFunction);
HP_EXTERN HPAssertFunction HPGetAssertFunction(void);

/**
 * This appends additional code to the existing assert function, without
 * replacing the existing functionality. Useful if you just want to forward
 * assert info to an extra service without changing the default behavior.
 */
HP_EXTERN void HPAddAssertFunction(HPAssertFunction assertFunction);

/**
 * This method temporarily overrides the assert function while performing the
 * specified block. This is useful for testing purposes (to detect if a given
 * function asserts something) or to suppress or override assertions temporarily.
 */
HP_EXTERN void HPPerformBlockWithAssertFunction(void (^block)(void), HPAssertFunction assertFunction);

/**
 * Get the current thread's name (or the current queue, if in debug mode)
 */
HP_EXTERN NSString *HPCurrentThreadName(void);

/**
 These methods get and set the current fatal handler called by the HPFatal method.
 */
HP_EXTERN void HPSetFatalHandler(HPFatalHandler fatalHandler);
HP_EXTERN HPFatalHandler HPGetFatalHandler(void);

HP_EXTERN void HPSetExceptionHandler(HPExceptionHandler exceptionhandler);
HP_EXTERN HPExceptionHandler HPGetExceptionHandler(void);

HP_EXTERN NSString *HPFormatError(NSString *message, NSArray<HPDriverStackFrame *> *stackTrace, NSUInteger maxMessageLength);

/**
 * Convenience macro to assert which thread is currently running (DEBUG mode only)
 */
#ifdef DEBUG

#define HPAssertThread(thread, format...)                                                                                      \
    _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wdeprecated-declarations\"")                            \
        HPAssert([(id)thread isKindOfClass:[NSString class]]   ? [HPCurrentThreadName() isEqualToString:(NSString *)thread] \
                    : [(id)thread isKindOfClass:[NSThread class]] ? [NSThread currentThread] == (NSThread *)thread                \
                                                                  : dispatch_get_current_queue() == (dispatch_queue_t)thread,     \
            format);                                                                                                              \
    _Pragma("clang diagnostic pop")

#else

#define HPAssertThread(thread, format...) \
    do {                                     \
    } while (0)

#endif


NS_ASSUME_NONNULL_END