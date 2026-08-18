#include "Camera.h"

const ark::Vec3 Camera::identity_front = ark::Vec3(0, 0, 1);
const ark::Vec3 Camera::identity_right = ark::Vec3(1, 0, 0);
const ark::Vec3 Camera::identity_up = ark::Vec3(0, 1, 0);

Camera::Camera(ark::Vec3 pos, ark::Vec3 ori, float focal, float aspect)
	: orientation(ori)
{
	myData = CameraDataContainer{identity_front, identity_right, identity_up, pos, focal, aspect };
	update();
	
}

Camera::~Camera() {}


void Camera::writeData(void* storage) const
{
	std::memcpy(storage, static_cast<const void*>(&myData), sizeof(myData));
}

void Camera::update()
{
	myData.front = ark::rotate(identity_front, orientation);
	myData.right = ark::rotate(identity_right, orientation);
	myData.up = ark::rotate(identity_up, orientation);
}

ark::Vec3 Camera::getFront() const
{
	return myData.front;
}

ark::Vec3 Camera::getRight() const
{
	return myData.right;
}

ark::Vec3 Camera::getUp() const
{
	return myData.up;
}

ark::Vec3 Camera::getPos() const
{
	return myData.pos;
}

ark::Vec3 Camera::updPos(ark::Vec3 displacement)
{
	myData.pos += displacement;
	return myData.pos;
}

float Camera::updFocal(float delta)
{
	myData.focal_length += delta;
	return myData.focal_length;
}

void Camera::setPos(ark::Vec3 position)
{
	myData.pos = position;

}

void Camera::setFocal(float focal)
{
	myData.focal_length = focal;
}

void Camera::setAspect(float aspect)
{
	myData.aspect_ratio = aspect;
}