#pragma once
#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>

// Интерфейс сериализации: анимации и прочие сущности реализуют его
class ISerializable {
public:
    virtual ~ISerializable() {}
    virtual size_t serializedSize() const = 0;
    virtual bool serialize(uint8_t* out, size_t maxLen) const = 0;
    virtual bool deserialize(const uint8_t* data, size_t len) = 0;
};
