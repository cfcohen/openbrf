#version 120
//#define varying in

%1

uniform vec3 spec_col;
uniform float spec_exp;
varying vec3 lightDir;
varying vec4 color;
varying vec2 tc;
varying vec3 norm;
varying vec3 tanx;
varying vec3 tany;

uniform sampler2D samplRgb;
uniform sampler2D samplBump;
uniform sampler2D samplSpec;
uniform float alphaLimit;

void main(){
	vec4 tex = texture2D( samplRgb,tc);
#ifdef USE_GREEN_NM
		vec3 normt;
		normt.xy = +texture2D( samplBump,tc).ag * 2.0 - vec2(1.0);
		/*normt.z = 1.0;normalize(normt);*/
		normt.z = sqrt( 1.0-dot(normt.xy,normt.xy));
#else
		vec3 normt = (texture2D( samplBump,tc).xyz * 2.0 )- vec3(1.0);
#endif


	normt = normt.z * norm +
            normt.x * tanx +
            normt.y * tany;
	float diffuse = dot(normt , lightDir ) ;
	diffuse = (diffuse<0.0)?0.0:diffuse;
	diffuse = (diffuse>1.0)?1.0:diffuse;
#ifdef SPECULAR_MAP
	vec3 specmapVal =  texture2D( samplSpec,tc).rgb;
#endif
	gl_FragColor.rgb = tex.rgb*color.rgb*(vec3(diffuse*0.8+0.2)
#ifdef SPECULAR_ALPHA
                     + spec_col*(   tex.a  *pow( diffuse , spec_exp))
#endif
#ifdef SPECULAR_MAP
                     + spec_col*(specmapVal*pow( diffuse , spec_exp))*0.5
#endif
	);
#ifdef ALPHA_CUTOUTS
	if (tex.a<alphaLimit) discard;
	gl_FragColor.a = tex.a;
#else
	gl_FragColor.a = 1.0;
#endif
}
