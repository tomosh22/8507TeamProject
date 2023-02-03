#version 400 core
#extension GL_ARB_shader_storage_buffer_object :     enable

uniform vec4 		objectColour;
uniform sampler2D 	mainTex;
uniform sampler2DShadow shadowTex;

uniform vec3	lightPos;
uniform float	lightRadius;
uniform vec4	lightColour;

uniform vec3	cameraPos;

uniform bool hasTexture;

uniform bool isPaintable;

uniform int width;
uniform int height;

layout(std430, binding = 4) buffer PaintSSBO{
	int paintData[];
};

in Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
	vec3 worldPos;
} IN;

out vec4 fragColor;

bool isAdjacentToInk(){
	float ut = (IN.texCoord.x * width) - floor(IN.texCoord.x * width);
	float vt = (IN.texCoord.y * height) - floor(IN.texCoord.y * height);
	int u = int(floor(IN.texCoord.x * width));
	int v = int(floor(IN.texCoord.y * height));
	int dataIndex;
	if(ut < 0.5f && vt < 0.5f){
		u = max(u-1,0);
		v = max(v-1,0);
		dataIndex = v * width + u;
	}
	if(ut > 0.5f && vt < 0.5f){
		u = min(u+1,width-1);
		v = max(v-1,0);
		dataIndex = v * width + u;
	}
	if(ut < 0.5f && vt > 0.5f){
		u = max(u-1,0);
		v = min(v+1,height-1);
		dataIndex = v * width + u;
	}
	if(ut > 0.5f && vt > 0.5f){
		u = min(u+1,width-1);
		v = min(v+1,height-1);
		dataIndex = v * width + u;
	}
	
	return paintData[dataIndex] == 1;
}

void main(void)
{
	float shadow = 1.0; // New !
	
	if( IN . shadowProj . w > 0.0) { // New !
		shadow = textureProj ( shadowTex , IN . shadowProj ) * 0.5f;
	}

	vec3  incident = normalize ( lightPos - IN.worldPos );
	float lambert  = max (0.0 , dot ( incident , IN.normal )) * 0.9; 
	
	vec3 viewDir = normalize ( cameraPos - IN . worldPos );
	vec3 halfDir = normalize ( incident + viewDir );

	float rFactor = max (0.0 , dot ( halfDir , IN.normal ));
	float sFactor = pow ( rFactor , 80.0 );
	
	vec4 albedo = IN.colour;
	
	if(hasTexture) {
	 albedo *= texture(mainTex, IN.texCoord);
	}
	
	albedo.rgb = pow(albedo.rgb, vec3(2.2));
	
	fragColor.rgb = albedo.rgb * 0.05f; //ambient
	
	fragColor.rgb += albedo.rgb * lightColour.rgb * lambert * shadow; //diffuse light
	
	fragColor.rgb += lightColour.rgb * sFactor * shadow; //specular light
	
	fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / 2.2f));
	
	fragColor.a = albedo.a;

	fragColor.rbg += height + width;
	fragColor = vec4(0);
	fragColor.a = 1;
	fragColor.rg = IN.texCoord;

	

	int dataIndex = int(floor(IN.texCoord.y * height)) * width + int(floor(IN.texCoord.x * width));
	if(paintData[dataIndex] == 1 || isAdjacentToInk()){fragColor = vec4(0,0,1,1);}

//fragColor.rgb = IN.normal;

	//fragColor = IN.colour;
	
	//fragColor.xy = IN.texCoord.xy;
	
	//fragColor = IN.colour;
}

// needs to go back to line 84