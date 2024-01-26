#include "Camera.h"
#include <iostream>

void Camera::updateViewMatrix()
{
	glm::mat4 rotM = glm::mat4(1.0f);
	glm::mat4 transM;

	//up/down
	rotM = glm::rotate(rotM, glm::radians(rotation.x * flipY), glm::vec3(1.0f, 0.0f, 0.0f));
	//left/right
	rotM = glm::rotate(rotM, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	//roll left/right
	rotM = glm::rotate(rotM, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	/*glm::vec3 translation = position;
	translation.y *= flipY;
	transM = glm::translate(glm::mat4(1.0f), translation);*/

	transM = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y * flipY, position.z));
	if (type == CameraType::firstperson)
	{
		//used to translate world coords to view coords
		matrices.view = rotM * transM;
	}
	else
	{
		matrices.view = transM * rotM;
	}

	//position of the camera
	viewPos = glm::vec4(position, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

	updated = true;
}

bool Camera::moving()
{
	//return keys.left || keys.right || keys.forward || keys.backward || keys.up || keys.down;
	return (keys.left ^ keys.right) || (keys.forward ^ keys.backward) || (keys.up ^ keys.down);
}

float Camera::getNearClip()
{
	return znear;
}

float Camera::getFarClip()
{
	return zfar;
}

void Camera::setPerspective(float fov, float aspect, float znear, float zfar)
{
	this->fov = fov;
	this->znear = znear;
	this->zfar = zfar;
	matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
	matrices.perspective[1][1] *= flipY;
}

void Camera::updateAspectRatio(float aspect)
{
	matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
	matrices.perspective[1][1] *= flipY;
}

void Camera::setPosition(glm::vec3 position)
{
	this->position = position;
	updateViewMatrix();
}

void Camera::setRotation(glm::vec3 rotation)
{
	this->rotation = rotation;
	updateViewMatrix();
}

void Camera::rotate(glm::vec3 delta)
{
	//Basic
	//this->rotation += delta;
	//this->rotation.x = glm::clamp(this->rotation.x, -90.0f, 90.0f);	
	

	//TEST
	//katso sel�n taakse siirt�m�ll� hiiri yl�s/alas (mene siltaan tai katso jalkojen v�list�)
	//rotaatiot toimii oikein kun kerrotaan delta -1.0f:ll�, mutta liikkuminen pit�� kans muuttaa samoin
	//+delta.x vai ei, siin�p� kysymys
	if (rotation.x + delta.x > 90.0f || rotation.x + delta.x < -90.0f)
	{
		delta.y *= -1.0f;
	}
	this->rotation.x = glm::clamp(this->rotation.x, -180.0f, 180.0f);

	this->rotation += delta;

	//std::cout << rotation.x << " " << rotation.y << " " << rotation.z << std::endl;

	updateViewMatrix();
}

void Camera::setTranslation(glm::vec3 translation)
{
	this->position = translation;
	updateViewMatrix();
}

void Camera::translate(glm::vec3 delta)
{
	this->position += delta;
	updateViewMatrix();
}

void Camera::setRotationSpeed(float rotationSpeed)
{
	this->rotationSpeed = rotationSpeed;
}

void Camera::setMovementSpeed(float movementSpeed)
{
	this->movementSpeed = movementSpeed;
}

void Camera::update(float deltaTime)
{
	updated = false;
	if (type == CameraType::firstperson)
	{
		//if (moving())
		//{
			//glm::vec3 camFront;
			//camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
			//camFront.y = sin(glm::radians(rotation.x));
			//camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
			//camFront = glm::normalize(camFront);

			//glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f); // Up vector

			//float moveSpeed = deltaTime * movementSpeed;

			//if (keys.forward)
			//	position += camFront * moveSpeed;
			//if (keys.backward)
			//	position -= camFront * moveSpeed;
			//if (keys.left)
			//	position -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;			
			//if (keys.right)
			//	position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;

			//// Additional controls for moving up and down
			//if (keys.up)
			//	position += camUp * moveSpeed;  // Move up
			//if (keys.down)
			//	position -= camUp * moveSpeed;  // Move down
		//}

		float moveSpeed = deltaTime * movementSpeed;


		int moveUpDown = -keys.down + keys.up;
		if (moveUpDown)
		{
			glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f); // Up vector
			position += (float)moveUpDown * camUp * moveSpeed;
		}


		int moveLeftRight = -keys.left + keys.right;
		int moveBackwardForward = -keys.backward + keys.forward;

		if (moveLeftRight || moveBackwardForward)
		{
			glm::vec3 camFront;
			camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
			camFront.y = sin(glm::radians(rotation.x));
			camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
			camFront = glm::normalize(camFront);

			if (moveLeftRight)
			{
				position += (float)moveLeftRight * glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
			}

			if (moveBackwardForward)
			{
				position += (float)moveBackwardForward * camFront * moveSpeed;
			}
		}
	}

	updateViewMatrix();
}

// Update camera passing separate axis data (gamepad)
// Returns true if view or position has been changed
bool Camera::updatePad(glm::vec2 axisLeft, glm::vec2 axisRight, float deltaTime)
{
	bool retVal = false;

	if (type == CameraType::firstperson)
	{
		// Use the common console thumbstick layout		
		// Left = view, right = move

		const float deadZone = 0.0015f;
		const float range = 1.0f - deadZone;

		glm::vec3 camFront;
		camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
		camFront.y = sin(glm::radians(rotation.x));
		camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
		camFront = glm::normalize(camFront);

		float moveSpeed = deltaTime * movementSpeed * 2.0f;
		float rotSpeed = deltaTime * rotationSpeed * 50.0f;

		// Move
		if (fabsf(axisLeft.y) > deadZone)
		{
			float pos = (fabsf(axisLeft.y) - deadZone) / range;
			position -= camFront * pos * ((axisLeft.y < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
			retVal = true;
		}
		if (fabsf(axisLeft.x) > deadZone)
		{
			float pos = (fabsf(axisLeft.x) - deadZone) / range;
			position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * pos * ((axisLeft.x < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
			retVal = true;
		}

		// Rotate
		if (fabsf(axisRight.x) > deadZone)
		{
			float pos = (fabsf(axisRight.x) - deadZone) / range;
			rotation.y += pos * ((axisRight.x < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
			retVal = true;
		}
		if (fabsf(axisRight.y) > deadZone)
		{
			float pos = (fabsf(axisRight.y) - deadZone) / range;
			rotation.x -= pos * ((axisRight.y < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
			retVal = true;
		}
	}
	else
	{
		// todo: move code from example base class for look-at
	}

	if (retVal)
	{
		updateViewMatrix();
	}

	return retVal;
}
