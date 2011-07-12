//--------------------------------------------------------------------------------------
// DefaultShaders.hlsl
//
// Simple Vertex and Pixel Shader, Default
//
// Xbox Advanced Technology Group.
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

struct VertexToPixel
{
	float4 Position			: POSITION;
	float4 Color			: COLOR0;
};

struct PixelToFrame
{
    float4 Color			: COLOR0;
};


//--------------------------------------------------------------------------------------
// Name:  Vertex Shader Constants
//--------------------------------------------------------------------------------------
float4x4 xViewProjection;

//--------------------------------------------------------------------------------------
// Name: SimpleVertexShader()
// Desc: Basic vertex shader
//--------------------------------------------------------------------------------------
VertexToPixel SimpleVertexShader( float4 inPos : POSITION )
{
    VertexToPixel Output = (VertexToPixel)0;				//Erase previous output
	
	Output.Position = mul(inPos, xViewProjection);			//Translate 3D Vertex To Screen Coord
	Output.Color = 1.0f;									//Set every vertex to white
    
    return Output;    
}

//--------------------------------------------------------------------------------------
// Name: SimpleVertexShader()
// Desc: Basic pixel shader
//--------------------------------------------------------------------------------------
PixelToFrame SimplePixelShader(VertexToPixel PSIn)
{
    PixelToFrame Output = (PixelToFrame)0;

    Output.Color = PSIn.Color;

    return Output;
}