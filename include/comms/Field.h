//
// Copyright 2014 - 2025 (C). Alex Robenko. All rights reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

/// @file
/// @brief Contains definition of @ref comms::Field class.

#pragma once

#include "comms/details/FieldBase.h"
#include "comms/details/field_alias.h"
#include "comms/details/fields_access.h"
#include "comms/details/macro_common.h"
#include "comms/util/access.h"

#include <type_traits>

namespace comms
{

/// @brief Base class to all the field classes.
/// @details Every custom "field" class should inherit from this one.
/// @tparam TOptions Zero or more options. The supported options are:
///     @li comms::option::def::BigEndian or comms::option::def::LittleEndian - Option to
///         specify serialisation endian. If none is provided "Big" endian is
///         assumed.
/// @headerfile comms/Field.h
template <typename... TOptions>
class Field : public details::FieldBase<TOptions...>
{
    using BaseImpl = details::FieldBase<TOptions...>;
public:
    /// @brief Endian type
    /// @details Equal to either @ref comms::traits::endian::Big or
    ///     @ref comms::traits::endian::Little
    using Endian = typename BaseImpl::Endian;

    /// @brief Version type
    using VersionType = typename BaseImpl::VersionType;

    /// @brief Default validity check
    /// @details Always returns true, can be overriden by the derived class
    /// @return Always @b true
    static constexpr bool valid()
    {
        return true;
    }

    /// @brief Default refresh functionality
    /// @details Does nothing and returns false, can be overriden by the
    ///     derived class
    /// @return Always @b false
    static constexpr bool refresh()
    {
        return false;
    }

    /// @brief Default check of whether the field is version dependent.
    /// @return Always @b false.
    static constexpr bool isVersionDependent()
    {
        return false;
    }

    /// @brief Default check of whether the field has defines refresh functionality
    /// @return Always @b false.
    static constexpr bool hasNonDefaultRefresh()
    {
        return false;
    }

    /// @brief Default check of whether the field has @b readNoStatus() member function
    /// @return Always @b true.
    static constexpr bool hasReadNoStatus()
    {
        return true;
    }

    /// @brief Default check of whether the field has @b writeNoStatus() member function
    /// @return Always @b true.
    static constexpr bool hasWriteNoStatus()
    {
        return true;
    }

    /// @brief Default check of whether the field has variable length definition
    ///     via @ref comms::option::def::VarLength option.
    /// @return Always @b false.
    static constexpr bool hasVarLength()
    {
        return false;
    }         

    /// @brief Default check of whether the field has a consistent value
    ///     for writing.
    /// @return Always @b true.
    static constexpr bool canWrite()
    {
        return true;
    }

    /// @brief Default version update functionality
    /// @details Does nothing and returns false, can be overriden by the
    ///     derived class
    /// @return Always @b false
    static constexpr bool setVersion(VersionType)
    {
        return false;
    }

    /// @brief Check of whether the field class defines @b name() function
    /// @return Always @b false
    static constexpr bool hasName()
    {
        return false;
    }     

protected:
    /// @brief Write data into the output buffer.
    /// @details Use this function to write data to the the buffer
    ///          maintained by the caller. The endianness of the data will be
    ///          as specified in the options provided to the class.
    /// @tparam T Type of the value to write. Must be integral.
    /// @tparam Type of output iterator
    /// @param[in] value Integral type value to be written.
    /// @param[in, out] iter Output iterator.
    /// @pre The iterator must be valid and can be successfully dereferenced
    ///      and incremented at least sizeof(T) times.
    /// @post The iterator is advanced.
    /// @note Thread safety: Safe for distinct buffers, unsafe otherwise.
    template <typename T, typename TIter>
    static void writeData(T value, TIter& iter)
    {
        writeData<sizeof(T), T>(value, iter);
    }

