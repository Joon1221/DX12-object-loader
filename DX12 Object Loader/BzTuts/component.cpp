#include "component.h"

Component::Component() {
	active = true;
}
Component::~Component() {}

bool Component::Init() {
	return false;
}