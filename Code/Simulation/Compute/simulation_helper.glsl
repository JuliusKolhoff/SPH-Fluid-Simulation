layout(std140, binding = 1) uniform Info {
    uint count;
} info;

#define COUNT (info.count)

layout(std430, binding = 0)  buffer PosBuf      { vec4  pos[];      };  // xyz, w unused
layout(std430, binding = 1)  buffer ColorBuf    { vec4  color[];    };  // rgb, a unused
layout(std430, binding = 2)  buffer VelBuf      { vec4  vel[];      };  // xyz, w unused
layout(std430, binding = 3)  buffer MassBuf     { float mass[];     };
layout(std430, binding = 4)  buffer DensityBuf  { vec2  density[];  };  // x = density, y = second value (e.g. near density)
layout(std430, binding = 5)  buffer PressureBuf { float pressure[]; };
layout(std430, binding = 6)  buffer MovableBuf  { uint  movable[]; };  // 0 = static, 1 = movable