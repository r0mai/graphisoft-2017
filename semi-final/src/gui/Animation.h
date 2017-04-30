#pragma once
#include <SFML/System/Time.hpp>


class Animation {
public:
	virtual ~Animation() {}
	virtual bool Animate(const sf::Time& t) = 0;
	virtual void Finish() = 0;
};
