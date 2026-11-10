// Algorithms.h - hand-written sorting algorithms (no std::sort / std::stable_sort).
// Author: Linda Obodo (COS202 Lab Assessment)
//
// Both algorithms are templates so they can sort any std::vector (patients, report rows ...)
// with any "less than" comparison. They are STABLE: records that compare equal keep
// their original relative order, which matters when sorting a clinic list by e.g. age.
#pragma once

#include <cstddef>
#include <utility>
#include <vector>

namespace scapms::utils {

/// INSERTION SORT  -  O(n^2) worst case, O(n) when the data is already (nearly) sorted.
///
/// Why it is used: for SMALL lists (a typical clinic ward list) or lists that are already
/// almost in order, insertion sort has the lowest overhead: no recursion, no extra memory,
/// and it is stable. Each element is "inserted" into the already sorted part on its left.
template <typename T, typename Less>
void insertionSort(std::vector<T>& items, Less less) {
    for (std::size_t i = 1; i < items.size(); ++i) {
        T key = std::move(items[i]);
        std::size_t j = i;
        // Shift larger elements one place to the right until key's position is found.
        while (j > 0 && less(key, items[j - 1])) {
            items[j] = std::move(items[j - 1]);
            --j;
        }
        items[j] = std::move(key);
    }
}

namespace detail {

// Recursive helper: sorts the half-open range [low, high).
template <typename T, typename Less>
void mergeSortRange(std::vector<T>& items, std::vector<T>& buffer,
                    std::size_t low, std::size_t high, Less& less) {
    if (high - low < 2) return;                     // base case: 0 or 1 element is sorted
    const std::size_t mid = low + (high - low) / 2;
    mergeSortRange(items, buffer, low, mid, less);  // sort left half  (recursion)
    mergeSortRange(items, buffer, mid, high, less); // sort right half (recursion)

    // Merge the two sorted halves into the buffer, then copy back.
    std::size_t left = low;
    std::size_t right = mid;
    std::size_t out = low;
    while (left < mid && right < high) {
        // Take from the right only if it is STRICTLY smaller -> keeps the sort stable.
        if (less(items[right], items[left])) buffer[out++] = std::move(items[right++]);
        else buffer[out++] = std::move(items[left++]);
    }
    while (left < mid) buffer[out++] = std::move(items[left++]);
    while (right < high) buffer[out++] = std::move(items[right++]);
    for (std::size_t k = low; k < high; ++k) items[k] = std::move(buffer[k]);
}

} // namespace detail

/// MERGE SORT  -  O(n log n) in the best, average AND worst case; stable; needs O(n) extra memory.
///
/// Why it is used: when the clinic database grows large, insertion sort's O(n^2) becomes slow.
/// Merge sort ("divide and conquer") splits the list in halves recursively, sorts each half
/// and merges them. Its running time does not depend on how the data is ordered.
template <typename T, typename Less>
void mergeSort(std::vector<T>& items, Less less) {
    std::vector<T> buffer(items.size());
    detail::mergeSortRange(items, buffer, 0, items.size(), less);
}

} // namespace scapms::utils