    /// @brief Write partial data into the output buffer.
    /// @details Use this function to write partial data to the buffer maintained
    ///          by the caller. The endianness of the data will be as specified
    ///          the class options.
    /// @tparam TSize Length of the value in bytes known in compile time.
    /// @tparam T Type of the value to write. Must be integral.
    /// @tparam TIter Type of output iterator
    /// @param[in] value Integral type value to be written.
    /// @param[in, out] iter Output iterator.
    /// @pre TSize <= sizeof(T)
    /// @pre The iterator must be valid and can be successfully dereferenced
    ///      and incremented at least TSize times.
    /// @post The iterator is advanced.
    /// @note Thread safety: Safe for distinct buffers, unsafe otherwise.
    template <std::size_t TSize, typename T, typename TIter>
    static void writeData(T value, TIter& iter)
    {
        static_assert(TSize <= sizeof(T),
                                    "Cannot put more bytes than type contains");
        return util::writeData<TSize, T>(value, iter, Endian());
    }

    /// @brief Read data from input buffer.
    /// @details Use this function to read data from the intput buffer maintained
    ///     by the caller. The endianness of the data will be as specified in
    ///     options of the class.
    /// @tparam T Return type
    /// @tparam TIter Type of input iterator
    /// @param[in, out] iter Input iterator.
    /// @return The integral type value.
    /// @pre TSize <= sizeof(T)
    /// @pre The iterator must be valid and can be successfully dereferenced
    ///      and incremented at least sizeof(T) times.
    /// @post The iterator is advanced.
    /// @note Thread safety: Safe for distinct stream buffers, unsafe otherwise.
    template <typename T, typename TIter>
    static T readData(TIter& iter)
    {
        return readData<T, sizeof(T)>(iter);
    }

