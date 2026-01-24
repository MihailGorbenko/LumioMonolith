#ifndef PERSISTANT_HPP
#define PERSISTANT_HPP
#include <Arduino.h>

class IPersistant {
public:
	// Сохранить состояние в NVS (ключ key). Вернуть true при успехе.
	virtual bool saveToNVS(const char* key) = 0;
	// Загрузить состояние из NVS (ключ key). Вернуть true при успехе.
	virtual bool loadFromNVS(const char* key) = 0;
	virtual ~IPersistant() {}
};

#endif // PERSISTANT_HPP
