#pragma once
#include <set>

#include "src/core/types.hpp"
#include "src/core/operation.hpp"

namespace ucsb
{

    struct bulk_metadata_t
    {
        std::set<std::string> files;
        size_t records_count = 0;
        void *data = nullptr;
    };

    /**
     * @brief A base class for data accessing: on DBs and Transactions state.
     *
     * @section Keys.
     * The key type is set to a 64-bit unsigned integer.
     * Todays engines often support string keys of variable length,
     * but generally work faster if keys are of identical length.
     * To avoid extra heap allocations and expensive integer formatting,
     * under the hood, we pass a view to raw bytes forming the integer key.
     * For order consistency it's also recommended to provide a custom
     * comparator.
     */
    struct data_accessor_t
    {
        virtual ~data_accessor_t() {}

        virtual operation_result_t insert(key_t key, value_spanc_t value) = 0;
        virtual operation_result_t update(key_t key, value_spanc_t value) = 0;
        virtual operation_result_t remove(key_t key) = 0;
        virtual operation_result_t read(key_t key, value_span_t value) const = 0;

        /**
         * @brief Performs many insert at once in a batch-asynchronous fashion.
         * Keys are in strict ascending order
         */
        virtual operation_result_t batch_insert(keys_spanc_t keys, values_spanc_t values, value_lengths_spanc_t sizes) = 0;

        /**
         * @brief Performs many reads at once in a batch-asynchronous fashion.
         * This means, that the order of lookups within the batch is irrelevant
         * and the engine can reorganize them for faster execution.
         *
         * Every DB engine implements the interface for this operation in a different
         * way, making designing generic interfaces cumbersome and costly (performance-
         * wise). For this benchmark we don't return the retrieved the values and only
         * check them under the hood.
         */
        virtual operation_result_t batch_read(keys_spanc_t keys) const = 0;

        /**
         * @brief Performs bulk import from external prepared data.
         * Keys are in strict ascending order
         */
        virtual bulk_metadata_t prepare_bulk_import_data(keys_spanc_t keys,
                                                         values_spanc_t values,
                                                         value_lengths_spanc_t sizes) const = 0;
        virtual operation_result_t bulk_import(bulk_metadata_t const &metadata) = 0;

        /**
         * @brief Performs many reads at once in an ordered fashion,
         * starting from a specified `key` location.
         *
         * Just like `batch_read(...)` we don't return all the found values.
         * It's irrelevant for benchmarking, so we only output the first one.
         *
         * @param key The first entry to find and read.
         * @param length The number of consecutive entries to read.
         * @param single_value A temporary buffer big enough for a single value.
         */
        virtual operation_result_t range_select(key_t key, size_t length, value_span_t single_value) const = 0;

        /**
         * @brief Reads all the entries in DBMS from start to end (in ordered fashion).
         * @param single_value A temporary buffer big enough for the biggest single value.
         */
        virtual operation_result_t scan(value_span_t single_value) const = 0;
    };

} // namespace ucsb