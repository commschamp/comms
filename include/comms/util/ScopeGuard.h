//
// Copyright 2012 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief Contains definition of the "Scope Guard" idiom, see @ref comms::util::ScopeGuard.

#pragma once

#include <functional>
#include <memory>
#include <type_traits>

namespace comms
{

namespace util
{

/// @brief Implements <a href="https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Scope_Guard">Scope Guard Idiom</a>.
/// @details Scope guard idiom allows to call
///          any function with any number of parameters when the guard is
///          destructed, unless release() method is called prior to the
///          destruction. The scope guard doesn't use any dynamic memory
///          allocation and takes as much space on the stack as needed
///          to bind the provided function with all its arguments.
///          The template parameter must be type of the functor
///          that doesn't receive any parameters and doesn't return any value.
///          In order to properly create such guard use makeScopeGuard()
///          function. For example:
///          @code
///          // Binding function with parameters:
///          auto guard = comms::util::makeScopeGuard(&func, std::ref(arg1), arg2);
///
///          // Binding lamda function:
///          auto guard =
///              comms::util::makeScopeGuard([&argByRef, argByValue]()
///                  {
///                      ...// Some code here
///                  });
///          @endcode
///          Note that all the bound parameters are passed by value, if
///          there is any need to bind function with reference to some
///          object, use "std::ref()" or "std::cref()" for const reference.
///          Also note that the guard doesn't provide copy constructor and
///          assignment operator, it supports only move semantics.
/// @tparam TFunc Functor object type.
/// @headerfile comms/util/ScopeGuard.h
template <typename TFunc>
class ScopeGuard
{
public:
    /// @brief Constructor
    /// @param[in] func Functor that will be executed when the scope guard is
    ///            destructed unless it is "released." Must provide move/copy
    ///            constructor.
    explicit ScopeGuard(TFunc&& func)
        : m_func(std::forward<TFunc>(func)),
          m_engaged(true)
    {
    }


    /// @brief No copy is allowed.
    ScopeGuard(const ScopeGuard& guard) = delete;

    /// @brief Move constructor
    /// @details After the functor is moved, it will be released in the
    ///          provided guard.
    /// @param[in] guard The other scope guard of the same type.
    ScopeGuard(ScopeGuard&& guard)
        : m_func(std::move(guard.m_func)),
          m_engaged(std::move(guard.m_engaged))
    {
        guard.release();
    }


    /// @brief Destructor
    /// @post The functor is called unless it was released with release()
    ///       prior to destruction.
    ~ScopeGuard() noexcept
    {
        if (!isReleased()) {
            m_func();
        }
    }

    /// @brief No copy is allowed.
    ScopeGuard& operator=(const ScopeGuard& guard) = delete;

    /// @brief Release the bound functor.
    /// @post The functor won't be called when the scope guard is out of scope.
    void release()
    {
        m_engaged = false;
    }

    /// @brief Check whether the functor is released.
    /// @return true in case of being released.
    bool isReleased() const
    {
        return !m_engaged;
    }

private:
    typename std::remove_reference<TFunc>::type m_func;
    bool m_engaged;
};

/// @brief Create scope guard with provided functor.
/// @details Use this function to create a scope guard with lambda function.
///          For example:
///          @code
///          auto guard =
///              comms::util::makeScopeGuard([&argByRef, argByValue]()
///                  {
///                      ...// Some code here
///                  });
///          @endcode
/// @tparam TFunctor Functor type, should be deduced automatically based on
///         provided argument.
/// @param[in] func Functor
/// @return Scope guard.
/// @related ScopeGuard
template <typename TFunctor>
ScopeGuard<TFunctor> makeScopeGuard(TFunctor&& func)
{
    return ScopeGuard<TFunctor>(std::forward<TFunctor>(func));
}

/// @brief Create scope guard by binding the provided function and
///        all the arguments.
/// @details Use this function to create a scope guard when some function
///          with one or more arguments needs to be called.
///          For example:
///          @code
///          // Binding function with parameters:
///          auto guard = comms::util::makeScopeGuard(&func, std::ref(arg1), arg2);
///          @endcode
///          Note that all the bound parameters are passed by value, if there
///          is any need to bind function with reference to some object,
///          use "std::ref()" or "std::cref()" for const reference.
///          Also note that this function uses variadic template arguments which
///          were introduced in C++11. Please make sure that you compiler
///          supports it.
/// @tparam TFunc Pointer to function type.
/// @tparam TParams Types of other arguments.
/// @param[in] func Functor
/// @param[in] args Function arguments
/// @return Scope guard.
/// @related ScopeGuard
template <typename TFunc,
          typename... TParams>
auto makeScopeGuard(TFunc&& func, TParams... args) ->
ScopeGuard<decltype(std::bind(std::forward<TFunc>(func),
                    std::forward<TParams>(args)...))>
{
    auto bindObj = std::bind(std::forward<TFunc>(func), std::forward<TParams>(args)...);
    return ScopeGuard<decltype(bindObj)>(std::move(bindObj));
}

}  // namespace util

}  // namespace comms
