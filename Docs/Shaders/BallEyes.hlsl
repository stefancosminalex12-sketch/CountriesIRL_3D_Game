// Source of the Custom node in /Game/CrownsAndCommoners/Characters/Materials/M_BallEyes.
// Draws both countryball eyes on a sphere shell around the ball body.
// Inputs:  P (local position on the shell), EyeScale, UpperLid, UpperLidAngle, LowerLid,
//          Dead (0 = alive, 1 = x_x, 2 = hollow skull sockets)
// Output:  float2(opacity mask, whiteness)  -> x drives Opacity Mask, y drives Base Color
// Eye placement and size constants live only here; UBallFaceComponent drives the parameters.
// The shell mesh is the engine sphere (radius 50 in local space) scaled to just above the body.

float3 d = normalize(P);
float mask = 0.0;
float white = 0.0;

const float Spread = radians(23.0);   // eye yaw from the ball's front
const float Height = radians(13.0);   // eye pitch above the equator
const float Radius = 50.0;            // shell mesh radius in local units (cm)
const float O = 1.6;                  // outline width (cm)
float W = 9.0 * EyeScale;             // half width (cm)
float H = 12.5 * EyeScale;            // half height (cm)

for (int i = 0; i < 2; i++)
{
    float s = (i == 0) ? -1.0 : 1.0;
    float yaw = s * Spread;
    float3 c = float3(cos(Height) * cos(yaw), cos(Height) * sin(yaw), sin(Height));
    if (dot(d, c) < 0.8) continue;

    // Tangent frame at the eye center: r = eye right, u = eye up
    float3 r = float3(-sin(yaw), cos(yaw), 0.0);
    float3 u = normalize(float3(0.0, 0.0, 1.0) - c * c.z);
    float2 q = float2(dot(d, r), dot(d, u)) * Radius;

    if (Dead > 1.5)
    {
        // Skull: hollow black sockets, a little wider than living eyes
        if (length(q / float2(W * 1.15, H)) < 1.0) { mask = 1.0; white = 0.0; }
        continue;
    }

    if (Dead > 0.5)
    {
        // x_x : two crossed bars
        float2 a = float2(0.766, 0.643);
        float d1 = abs(q.x * a.y - q.y * a.x);
        float d2 = abs(q.x * a.y + q.y * a.x);
        if (min(d1, d2) < 1.6 && length(q) < H * 0.9) { mask = 1.0; white = 0.0; }
        continue;
    }

    // Approximate signed distance to the eye ellipse (cm)
    float e = length(q / float2(W, H));
    float dist = (e - 1.0) * min(W, H);
    if (dist > O) continue;

    // Lids: straight cuts across the eye; positive angle lowers the inner corner
    float HO = H + O;
    float ang = radians(UpperLidAngle) * s;
    float qy = -sin(ang) * q.x + cos(ang) * q.y;
    float upperEdge = HO - UpperLid * 2.0 * HO;
    float lowerEdge = -HO + LowerLid * 2.0 * HO;
    bool upperOn = UpperLid > 0.01;
    bool lowerOn = LowerLid > 0.01;
    if (upperOn && qy > upperEdge) continue;
    if (lowerOn && q.y < lowerEdge) continue;

    bool black = dist > 0.0 || (upperOn && qy > upperEdge - O) || (lowerOn && q.y < lowerEdge + O);
    mask = 1.0;
    white = black ? 0.0 : 1.0;
}

return float2(mask, white);
