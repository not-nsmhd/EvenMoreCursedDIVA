#include "HalfPixelFixup.h"
#include <vector>

namespace Starshine::Rendering::D3D9::Detail
{
	// Implementation: https://gist.github.com/aras-p/c2ea7b45ff3fbd5312eb9904c4bb8415

	static constexpr u32 kD3D9ShaderTypeVertex = 0xFFFE0000;

	static constexpr u32 kD3D9SwizzleShift = 16;
	static constexpr u32 kD3D9NoSwizzle = ((0 << kD3D9SwizzleShift) | (1 << (kD3D9SwizzleShift + 2)) | (2 << (kD3D9SwizzleShift + 4)) | (3 << (kD3D9SwizzleShift + 6)));

	static constexpr u32 kD3D9WriteMaskX = 0x00010000;
	static constexpr u32 kD3D9WriteMaskY = 0x00020000;
	static constexpr u32 kD3D9WriteMaskZ = 0x00040000;
	static constexpr u32 kD3D9WriteMaskW = 0x00080000;

	enum D3D9Opcode
	{
		kD3D9Op_NOP = 0,
		kD3D9Op_MOV,
		kD3D9Op_ADD,
		kD3D9Op_SUB,
		kD3D9Op_MAD,
		kD3D9Op_MUL,
		kD3D9Op_RCP,
		kD3D9Op_RSQ,
		kD3D9Op_DP3,
		kD3D9Op_DP4,
		kD3D9Op_MIN,
		kD3D9Op_MAX,
		kD3D9Op_SLT,
		kD3D9Op_SGE,
		kD3D9Op_EXP,
		kD3D9Op_LOG,
		kD3D9Op_LIT,
		kD3D9Op_DST,
		kD3D9Op_LRP,
		kD3D9Op_FRC,
		kD3D9Op_M4x4,
		kD3D9Op_M4x3,
		kD3D9Op_M3x4,
		kD3D9Op_M3x3,
		kD3D9Op_M3x2,
		kD3D9Op_CALL,
		kD3D9Op_CALLNZ,
		kD3D9Op_LOOP,
		kD3D9Op_RET,
		kD3D9Op_ENDLOOP,
		kD3D9Op_LABEL,
		kD3D9Op_DCL,
		kD3D9Op_POW,
		kD3D9Op_CRS,
		kD3D9Op_SGN,
		kD3D9Op_ABS,
		kD3D9Op_NRM,
		kD3D9Op_SINCOS,
		kD3D9Op_REP,
		kD3D9Op_ENDREP,
		kD3D9Op_IF,
		kD3D9Op_IFC,
		kD3D9Op_ELSE,
		kD3D9Op_ENDIF,
		kD3D9Op_BREAK,
		kD3D9Op_BREAKC,
		kD3D9Op_MOVA,
		kD3D9Op_DEFB,
		kD3D9Op_DEFI,

		kD3D9Op_TEXCOORD = 64,
		kD3D9Op_TEXKILL,
		kD3D9Op_TEX,
		kD3D9Op_TEXBEM,
		kD3D9Op_TEXBEML,
		kD3D9Op_TEXREG2AR,
		kD3D9Op_TEXREG2GB,
		kD3D9Op_TEXM3x2PAD,
		kD3D9Op_TEXM3x2TEX,
		kD3D9Op_TEXM3x3PAD,
		kD3D9Op_TEXM3x3TEX,
		kD3D9Op_RESERVED0,
		kD3D9Op_TEXM3x3SPEC,
		kD3D9Op_TEXM3x3VSPEC,
		kD3D9Op_EXPP,
		kD3D9Op_LOGP,
		kD3D9Op_CND,
		kD3D9Op_DEF,
		kD3D9Op_TEXREG2RGB,
		kD3D9Op_TEXDP3TEX,
		kD3D9Op_TEXM3x2DEPTH,
		kD3D9Op_TEXDP3,
		kD3D9Op_TEXM3x3,
		kD3D9Op_TEXDEPTH,
		kD3D9Op_CMP,
		kD3D9Op_BEM,
		kD3D9Op_DP2ADD,
		kD3D9Op_DSX,
		kD3D9Op_DSY,
		kD3D9Op_TEXLDD,
		kD3D9Op_SETP,
		kD3D9Op_TEXLDL,
		kD3D9Op_BREAKP,

		kD3D9Op_PHASE = 0xFFFD,
		kD3D9Op_COMMENT = 0xFFFE,
		kD3D9Op_END = 0xFFFF,
	};

