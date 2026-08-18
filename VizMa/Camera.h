#pragma once

#include <ArkMat.hpp>

struct Camera
{
public:
	Camera(ark::Vec3 pos = ark::Vec3(0, 0, 0), ark::Vec3 ori = ark::Vec3(0, 0, 0), float focal = 1.0f, float aspect = 1.0f);
	~Camera();

	ark::Vec3 position;
	ark::Vec3 orientation; // x rot, y rot, z rot

	float focal_length;
	float aspect_ratio;

	ark::Vec3 getFront() const;
	ark::Vec3 getRight() const;
	ark::Vec3 getUp() const;

	void writeData(void* storage) const;
	void update() const;

private:
	static const ark::Vec3 identity_front;
	static const ark::Vec3 identity_right;
	static const ark::Vec3 identity_up;

	ark::Vec3 front;
	ark::Vec3 right;
	ark::Vec3 up;

};