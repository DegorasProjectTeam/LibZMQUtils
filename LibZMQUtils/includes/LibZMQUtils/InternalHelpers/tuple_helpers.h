/***********************************************************************************************************************
 *   LibZMQUtils (ZeroMQ High-Level Utilities C++ Library).                                                            *
 *                                                                                                                     *
 *   A modern open-source and cross-platform C++ library with high-level utilities based on the well-known ZeroMQ      *
 *   open-source universal messaging library. Includes a suite of modules that encapsulates the ZMQ communication      *
 *   patterns as well as automatic binary serialization capabilities, specially designed for system infraestructure.   *
 *   The library is suited for the quick and easy integration of new and old systems and can be used in different      *
 *   sectors and disciplines seeking robust messaging and serialization solutions.                                     *
 *                                                                                                                     *
 *   Developed as free software within the context of the Degoras Project for the Satellite Laser Ranging Station      *
 *   (SFEL) at the Spanish Navy Observatory (ROA) in San Fernando, Cádiz. The library is open for use by other SLR     *
 *   stations and organizations, so we warmly encourage you to give it a try and feel free to contact us anytime!      *
 *                                                                                                                     *
 *   Copyright (C) 2024 Degoras Project Team                                                                           *
 *                      < Ángel Vera Herrera, avera@roa.es - angeldelaveracruz@gmail.com >                             *
 *                      < Jesús Relinque Madroñal >                                                                    *
 *                                                                                                                     *
 *   This file is part of LibZMQUtils.                                                                                 *
 *                                                                                                                     *
 *   Licensed under the European Union Public License (EUPL), Version 1.2 or subsequent versions of the EUPL license   *
 *   as soon they will be approved by the European Commission (IDABC).                                                 *
 *                                                                                                                     *
 *   This project is free software: you can redistribute it and/or modify it under the terms of the EUPL license as    *
 *   published by the IDABC, either Version 1.2 or, at your option, any later version.                                 *
 *                                                                                                                     *
 *   This project is distributed in the hope that it will be useful. Unless required by applicable law or agreed to in *
 *   writing, it is distributed on an "AS IS" basis, WITHOUT ANY WARRANTY OR CONDITIONS OF ANY KIND; without even the  *
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the EUPL license to check specific   *
 *   language governing permissions and limitations and more details.                                                  *
 *                                                                                                                     *
 *   You should use this project in compliance with the EUPL license. You should have received a copy of the license   *
 *   along with this project. If not, see the license at < https://eupl.eu/ >.                                         *
 **********************************************************************************************************************/

/** ********************************************************************************************************************
 * @file tuple_helpers.h
 * @brief This file contains several helper tools related with tuples.
 * @warning Not exported. Only for internal library usage.
 * @author Degoras Project Team.
 * @copyright EUPL License
 * @version 2309.5
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// C++ INCLUDES
//======================================================================================================================
#include <tuple>
// =====================================================================================================================

// LIBZMQUTILS INCLUDES
// =====================================================================================================================
// =====================================================================================================================

// LIBZMQUTILS NAMESPACES
// =====================================================================================================================
namespace zmqutils{
namespace internal_helpers{
namespace tuple{

/**
 * @brief Implememtation of tuple split. This version performs the split by movement.
 *
 * @tparam N1, the sequence for the first subtuple.
 * @tparam N2, the sequence for the second subtuple.
 * @tparam Ts, the types of the tuple to split.

 * @param t, the tuple to split.
 */
template <std::size_t... N1 , std::size_t ...N2, typename... Ts>
auto tuple_split_impl(std::index_sequence<N1...> , std::index_sequence<N2...>, std::tuple<Ts...> &&t)
{
    return std::make_tuple(std::make_tuple(std::get<N1>(std::move(t))...),
                           std::make_tuple(std::get<N2 + sizeof...(N1)>(std::move(t))...));
}

/**
 * @brief Split one tuple into two subtuples. The types of subtuples concatenated must be the same as the concatenated.
 *        This version performs the split by movement.
 *
 * @tparam TypesCat, the types of the tuple to split.
 * @tparam Types1, the types of the first subtuple.
 * @tparam Types2, the types of the second subtuple.
 * @param cat, the tuple to split.
 * @param t1, the first subtuple.
 * @param t2, the second subtuple.
 */
template <typename ...TypesCat, typename ...Types1, typename ...Types2>
void tuple_split(std::tuple<TypesCat...> &&cat, std::tuple<Types1...> &t1, std::tuple<Types2...> &t2)
{
    static_assert(std::is_same_v<std::tuple<TypesCat...>, std::tuple<Types1..., Types2...>>,
                  "The types of t1 and t2 concatenated must be the same as cat" );

    std::tie(t1,t2) = tuple_split_impl(std::make_index_sequence<sizeof...(Types1)>(),
                                        std::make_index_sequence<sizeof...(Types2)>(), std::move(cat));
}

/**
 * @brief Implememtation of tuple split. This version performs the split by copy.
 *
 * @tparam N1, the sequence for the first subtuple.
 * @tparam N2, the sequence for the second subtuple.
 * @tparam Ts, the types of the tuple to split.

 * @param t, the tuple to split.
 */
template <std::size_t... N1 , std::size_t ...N2, typename... Ts>
auto tuple_split_impl(std::index_sequence<N1...> , std::index_sequence<N2...>, const std::tuple<Ts...> &t)
{
    return std::make_tuple(std::make_tuple(std::get<N1>(t)...),
                           std::make_tuple(std::get<N2 + sizeof...(N1)>(t)...));
}

/**
 * @brief Split one tuple into two subtuples. The types of subtuples concatenated must be the same as the concatenated.
 *        This version performs the split by copy.
 *
 * @tparam TypesCat, the types of the tuple to split.
 * @tparam Types1, the types of the first subtuple.
 * @tparam Types2, the types of the second subtuple.
 * @param cat, the tuple to split.
 * @param t1, the first subtuple.
 * @param t2, the second subtuple.
 */
template <typename ...TypesCat, typename ...Types1, typename ...Types2>
void tuple_split(const std::tuple<TypesCat...> &cat, std::tuple<Types1...> &t1, std::tuple<Types2...> &t2)
{
    static_assert(std::is_same_v<std::tuple<TypesCat...>, std::tuple<Types1..., Types2...>>,
                  "The types of t1 and t2 concatenated must be the same as cat" );

    std::tie(t1,t2) = tuple_split_impl(std::make_index_sequence<sizeof...(Types1)>(),
                                        std::make_index_sequence<sizeof...(Types2)>(), cat);

}

}}} // END NAMESPACES.
// =====================================================================================================================
