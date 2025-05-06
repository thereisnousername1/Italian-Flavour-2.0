#version 460
layout( triangles ) in;
layout( triangle_strip, max_vertices = 3) out;

uniform mat4 ViewportMatrix; // for transformation into the clip space

out vec3 GNormal;
out vec3 GPosition;

noperspective out vec3 GEdgeDistance;

in vec3 n2[];
in vec3 pos2[];

uniform float time;

vec4 explode(vec4 position, vec3 normal)
{
    float magnitude = 50.0;
    vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude; 
    return position + vec4(direction, 0.0);
} 

void main()
{
	// Transform each vertex into viewport space from clip coordinates 
	//divide by the w coordinate as the clip coordinates are homogeneous 
	//and need to be converted back to true Cartesian coordinates
	vec2 p0 = vec2(ViewportMatrix * (gl_in[0].gl_Position / gl_in[0].gl_Position.w)); 
	vec2 p1 = vec2(ViewportMatrix * (gl_in[1].gl_Position / gl_in[1].gl_Position.w)); 
	vec2 p2 = vec2(ViewportMatrix * (gl_in[2].gl_Position / gl_in[2].gl_Position.w));

	//calculate the edges of the triangle
	float a = length(p1 - p2);
	float b = length(p2 - p0);
	float c = length(p1 - p0);

	//calculate the angles
	float alpha	= acos( (b*b + c*c - a*a) / (2.0*b*c) ); 
	float beta = acos( (a*a + c*c - b*b) / (2.0*a*c) );

	//calculate the distances
	float ha = abs( c * sin(beta) );
	float hb = abs( c * sin(alpha) );
	float hc = abs( b * sin(alpha) );

	//pass the distance and position of the new vertex to the fragment 
	//also pass the normal, position of the mesh
	GEdgeDistance =	vec3( ha, 0, 0);
	GNormal = n2[0];
	GPosition = pos2[0];
	gl_Position = explode(gl_in[0].gl_Position, n2[0]);
    EmitVertex();

	GEdgeDistance =	vec3( 0, hb, 0);
	GNormal = n2[1];
	GPosition = pos2[1];
	gl_Position = explode(gl_in[1].gl_Position, n2[1]);
    EmitVertex();

	GEdgeDistance =	vec3( 0, 0, hc);
	GNormal = n2[2];
	GPosition = pos2[2];
    gl_Position = explode(gl_in[2].gl_Position, n2[2]);
    EmitVertex();

    EndPrimitive();
	
}