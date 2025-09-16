/* date = September 13th 2025 2:00 pm */

#ifndef PERSPECTIVE_TEXTURE_MAP_H
#define PERSPECTIVE_TEXTURE_MAP_H

#define TEXTURE_WIDTH 16
#define TEXTURE_HEIGHT 16

//static uint32 TextureBytes[TEXTURE_HEIGHT][TEXTURE_WIDTH];

typedef struct
{
    float OneOverZ[3];
    float UOverZ[3];
    float VOverZ[3];
    
    float dOneOverZdX;
    float dOneOverZdY;
    
    float dUOverZdX;
    float dUOverZdY;
    
    float dVOverZdX;
    float dVOverZdY;
}gradient;

typedef struct
{
    vec5 V1;
    vec5 V2;
    
    uint32 Y;
    uint32 Height;
    uint32 IntY1;
    uint32 IntY2;
    
    real32 X;
    real32 XStep;
    
    real32 UOverZ;
    real32 UOverZStep;
    
    real32 OneOverZ;
    real32 OneOverZStep;
    
    real32 VOverZ;
    real32 VOverZStep;
    
}edge;

#endif //PERSPECTIVE_TEXTURE_MAP_H