    /// @brief Read partial data from input buffer.
    /// @details Use this function to read data from the intput buffer maintained
    ///     by the caller. The endianness of the data will be as specified in
    ///     options of the class.
    /// @tparam T Return type
    /// @tparam TSize number of bytes to read
    /// @tparam TIter Type of input iterator
    /// @param[in, out] iter Input iterator.
    /// @return The integral type value.
    /// @pre TSize <= sizeof(T)
    /// @pre The iterator must be valid and can be successfully dereferenced
    ///      and incremented at least TSize times.
    /// @post The internal pointer of the stream buffer is advanced.
    /// @note Thread safety: Safe for distinct stream buffers, unsafe otherwise.
    template <typename T, std::size_t TSize, typename TIter>
    static T readData(TIter& iter)
    {
        static_assert(TSize <= sizeof(T),
            "Cannot get more bytes than type contains");
        return util::readData<T, TSize>(iter, Endian());
    }

};

/// @brief Add convenience access enum and functions to the members of
///     composite fields, such as @ref comms::field::Bundle or @ref comms::field::Bitfield.
/// @details Very similar to @ref COMMS_FIELD_MEMBERS_NAMES(), but does @b NOT
///     require definition of @b Base inner member type (for some compilers) and does @b NOT
///     define inner @b Field_* types for used member fields.
/// @param[in] ... List of fields' names.
/// @see @ref COMMS_FIELD_MEMBERS_NAMES()
/// @note Relevant to @ref comms::field::Bundle and @ref comms::field::Bitfield
/// @note Defined in "comms/Field.h"
/// @warning Some compilers, such as @b clang or early versions of @b g++
///     may have problems compiling code generated by this macro even
///     though it uses valid C++11 constructs in attempt to automatically identify the
///     type of the base class. If the compilation fails,
///     and this macro resides inside a @b NON-template class, please use
///     @ref COMMS_FIELD_MEMBERS_ACCESS_NOTEMPLATE() macro instead. In
///     case this macro needs to reside inside a @b template class, then
///     there is still a need to define inner @b Base type, which specifies
///     exact type of the @ref comms::field::Bitfield or @ref comms::field::Bundle class.
///     @code
///     template <typename... TExtraOptions>
///     class MyField : public
///         comms::field::Bitfield<
///             MyFieldBase,
///             std::tuple<Field1, Field2, Field3>,
///             TExtraOptions...
///         >
///     {
///         // Define type of the base class
///         using Base =
///             comms::field::Bitfield<
///                 MyFieldBase,
///                 std::tuple<Field1, Field2, Field3>,
///                 TExtraOptions...
///             >;
///     public:
///         COMMS_FIELD_MEMBERS_ACCESS(member1, member2, member3);
///     };
///     @endcode
#define COMMS_FIELD_MEMBERS_ACCESS(...) \
    COMMS_EXPAND(COMMS_DEFINE_FIELD_ENUM(__VA_ARGS__)) \
    COMMS_FIELD_VALUE_ACCESS_FUNC { \
        auto& val = comms::field::toFieldBase(*this).value(); \
        using AllFieldsTuple = typename std::decay<decltype(val)>::type; \
        static_assert(std::tuple_size<AllFieldsTuple>::value == FieldIdx_numOfValues, \
            "Invalid number of names for fields tuple"); \
        return val; \
    } \
    COMMS_FIELD_VALUE_ACCESS_CONST_FUNC { \
        auto& val = comms::field::toFieldBase(*this).value(); \
        using AllFieldsTuple = typename std::decay<decltype(val)>::type; \
        static_assert(std::tuple_size<AllFieldsTuple>::value == FieldIdx_numOfValues, \
            "Invalid number of names for fields tuple"); \
        return val; \
    } \
    COMMS_EXPAND(COMMS_DO_FIELD_ACC_FUNC(ValueType, value(), __VA_ARGS__))

/// @brief Similar to @ref COMMS_FIELD_MEMBERS_ACCESS(), but dedicated for
///     non-template classes.
/// @details The @ref COMMS_FIELD_MEMBERS_ACCESS() macro is a generic one,
///     which can be used in any class (template, or non-template). However,
///     some compilers (such as <b>g++-4.9</b> and below, @b clang-4.0 and below) may fail
///     to compile it even though it uses valid C++11 constructs. If the
///     compilation fails and the class it is being used in is @b NOT a
///     template one, please use @ref COMMS_FIELD_MEMBERS_ACCESS_NOTEMPLATE()
///     instead.
/// @see @ref COMMS_FIELD_MEMBERS_NAMES()
/// @see @ref COMMS_FIELD_MEMBERS_ACCESS()
/// @note Defined in "comms/Field.h"
#define COMMS_FIELD_MEMBERS_ACCESS_NOTEMPLATE(...) \
    COMMS_EXPAND(COMMS_DEFINE_FIELD_ENUM(__VA_ARGS__)) \
    COMMS_EXPAND(COMMS_DO_FIELD_ACC_FUNC_NOTEMPLATE(__VA_ARGS__))

/// @brief Provide names for member fields of composite
///     fields, such as @ref comms::field::Bundle or @ref comms::field::Bitfield.
/// @details The fields of "composite" types, such as @ref comms::field::Bundle or
///     @ref comms::field::Bitfield keep their members bundled in
///     <a href="http://en.cppreference.com/w/cpp/utility/tuple">std::tuple</a>
///     and provide access to them via @b value() member functions.
///     The access to the specific member field can be obtained using
///     <a href="http://en.cppreference.com/w/cpp/utility/tuple/get">std::get</a>
///     later on:
///     @code
///     using MyFieldBase = comms::Field<comms::option::def::BigEndian>;
///     using Field1 = ... /* some field definition */;
///     using Field2 = ... /* some field definition */;
///     using Field3 = ... /* some field definition */;
///     using MyField =
///         comms::field::Bitfield<
///             MyFieldBase,
///             std::tuple<Field1, Field2, Field3>
///         >;
///
///     MyField field;
///     auto& members = field.value();
///     auto& firstMember = std::get<0>(members);
///     auto& secondMember = std::get<1>(members);
///     auto& thirdMember = std::get<2>(members);
///     @endcode
///     However, it would be convenient to provide names and easier access to
///     the member fields. The COMMS_FIELD_MEMBERS_NAMES() macro does exactly
///     that when used inside the field class definition. Just inherit from
///     the "composite" field and use the macro inside with the names for the
///     member fields:
///     @code
///     class MyField : public comms::field::Bitfield<...>
///     {
///         // (Re)define base class as inner Base type, required by COMMS_FIELD_MEMBERS_NAMES()
///         using Base = comms::field::Bitfield<...>;
///     public:
///         COMMS_FIELD_MEMBERS_NAMES(member1, member2, member3);
///     }
///     @endcode
///     @b NOTE that there is a required to have @b Base member type that
///     specifies base class used.
///
///     Usage of the COMMS_FIELD_MEMBERS_NAMES() is equivalent to having
///     the following types and functions  definitions:
///     @code
///     class MyField : public comms::field::Bitfield<...>
///     {
///     public:
///         // Access indices for member fields
///         enum FieldIdx {
///             FieldIdx_member1,
///             FieldIdx_member2,
///             FieldIdx_member3,
///             FieldIdx_numOfValues
///         };
///
///         // Accessor to "member1" member field.
///         auto field_member1() -> decltype(std::get<FieldIdx_member1>(value()))
///         {
///             return std::get<FieldIdx_member1>(value());
///         }
///
///         // Accessor to const "member1" member field.
///         auto field_member1() const -> decltype(std::get<FieldIdx_member1>(value()))
///         {
///             return std::get<FieldIdx_member1>(value());
///         }
///
///         // Accessor to "member2" member field.
///         auto field_member2() -> decltype(std::get<FieldIdx_member2>(value()))
///         {
///             return std::get<FieldIdx_member2>(value());
///         }
///
///         // Accessor to const "member2" member field.
///         auto field_member2() const -> decltype(std::get<FieldIdx_member2>(value()))
///         {
///             return std::get<FieldIdx_member2>(value());
///         }
///
///         // Accessor to "member3" member field.
///         auto field_member3() -> decltype(std::get<FieldIdx_member2>(value()))
///         {
///             return std::get<FieldIdx_member3>(value());
///         }
///
///         // Accessor to const "member3" member field.
///         auto field_member2() const -> decltype(std::get<FieldIdx_member3>(value()))
///         {
///             return std::get<FieldIdx_member3>(value());
///         }
///
///         // Redefinition of the field types:
///         using Field_member1 = Field1;
///         using Field_member2 = Field2;
///         using Field_member3 = Field3;
///     };
///     @endcode
///     @b NOTE, that provided names @b member1, @b member2, and @b member3, have
///         found their way to the following definitions:
///     @li @b FieldIdx enum. The names are prefixed with @b FieldIdx_. The
///         @b FieldIdx_nameOfValues value is automatically added at the end.
///     @li Accessor functions prefixed with @b field_*
///     @li Types of fields prefixed with @b Field_*
///
///     See @ref sec_field_tutorial_bitfield and @ref sec_field_tutorial_bundle
///         for more examples and details
/// @param[in] ... List of member fields' names.
/// @note Defined in "comms/Field.h"
#define COMMS_FIELD_MEMBERS_NAMES(...) \
    COMMS_EXPAND(COMMS_FIELD_MEMBERS_ACCESS(__VA_ARGS__)) \
    COMMS_EXPAND(COMMS_DO_FIELD_TYPEDEF(typename Base::ValueType, Field_, FieldIdx_, __VA_ARGS__))

/// @brief Generate convinience alias access member functions for other
///     member fields.
/// @details Same as #COMMS_MSG_FIELD_ALIAS() but applicable to
///     @ref comms::field::Bundle field.
/// @pre The macro #COMMS_FIELD_MEMBERS_ACCESS() needs to be used before
///     @ref COMMS_FIELD_ALIAS() to define convenience access functions.
#define COMMS_FIELD_ALIAS(f_, ...) COMMS_DO_ALIAS(field_, f_, __VA_ARGS__)

}  // namespace comms

