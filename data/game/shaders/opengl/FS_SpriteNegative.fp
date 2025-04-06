!!ARBfp1.0
ATTRIB v_color = fragment.color;
ATTRIB v_texcoord = fragment.texcoord;
TEMP temp;
TEMP temp2;

TEX temp, v_texcoord, texture[0], 2D;
SUB temp2, { 1.0, 1.0, 1.0, 1.0 }, temp;
MOV temp2.a, temp.a;
MUL result.color, v_color, temp2;

END
