!!ARBfp1.0
ATTRIB v_color = fragment.color;
ATTRIB v_texcoord = fragment.texcoord;
TEMP temp;

TEX temp, v_texcoord, texture[0], 2D;
MUL result.color, v_color, temp;

END
