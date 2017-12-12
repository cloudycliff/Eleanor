//
//  camera.h
//  Eleanor
//
//  Created by cliff on 24/11/2017.
//  Copyright © 2017 cliff. All rights reserved.
//

#ifndef camera_h
#define camera_h

#include "math/math.h"
#include "TransformUtils.h"

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

const float DEG2RAD = 3.14159265358979f / 180.0f;

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 1.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera {
public:
    vector3 Position;
    vector3 Front;
    vector3 Up;
    vector3 Right;
    vector3 WorldUp;
    
    float Yaw;
    float Pitch;
    
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    
    // Constructor with vectors
    Camera(vector3 position = vector3(0.0f, 0.0f, 0.0f), vector3 up = vector3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(vector3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    // Constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(vector3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = vector3(posX, posY, posZ);
        WorldUp = vector3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    
    matrix44 GetViewMatrix() {
        return lookat(Position, Position+Front, Up);
    }
    
    void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position = Position + Front * velocity;
        if (direction == BACKWARD)
            Position = Position - Front * velocity;
        if (direction == LEFT)
            Position = Position - Right * velocity;
        if (direction == RIGHT)
            Position = Position + Right * velocity;
    }
    
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;
        
        Yaw -= xoffset;
        Pitch += yoffset;
        
        if (constrainPitch) {
            if (Pitch > 89.0f) Pitch = 89.0f;
            if (Pitch < -89.0f) Pitch = -89.0f;
        }
        
        updateCameraVectors();
    }
    
    void ProcessMouseScroll(float yoffset) {
        if (Zoom >= 1.0f && Zoom <= 45.0f) Zoom -= yoffset;
        if (Zoom <= 1.0f) Zoom = 1.0f;
        if (Zoom >= 45.0f) Zoom = 45.0f;
    }
    
private:
    void updateCameraVectors() {
        vector3 front;
        front.x = cos(Yaw*DEG2RAD) * cos(Pitch*DEG2RAD);
        front.y = sin(Pitch*DEG2RAD);
        front.z = sin(Yaw*DEG2RAD) * cos(Pitch*DEG2RAD);
        
        Front = normalize(front);
        vector3Cross(Right, Front, WorldUp);
        Right.normalize();

        vector3Cross(Up, Right, Front);
        Up.normalize();
    }
};

#endif /* camera_h */
