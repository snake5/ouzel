// Copyright (C) 2015 Elviss Strazdins
// This file is part of the Ouzel engine.

const uint8_t COLOR_VERTEX_SHADER_OGL[] =
    "#version 400\n"
    "layout(location=0) in vec3 in_Position;\n"
    "layout(location=1) in vec4 in_Color;\n"
    "uniform mat4 modelViewProj;\n"
    "out vec4 ex_Color;\n"
    "void main(void)\n"
    "{\n"
    "    gl_Position = modelViewProj * vec4(in_Position, 1.0);\n"
    "    ex_Color = in_Color;\n"
    "}";
