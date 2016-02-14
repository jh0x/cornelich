/*
Copyright 2015-2016 Joanna Hulboj <j@hulboj.org>
Copyright 2016 Milosz Hulboj <m@hulboj.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>

#include <utility>

namespace cornelich
{
namespace util
{

template<typename V>
struct default_validator
{
    bool operator()(const V &) const { return true; }
};

/**
 * A very simple cache from K to V that uses the specified Provider to compute the V if the specified
 * K is not already associated with a V. The V is added to the cache iff Validator returns true.
 */
template<typename K, typename V, typename Validator = default_validator<V>>
class cache
{
public:
    /** Create a cache with the given capacity and a specified validator */
    cache(std::size_t capacity, Validator validator = default_validator<V>());

    cache(const cache &) = delete;
    cache & operator=(const cache &) = delete;

    std::size_t capacity() const { return m_capacity; }
    std::size_t size() const { return m_container.size(); }

    /**
     * Get or compute a value corresponding to the given key.
     * - If the value corresponding to the given key is stored in the cache then mark it as most recently used and return the value.
     * - If the value corresponding to the given key is not in the cache then:
     *     - compute the value using the provider
     *     - check the value with the validator. If not OK then just return the value else:
     *         - if the cache is full then evict the LRU element
     *         - store the value in the cache and mark it as most recently used
     *
     * @param k key for which we want the value should
     * @param provider a function: k -> value
     * @return value corresponding to the given key
     */
    template<typename Provider>
    V get(const K & k, Provider && provider) const;
private:
    /// Tag for referring to the seequence index
    struct tag_sequence{};
    struct tag_key{};
    using pair_t = std::pair<K, V>;
    using container_t = boost::multi_index_container<
        pair_t, // use pair_t as an entry in the multi_index container and create the following two indices:
        boost::multi_index::indexed_by<
            // bidirectional list-like sorted according to insertion order behaviour:
            boost::multi_index::sequenced<boost::multi_index::tag<tag_sequence>>,
            // unordered map like behaviour:
            boost::multi_index::hashed_unique<boost::multi_index::tag<tag_key>, boost::multi_index::member<pair_t, K, &pair_t::first> >
        >
    >;
    // Aliases for the different views
    using sequence_view = typename boost::multi_index::index<container_t, tag_sequence>::type;
    using key_view = typename boost::multi_index::index<container_t, tag_key>::type;


    const std::size_t m_capacity;
    const Validator m_validator;

    mutable container_t m_container;
};


template<typename K, typename V, typename Validator>
cache<K, V, Validator>::cache(std::size_t capacity, Validator validator)
    : m_capacity(capacity)
    , m_validator(validator)
{
}

template<typename K, typename V, typename Validator>
template<typename Provider>
inline V cache<K, V, Validator>::get(const K & k, Provider && provider) const
{
    // Use the key index to check if we have that {k, v}
    auto & index = m_container.template get<tag_key>();
    auto it = index.find(k);
    if(it != index.end())
    {
        // We have it, mark it as the most recently used and we're done
        auto & sequence_index = m_container.template get<tag_sequence>();
        sequence_index.relocate(sequence_index.end(), boost::multi_index::project<tag_sequence>(m_container, it));
        return it->second;
    }

    // Compute the value
    auto v = provider(k);
    if(!m_validator(v)) // Don't store the {k, v} if not validated
        return v;

    // Evict the LRU entry if needed
    if(m_container.size() >= m_capacity) {
        m_container.pop_front();
    }

    auto element = m_container.push_back({k, v});
    return element.first->second;
}

}
}
