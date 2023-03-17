#version 400 core

uniform sampler2D mainTex;
uniform sampler2D edgesTex;
uniform int width;
uniform int height;

uniform bool edgeDetection;

vec2 frameBufSize = vec2(width,height);
in Vertex
{
	
	vec2 texCoord;
} IN;

out vec4 fragColor[2];

void main( void ) {
    
    

    if(edgeDetection){
        //fragColor = vec4(1);return;
        vec3 edge = texture2D(edgesTex,IN.texCoord).xyz;
        bool isOnEdge = length(edge) > 0.1;
        if(!isOnEdge){
            fragColor[0] = texture2D(mainTex,IN.texCoord);
        return;
        }
    }

    float FXAA_SPAN_MAX = 8.0;
    float FXAA_REDUCE_MUL = 1.0/8.0;
    float FXAA_REDUCE_MIN = 1.0/128.0;

    vec3 rgbNW=texture2D(mainTex,IN.texCoord+(vec2(-1.0,-1.0)/frameBufSize)).xyz;
    vec3 rgbNE=texture2D(mainTex,IN.texCoord+(vec2(1.0,-1.0)/frameBufSize)).xyz;
    vec3 rgbSW=texture2D(mainTex,IN.texCoord+(vec2(-1.0,1.0)/frameBufSize)).xyz;
    vec3 rgbSE=texture2D(mainTex,IN.texCoord+(vec2(1.0,1.0)/frameBufSize)).xyz;
    vec3 rgbM=texture2D(mainTex,IN.texCoord).xyz;
    

    vec3 luma=vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max(
        (lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),
        FXAA_REDUCE_MIN);

    float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX),
          max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
          dir * rcpDirMin)) / frameBufSize;

    vec3 rgbA = (1.0/2.0) * (
        texture2D(mainTex, IN.texCoord.xy + dir * (1.0/3.0 - 0.5)).xyz +
        texture2D(mainTex, IN.texCoord.xy + dir * (2.0/3.0 - 0.5)).xyz);
    vec3 rgbB = rgbA * (1.0/2.0) + (1.0/4.0) * (
        texture2D(mainTex, IN.texCoord.xy + dir * (0.0/3.0 - 0.5)).xyz +
        texture2D(mainTex, IN.texCoord.xy + dir * (3.0/3.0 - 0.5)).xyz);
    float lumaB = dot(rgbB, luma);

    
    
    if((lumaB < lumaMin) || (lumaB > lumaMax)){
        fragColor[0]=vec4(rgbA,1);
    }else{
        fragColor[0]=vec4(rgbB,1);
    }
    fragColor[1] =vec4(0);
    fragColor[0] = vec4(1);
}