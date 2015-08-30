//======================================================================================
// Constant buffers
//======================================================================================

cbuffer VSConstants : register (b0)
{
    float4x4 View : packoffset(c0);
    float4x4 Projection : packoffset(c4);
    float3 Bias : packoffset(c8);    
}

//======================================================================================
// Samplers
//======================================================================================

TextureCube	EnvironmentMap : register(t0);
SamplerState LinearSampler : register(s0);


//======================================================================================
// Input/Output structs
//======================================================================================

struct VSInput
{
    float3 PositionOS : POSITION;
};

struct VSOutput
{
    float4 PositionCS : SV_Position;
    float3 TexCoord : TEXCOORD;
    float3 Bias : BIAS;    
};

//======================================================================================
// Vertex Shader
//======================================================================================
VSOutput SkyboxVS(in VSInput input)
{
    VSOutput output;

    // Rotate into view-space, centered on the camera
    float3 positionVS = mul(input.PositionOS.xyz, (float3x3)View);    
    
    // Transform to clip-space
    output.PositionCS = mul(float4(positionVS, 1.0f), Projection);

    // Make a texture coordinate
    output.TexCoord = input.PositionOS;
    
    // Pass along the bias
    output.Bias = Bias;

    return output;
}

//======================================================================================
// Pixel Shader
//======================================================================================
float4 SkyboxPS(in VSOutput input) : SV_Target
{
    // Sample the skybox
    float3 texColor = EnvironmentMap.Sample(LinearSampler, normalize(input.TexCoord)).rgb;

    // Apply the bias
    texColor = texColor * input.Bias;
    
    return float4(texColor, 1.0f);    
}
