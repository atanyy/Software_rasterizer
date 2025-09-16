vec2 AddVec2(vec2 V1, vec2 V2)
{
    vec2 Result;
    Result.X = V1.X + V2.X;
    Result.Y = V1.Y + V2.Y;
    
    return Result;
}

vec2 SubtractVec2(vec2 V1, vec2 V2)
{
    vec2 Result;
    Result.X = V1.X - V2.X;
    Result.Y = V1.Y - V2.Y;
    
    return Result;
}

real32 DotVec2(vec2 V1, vec2 V2)
{
    real32 Result = (V1.X * V2.X) + (V1.Y * V2.Y);
    return Result;
}

vec3 AddVec3(vec3 V1, vec3 V2)
{
    vec3 Result;
    Result.X = V1.X + V2.X;
    Result.Y = V1.Y + V2.Y;
    Result.Z = V1.Z + V2.Z;
    
    return Result;
}

vec3 SubtractVec3(vec3 V1, vec3 V2)
{
    vec3 Result;
    Result.X = V1.X - V2.X;
    Result.Y = V1.Y - V2.Y;
    Result.Z = V1.Z - V2.Z;
    
    return Result;
}

real32 DotVec3(vec3 V1, vec3 V2)
{
    real32 Result = (V1.X * V2.X) + (V1.Y * V2.Y) + (V1.Z * V2.Z);
    return Result;
}

vec3 CrossVec3(vec3 V1, vec3 V2)
{
    vec3 Result;
    Result.X = (V1.Y * V2.Z) - (V1.Z * V2.Y);
    Result.Y = -(V1.X * V2.Z) + (V1.Z * V2.X);
    Result.Z = (V1.X * V2.Y) - (V1.Y * V2.X);
    
    return Result;
}

static vec2 Vec2CoordToScreenCoord(win32_pixel_buffer *Buffer, vec2 Vector)
{
    vec2 Result;
    
    uint32 CenX = Buffer->Width / 2;
    uint32 CenY = Buffer->Height / 2;
    Result.X = CenX + Vector.X;
    Result.Y = CenY - Vector.Y;
    
    return Result;
}

static vec3 Vec3CoordToScreenCoord(win32_pixel_buffer *Buffer, vec3 Vector)
{
    vec3 Result;
    
    uint32 CenX = Buffer->Width / 2;
    uint32 CenY = Buffer->Height / 2;
    Result.X = CenX + Vector.X;
    Result.Y = CenY - Vector.Y;
    Result.Z = Vector.Z;
    
    return Result;
}

static vec2 ProjectVector(vec3 Vector, vec3 CameraPos)
{
    vec2 Result;
    real32 ScalingFactor = 3500.0f;
    
    real32 ScreenX = (real32)fabs(CameraPos.Z) * (Vector.X / Vector.Z);
    real32 ScreenY = (real32)fabs(CameraPos.Z) * (Vector.Y / Vector.Z);
    
    //This calculation is wrong for CameraPos.Z = 0
    Result.X = ScalingFactor * ScreenX;
    Result.Y = ScalingFactor * ScreenY;
    
    return Result;
}

vec3 RotateAlongX(vec3 Point, real32 Angle)
{
    vec3 Result;
    Result.Y = Point.Y * cosf(Angle) - Point.Z * sinf(Angle);
    Result.Z = Point.Z * cosf(Angle) + Point.Y * sinf(Angle);
    Result.X = Point.X;
    
    return Result;
}

vec3 RotateAlongY(vec3 Point, real32 Angle)
{
    vec3 Result;
    Result.X = Point.X * cosf(Angle) - Point.Z * sinf(Angle);
    Result.Z = Point.Z * cosf(Angle) + Point.X * sinf(Angle);
    Result.Y = Point.Y;
    
    return Result;
}

vec3 RotateAlongZ(vec3 Point, real32 Angle)
{
    vec3 Result;
    Result.X = Point.X * cosf(Angle) - Point.Y * sinf(Angle);
    Result.Y = Point.Y * cosf(Angle) + Point.X * sinf(Angle);
    Result.Z = Point.Z;
    
    return Result;
}

void SwapVec2(vec2 *V1, vec2 *V2)
{
    vec2 Temp = *V1;
    *V1 = *V2;
    *V2 = Temp;
}

vec4 MultiplyMat4Vec3(mat4 Matrix, vec3 V)
{
    vec4 Result;
    Result.X = Matrix.M[0][0] * V.X + Matrix.M[0][1] * V.Y + Matrix.M[0][2] * V.Z + Matrix.M[0][3] * 1;
    Result.Y = Matrix.M[1][0] * V.X + Matrix.M[1][1] * V.Y + Matrix.M[1][2] * V.Z + Matrix.M[1][3] * 1;
    Result.Z = Matrix.M[2][0] * V.X + Matrix.M[2][1] * V.Y + Matrix.M[2][2] * V.Z + Matrix.M[2][3] * 1;
    Result.W = Matrix.M[3][0] * V.X + Matrix.M[3][1] * V.Y + Matrix.M[3][2] * V.Z + Matrix.M[3][3] * 1;
    
    return Result;
}

inline real32 GetMagnitudeVec3(vec3 V)
{
    real32 Result;
    
    Result = sqrtf((V.X * V.X) + (V.Y * V.Y) + (V.Z * V.Z));
    
    return Result;
}

inline vec3 NormalizeVec3(vec3 V)
{
    vec3 Result;
    real32 Magnitude = GetMagnitudeVec3(V);
    Result.X = V.X / Magnitude;
    Result.Y = V.Y / Magnitude;
    Result.Z = V.Z / Magnitude;
    
    return Result;
}

uint32 GetPercentOfColor(uint32 Color, real32 PercentColor)
{
    uint32 Result;
    
    uint32 RedMask = 0x00FF0000;
    uint32 GreenMask = 0x0000FF00;
    uint32 BlueMask = 0x000000FF;
    uint32 AlphaMask = 0xFF000000;
    
    uint32 Alpha = (Color & AlphaMask) >> 24;
    uint32 Red = (Color & RedMask) >> 16;
    uint32 Green = (Color & GreenMask) >> 8;
    uint32 Blue = (Color & BlueMask) >> 0;
    
    uint32 NewRed = RoundReal32ToUInt32(Red * PercentColor);
    uint32 NewGreen = RoundReal32ToUInt32(Green * PercentColor);
    uint32 NewBlue = RoundReal32ToUInt32(Blue * PercentColor);
    
    Result = (Alpha << 24) | (NewRed << 16) | (NewGreen << 8) | (NewBlue << 0);
    
    return Result;
}

uint32 GetFlatShadingColor(vec3 Normal, light Light, uint32 Color)
{
    uint32 Result;
    
    real32 DotNormalLight = DotVec3(Normal, Light.NormalizedDirection);
    
    //A 1 is added to DotNormalLight to change its range from [-1,1] to [0,2]. This makes it easier to calculate a percent value.
    // NOTE(not-set): Keep in mind the '-' in front of DotNormalLight.
    real32 PercentColor = (-DotNormalLight + 1) / 2.0f;
    
    Assert(PercentColor >= 0.0f);
    
    Result = GetPercentOfColor(Color, PercentColor);
    
    return Result;
}