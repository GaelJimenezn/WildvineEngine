cbuffer ParticleConstants : register(b0)
{
    float4x4 ViewProjection;
}

struct VSIn
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct VSOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VSOut
VS(VSIn input)
{
    VSOut output;
    output.position = mul(float4(input.position, 1.0f), ViewProjection);
    output.color = input.color;
    return output;
}

float4
PS(VSOut input) : SV_Target
{
    return input.color;
}
