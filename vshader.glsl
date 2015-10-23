#version 130

uniform int mode;
uniform mat4 lowerArmTranslation;
uniform mat4 upperArmTranslation;
uniform mat4 baseRotateX;
uniform mat4 baseRotateY;
uniform mat4 baseRotateZ;
uniform mat4 lowerArmRotateX;
uniform mat4 lowerArmRotateY;
uniform mat4 lowerArmRotateZ;
uniform mat4 upperArmRotateX;
uniform mat4 upperArmRotateY;
uniform mat4 upperArmRotateZ;

uniform mat4 modelview;
uniform mat4 projection;
uniform vec4 eyeposition;

uniform vec4 light1_pos;
uniform vec4 light2_pos;
uniform float shininess;

in vec4 vPosition;
in vec4 vNormal;

out vec4 color;
out vec3 fE1;
out vec3 fE2;
out vec3 fN;
out vec3 fL1;
out vec3 fL2;

void
main()
{
    vec4 Position = vPosition, Normal = vNormal;
    if (mode == 1) {
        Position = baseRotateX * baseRotateY * baseRotateZ * lowerArmTranslation * lowerArmRotateX * lowerArmRotateY * lowerArmRotateZ * Position;
        Normal = baseRotateX * baseRotateY * baseRotateZ * lowerArmRotateX * lowerArmRotateY * lowerArmRotateZ * Normal;
    } else if (mode == 2) {
        Position = baseRotateX * baseRotateY * baseRotateZ * lowerArmTranslation * lowerArmRotateX * lowerArmRotateY * lowerArmRotateZ * upperArmTranslation * upperArmRotateX * upperArmRotateY * upperArmRotateZ * Position;
        Normal = baseRotateX * baseRotateY * baseRotateZ * lowerArmRotateX * lowerArmRotateY * lowerArmRotateZ * upperArmRotateX * upperArmRotateY * upperArmRotateZ * Normal;
    } else {
        Position = baseRotateX * baseRotateY * baseRotateZ * Position;
        Normal = baseRotateX * baseRotateY * baseRotateZ * Normal;
    }
    gl_Position = projection * modelview * Position;

    fN = Normal.xyz;
    fE1 = -(modelview*Position).xyz;
    fE2 = (eyeposition - Position).xyz;

    if ( light1_pos.w != 0.0 ) fL1 = light1_pos.xyz - (modelview*Position).xyz;
    else fL1 = light1_pos.xyz;

    if ( light2_pos.w != 0.0 ) fL2 = light2_pos.xyz - Position.xyz;
    else fL2 = light2_pos.xyz;
}