	enum D3D9Register
	{
		kD3D9Reg_NONE = -1,
		kD3D9Reg_TEMP = 0,
		kD3D9Reg_INPUT = 1,
		kD3D9Reg_CONST = 2,
		kD3D9Reg_ADDR_or_TEXTURE = 3,
		kD3D9Reg_RASTOUT = 4,
		kD3D9Reg_ATTROUT = 5,
		kD3D9Reg_OUTPUT = 6,
		kD3D9Reg_CONST_INT = 7,
		kD3D9Reg_COLOROUT = 8,
		kD3D9Reg_DEPTHOUT = 9,
		kD3D9Reg_SAMPLER = 10,
		kD3D9Reg_CONST2 = 11, // constants 2048..4095
		kD3D9Reg_CONST3 = 12, // constants 4096..6143
		kD3D9Reg_CONST4 = 13, // constants 6144..8191
		kD3D9Reg_CONST_BOOL = 14,
		kD3D9Reg_LOOP = 15,
		kD3D9Reg_TEMPFLOAT16 = 16, // temp for half-precision floats
		kD3D9Reg_MISC = 17,
		kD3D9Reg_LABEL = 18, // label pseudo-register
		kD3D9Reg_PREDICATE = 19,
	};

	static void DecodeShaderVersionD3D9(u32 token, u32* outType, u32* outMajor, u32* outMinor)
	{
		*outType = token & 0xFFFF0000;
		*outMajor = (token >> 8) & 0xFF;
		*outMinor = token & 0xFF;
	}

	static D3D9Opcode DecodeOpcode(u32 token)
	{
		return (D3D9Opcode)(token & 0x0000FFFF);
	}

	static u32 DecodeInstructionLength(u32 token)
	{
		return (token & 0x0F000000) >> 24;
	}

	static u32 DecodeCommentLength(u32 token)
	{
		return (token & 0x7FFF0000) >> 16;
	}

	static u32 DecodeRegisterIndex(u32 token)
	{
		return token & 0x7FF;
	}

	static u32 EncodeRegisterIndex(int index)
	{
		return index & 0x7FF;
	}

	static D3D9Register DecodeRegisterType(u32 token)
	{
		return (D3D9Register)(((token & 0x70000000) >> 28) | ((token & 0x00001800) >> 8));
	}

	static u32 EncodeRegisterType(D3D9Register type)
	{
		return ((type & 7) << 28) | ((type & 0x18) << 8);
	}

	static u32 EncodeReplicateSwizzle(u32 comp)
	{
		return (comp << kD3D9SwizzleShift) | (comp << (kD3D9SwizzleShift + 2)) | (comp << (kD3D9SwizzleShift + 4)) | (comp << (kD3D9SwizzleShift + 6));
	}

	static bool NextToken(const std::vector<u32>& byteCode, size_t& inOutIndex)
	{
		if (inOutIndex >= byteCode.size())
			return false;
		const u32 token = byteCode[inOutIndex];
		const D3D9Opcode op = DecodeOpcode(token);
		u32 length = DecodeInstructionLength(token);
		// comment instructions have different length encoding
		if (op == kD3D9Op_COMMENT)
			length = DecodeCommentLength(token);
		inOutIndex += length + 1;
		if (op == kD3D9Op_END)
			return false;
		return true;
	}

	// "regular" instructions have destination + source registers right after instruction token
	static bool IsRegularInstruction(D3D9Opcode op)
	{
		if (op == kD3D9Op_END || op == kD3D9Op_COMMENT || op == kD3D9Op_DCL || op == kD3D9Op_DEF || op == kD3D9Op_DEFI || op == kD3D9Op_DEFB)
			return false;
		return true;
	}


	static int FindUnusedTempRegisterD3D9(const std::vector<u32>& byteCode)
	{
		size_t index = 1;
		// Find max used temporary register slot.
		// HLSL compiler does fairly tight temporary register allocation,
		// so we'll go with "max used + 1" as the "free register".
		int maxUsed = -1;
		do
		{
			const D3D9Opcode op = DecodeOpcode(byteCode[index]);
			if (IsRegularInstruction(op))
			{
				const u32 length = DecodeInstructionLength(byteCode[index]);
				for (u32 i = 0; i < length; ++i)
				{
					u32 token = byteCode[index + 1 + i];
					D3D9Register type = DecodeRegisterType(token);
					if (type == kD3D9Reg_TEMP)
					{
						int regIndex = DecodeRegisterIndex(token);
						if (regIndex > maxUsed)
							maxUsed = regIndex;
					}
				}
			}

		} while (NextToken(byteCode, index));
		return maxUsed + 1;
	}

	// Finds output position register index in VS3.0
	static int FindPositionOutputRegisterD3D9(const std::vector<u32>& byteCode)
	{
		size_t index = 1;
		do
		{
			const D3D9Opcode op = DecodeOpcode(byteCode[index]);
			if (op == kD3D9Op_DCL)
			{
				const u32 length = DecodeInstructionLength(byteCode[index]);
				if (length >= 2)
				{
					u32 token1 = byteCode[index + 1];
					u32 token2 = byteCode[index + 2];
					if (token1 == 0x80000000 && DecodeRegisterType(token2) == kD3D9Reg_OUTPUT)
					{
						return DecodeRegisterIndex(token2);
					}
				}
			}

		} while (NextToken(byteCode, index));

		return -1;
	}

