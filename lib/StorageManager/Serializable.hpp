#pragma once
#include <cstddef>
#include <cstdint>

class ISerializable {
public:
    virtual size_t serializedSize() const = 0;
    virtual bool serialize(uint8_t* buf, size_t len) const = 0;
    virtual bool deserialize(const uint8_t* buf, size_t len) = 0;
    virtual ~ISerializable() {}
};
