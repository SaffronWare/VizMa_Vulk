#pragma once

#include <ArkMat.hpp>


struct alignas(16) CameraDataContainer
{
	ark::Vec3 front; // internally a vec4 with padding as 4th comp
	ark::Vec3 right; // internally a vec4 as padding with 4th comp
	ark::Vec3 up; // ...
	ark::Vec3 pos; // ...
	float focal_length;
	float aspect_ratio;
};


struct Camera
{
public:
	Camera(ark::Vec3 pos = ark::Vec3(-1.0, 0.75, 0), ark::Vec3 ori = ark::Vec3(0, 0, 0), float focal = 2.0f, float aspect = 1.0f);
	~Camera();

	ark::Vec3 orientation; // x rot, y rot, z rot

	ark::Vec3 getFront() const;
	ark::Vec3 getRight() const;
	ark::Vec3 getUp() const;
	ark::Vec3 getPos() const;

	ark::Vec3 updPos(ark::Vec3 displacement);
	float updFocal(float delta);

	void setPos(ark::Vec3 position);
	void setAspect(float aspect);
	void setFocal(float focal);
	
	

	void writeData(void* storage) const;
	void update();

private:
	static const ark::Vec3 identity_front;
	static const ark::Vec3 identity_right;
	static const ark::Vec3 identity_up;

	CameraDataContainer myData;
	


};