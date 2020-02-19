#pragma once
class vec4 {
public:
    vec4() {}
    vec4(float e0, float e1, float e2, float e3) { e[0] = e0; e[1] = e1; e[2] = e2; e[3] = e3; }


    float e[4];
};