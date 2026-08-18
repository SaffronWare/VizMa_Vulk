#include "Camera.h"


Camera::Camera(ark::Vec3 pos = ark::Vec3(0,0,0), ark::Vec3 ori = ark::Vec3(0,0,0), float focal = 1.0f, float aspect = 1.0f)
	: position(pos), orientation(ori), focal_length(focal), aspect_ratio(aspect)
{

}

Camera::~Camera() {}


// NOTE THAT FOCAL IS 4TH COMP OF FRONT AND ASPECT IS 4TH COMP OF RIGHT
void Camera::writeData(void* storage) const
{
	return 
}

void Camera::update() const
{

}

ark::Vec3 Camera::getFront() const
{
	return front;
}

ark::Vec3 Camera::getRight() const
{
	return right;
}

ark::Vec3 Camera::getUp() const
{
	return up;
}