	// Rewrites all usages of "src" register into "dst" one.
	static void RewriteRegisterD3D9(std::vector<u32>& byteCode, D3D9Register srcType, int srcIndex, D3D9Register dstType, int dstIndex)
	{
		size_t index = 1;
		do
		{
			const D3D9Opcode op = DecodeOpcode(byteCode[index]);
			if (IsRegularInstruction(op))
			{
				const u32 length = DecodeInstructionLength(byteCode[index]);
				for (u32 i = 0; i < length; ++i)
				{
					u32& token = byteCode[index + 1 + i];
					if (DecodeRegisterType(token) == srcType && DecodeRegisterIndex(token) == srcIndex)
					{
						token &= ~0x70001800; // clear register type
						token |= EncodeRegisterType(dstType);
						token &= ~0x000007FF; // clear register index
						token |= EncodeRegisterIndex(dstIndex);
					}
				}
			}

		} while (NextToken(byteCode, index));
	}

	static size_t FindEndOfShaderD3D9(const std::vector<u32>& byteCode)
	{
		size_t index = 1;
		do
		{
			const D3D9Opcode op = DecodeOpcode(byteCode[index]);
			if (op == kD3D9Op_END)
				return index;
		} while (NextToken(byteCode, index));
		return index;
	}

	std::unique_ptr<u32[]> HalfPixelFixup::GetPatchedVertexShaderBytecode(const u8* bytecode, size_t bytecodeSize)
	{
		std::vector<u32> bytecodeTokens;
		size_t tokenCount = bytecodeSize / 4;
		bytecodeTokens.reserve(tokenCount);

		for (size_t i = 0; i < tokenCount; i++)
		{
			bytecodeTokens.push_back(reinterpret_cast<const u32*>(bytecode)[i]);
		}

		u32 shaderType, shaderVersionMajor, shaderVersionMinor{};
		DecodeShaderVersionD3D9(bytecodeTokens[0], &shaderType, &shaderVersionMajor, &shaderVersionMinor);

		if (shaderType != kD3D9ShaderTypeVertex) { return nullptr; }
		if (shaderVersionMajor != 2 || shaderVersionMinor != 0) { return nullptr; }

		D3D9Register positionType = kD3D9Reg_RASTOUT;
		int positionIndex = 0;

		int tempIndex = FindUnusedTempRegisterD3D9(bytecodeTokens);
		if (tempIndex >= 12) { return nullptr; }

		RewriteRegisterD3D9(bytecodeTokens, positionType, positionIndex, kD3D9Reg_TEMP, tempIndex);

		size_t insertPos = FindEndOfShaderD3D9(bytecodeTokens);
		bytecodeTokens.insert(bytecodeTokens.begin() + insertPos, 8, 0); // 5 tokens for mad, 3 tokens for mov
		// mad oPos.xy, tmpPos.w, constFixup, tmpPos
		bytecodeTokens[insertPos + 0] = kD3D9Op_MAD + (4 << 24);
		bytecodeTokens[insertPos + 1] = EncodeRegisterIndex(positionIndex) | EncodeRegisterType(positionType) | kD3D9WriteMaskX | kD3D9WriteMaskY | 0x80000000; // oPos.xy
		bytecodeTokens[insertPos + 2] = EncodeRegisterIndex(tempIndex) | EncodeRegisterType(kD3D9Reg_TEMP) | EncodeReplicateSwizzle(3) | 0x80000000; // tmpPos.w
		bytecodeTokens[insertPos + 3] = EncodeRegisterIndex(255) | EncodeRegisterType(kD3D9Reg_CONST) | kD3D9NoSwizzle | 0x80000000; // constFixup
		bytecodeTokens[insertPos + 4] = EncodeRegisterIndex(tempIndex) | EncodeRegisterType(kD3D9Reg_TEMP) | kD3D9NoSwizzle | 0x80000000; // tmpPos
		// mov oPos.zw, tmpPos
		bytecodeTokens[insertPos + 5] = kD3D9Op_MOV + (2 << 24);
		bytecodeTokens[insertPos + 6] = EncodeRegisterIndex(positionIndex) | EncodeRegisterType(positionType) | kD3D9WriteMaskZ | kD3D9WriteMaskW | 0x80000000; // oPos.zw;
		bytecodeTokens[insertPos + 7] = EncodeRegisterIndex(tempIndex) | EncodeRegisterType(kD3D9Reg_TEMP) | kD3D9NoSwizzle | 0x80000000; // tmpPos

		std::unique_ptr<u32[]> patchedBytecode = std::make_unique<u32[]>(bytecodeTokens.size());
		::memcpy(patchedBytecode.get(), bytecodeTokens.data(), bytecodeTokens.size() * sizeof(u32));
		bytecodeTokens.clear();

		return patchedBytecode;
	}
}
