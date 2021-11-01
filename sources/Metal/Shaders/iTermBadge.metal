#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

#import "iTermShaderTypes.h"

typedef struct {
    float4 clipSpacePosition [[position]];
    float2 textureCoordinate;
} iTermBadgeVertexFunctionOutput;

vertex iTermBadgeVertexFunctionOutput
iTermBadgeVertexShader(uint vertexID [[ vertex_id ]],
                       constant iTermVertex *vertexArray [[ buffer(iTermVertexInputIndexVertices) ]],
                       constant vector_uint2 *viewportSizePointer  [[ buffer(iTermVertexInputIndexViewportSize) ]]) {
    iTermBadgeVertexFunctionOutput out;

    float2 pixelSpacePosition = vertexArray[vertexID].position.xy;
    float2 viewportSize = float2(*viewportSizePointer);

    out.clipSpacePosition.xy = pixelSpacePosition / viewportSize;
    out.clipSpacePosition.z = 0.0;
    out.clipSpacePosition.w = 1;
    const float2 coord = vertexArray[vertexID].textureCoordinate;
    out.textureCoordinate = float2(coord.x, 1 - coord.y);

    return out;
}

fragment float4
iTermBadgeFragmentShader(iTermBadgeVertexFunctionOutput in [[stage_in]],
                         texture2d<float> texture [[ texture(iTermTextureIndexPrimary) ]]) {
    constexpr sampler textureSampler(mag_filter::linear,
                                     min_filter::linear);
//    return float4(0.75, 0.25, 0.1, 1) * 0.25;
//    return float4(0.694, 0.285, 0.159, 1) * 0.25;
    return texture.sample(textureSampler, in.textureCoordinate);
//    return float4(1, 0.1491, 0, 1) * 0.25;
//    return float4(1, 0.1491, 0, 1);
}

