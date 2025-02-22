#include <vector>
#include <cstdint>
#include <cstring>
#include <iostream>

struct Buffer {
    std::vector<uint8_t> storage; // Underlying storage
    size_t data_begin = 0;        // Read position
    size_t data_end = 0;          // Write position

    Buffer(size_t initial_size = 1024) {
        storage.resize(initial_size);
    }

    // Write data into the buffer, expanding if needed
    void write(const uint8_t* data, size_t len) {
        ensure_capacity(len);
        std::memcpy(&storage[data_end], data, len);
        data_end += len;
    }

    // Read data without removal (peek)
    void peek(uint8_t* dest, size_t len) const {
        if (len > size()) throw std::out_of_range("Buffer underflow");
        std::memcpy(dest, &storage[data_begin], len);
    }

    // Consume data from the front
    void consume(size_t len) {
        if (len > size()) throw std::out_of_range("Buffer underflow");
        data_begin += len;
        if (data_begin == data_end) {
            // Reset indices to avoid growing buffer indefinitely
            data_begin = data_end = 0;
        }
    }

    // Get available data size
    size_t size() const {
        return data_end - data_begin;
    }

private:
    // Ensure buffer has enough capacity for new writes
    void ensure_capacity(size_t len) {
        if (data_end + len > storage.size()) {
            if (data_begin > 0) {
                // Compact buffer if possible
                size_t active_size = size();
                std::memmove(&storage[0], &storage[data_begin], active_size);
                data_end -= data_begin;
                data_begin = 0;
            }
            if (data_end + len > storage.size()) {
                // Expand buffer if still not enough
                storage.resize((storage.size() + len) * 2);
            }
        }
    }
};
