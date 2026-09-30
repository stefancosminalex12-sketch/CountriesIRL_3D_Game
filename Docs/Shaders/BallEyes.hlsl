// The code of the Custom node in /Game/CrownsAndCommoners/Characters/Materials/M_BallEyes, written out by
// Tools/Unreal/update_ball_eyes.py (this header is not in the node). Draws both countryball eyes on a sphere shell.
// Inputs: P (local position on the shell), EyeScale, UpperLid, UpperLidAngle, LowerLid,
//         Dead (0 alive, 1 x_x, 2 hollow skull sockets), Stretch (the shell's egg stretch), Lashes (0 / 1).
// Output: float2(opacity mask, whiteness) -> x drives Opacity Mask, y drives Base Color.


// The shell may be stretched into the ball's egg shape: undo it so the eyes stay round
float3 d = normalize(P * float3(1.0, 1.0, Stretch));
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

    // Eyelashes (Lashes = 1): three short strokes fanning out from the outer top corner of the eye.
    // They start on the eye's edge (or on the upper lid when it is lowered, so they follow a blink)
    if (Lashes > 0.5)
    {
        float2 qo = float2(q.x * s, q.y);   // x runs toward the outer side of the face
        float lidTop = (UpperLid > 0.01) ? ((H + O) - UpperLid * 2.0 * (H + O)) : 1000.0;
        for (int k = 0; k < 3; k++)
        {
            float t = radians(18.0 + 26.0 * k);
            float2 lashRoot = float2(W * cos(t), min(H * sin(t), lidTop));
            float2 lashDir = normalize(float2(cos(t) * 1.3, sin(t) * 0.8 + 0.3));
            float2 rel = qo - lashRoot;
            float along = clamp(dot(rel, lashDir), 0.0, 0.55 * W);
            if (length(rel - lashDir * along) < 0.95) { mask = 1.0; white = 0.0; }
        }
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
