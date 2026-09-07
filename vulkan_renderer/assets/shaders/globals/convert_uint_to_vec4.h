vec4 ConvertUintToVec4(uint value) {
    return vec4(
        float((value >> 0) & 0xFF),
        float((value >> 8) & 0xFF),
        float((value >> 16) & 0xFF),
        float((value >> 24) & 0xFF)
    ) / 255.0;
}