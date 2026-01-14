#include "Core/OpenGL/OpenGLBackend.h"
#include "Renderer.h"
#include "io/Xml.h"
#include "io/File.h"
#include <string>
#include <string_view>
#include "util/logging.h"
#include <Misc/ImageHelper.h>

namespace Starshine::GFX
{
	using namespace Logging;
	using namespace Core;
	using namespace IO;
	using OpenGLBackend = Core::OpenGL::OpenGLBackend;
	using std::string;
	using std::string_view;

	constexpr const char* LogName = "Starshine::GFX";
	constexpr char ShaderVariableLocationSeparator = ':';

	Renderer* RendererInstance = nullptr;

	struct Renderer::Impl
	{
		RendererBackendType CurrentBackendType{};
		IBackend* CurrentBackend = nullptr;

		void SetBackendType(RendererBackendType type)
		{
			if (CurrentBackend == nullptr)
			{
				CurrentBackendType = type;

				switch (type)
				{
				case RendererBackendType::OpenGL:
				default:
					CurrentBackend = new OpenGLBackend();
					break;
				}
			}
		}

		bool Initialize(SDL_Window* gameWindow)
		{
			LogInfo(LogName, "Backend: %s", RendererBackendTypeNames[static_cast<size_t>(CurrentBackendType)].data());

			CurrentBackend->Initialize(gameWindow);
			return true;
		}

		void Destroy()
		{
			CurrentBackend->Destroy();
			delete CurrentBackend;
			CurrentBackend = nullptr;
		}

		void Clear(ClearFlags flags, const Color& color, f32 depth, u8 stencil)
		{
			CurrentBackend->Clear(flags, color, depth, stencil);
		}
		
		void SwapBuffers()
		{
			CurrentBackend->SwapBuffers();
		}

		Shader* LoadShaderFromXml(const std::string_view basePath, const u8* xmlData, size_t xmlSize)
		{
			if (xmlData == nullptr || xmlSize == 0)
			{
				return nullptr;
			}

			Xml::Document document = Xml::Document();
			document.Parse(reinterpret_cast<const char*>(xmlData), xmlSize);

			if (document.Error())
			{
				LogError(LogName, "Failed to parse shader XML file. Error: %s", document.ErrorStr());
				document.Clear();
				return nullptr;
			}

			Xml::Element* rootElement = document.FirstChildElement("Shader");

			auto findBackendSpecificElement = [&](std::string_view name, Xml::Element* root, Xml::Element** output)
			{
				const Xml::Attribute* backendAttrib = nullptr;
				std::string_view backendName = RendererBackendTypeNames[static_cast<size_t>(CurrentBackendType)];

				while (true)
				{
					*output = Xml::FindElement(root, name);
					backendAttrib = (*output)->FindAttribute("Backend");

					if (SDL_strncmp(backendAttrib->Value(), backendName.data(), backendName.size()) == 0)
					{
						break;
					}
				}
			};


			Xml::Element* filesElement = nullptr;
			findBackendSpecificElement("Files", rootElement, &filesElement);

			Xml::Element* vertexFilePathElement = filesElement->FirstChildElement("Vertex");
			Xml::Element* fragmentFilePathElement = filesElement->FirstChildElement("Fragment");

			string vertexFilePath = string(basePath);
			vertexFilePath.append("/");
			vertexFilePath.append(vertexFilePathElement->GetText());

			string fragmentFilePath = string(basePath);
			fragmentFilePath.append("/");
			fragmentFilePath.append(fragmentFilePathElement->GetText());

			u8* vsFileData = nullptr;
			size_t vsFileSize = IO::File::ReadAllBytes(vertexFilePath, &vsFileData);

			u8* fsFileData = nullptr;
			size_t fsFileSize = IO::File::ReadAllBytes(fragmentFilePath, &fsFileData);

			Shader* shader = CurrentBackend->LoadShader(vsFileData, vsFileSize, fsFileData, fsFileSize);

			if (shader == nullptr)
			{
				document.Clear();
				delete[] vsFileData;
				delete[] fsFileData;
				return nullptr;
			}

			// NOTE: Variable parsing

			Xml::Element* variablesElement = nullptr;
			findBackendSpecificElement("Variables", rootElement, &variablesElement);

			if (variablesElement != nullptr)
			{
				Xml::Element* xmlShaderVariable = variablesElement->FirstChildElement();
				while (xmlShaderVariable != nullptr)
				{
					string_view varTypeString = xmlShaderVariable->Name();

					const Xml::Attribute* nameAttrib = xmlShaderVariable->FindAttribute("Name");
					const Xml::Attribute* locationAttrib = xmlShaderVariable->FindAttribute("Location");

					if (nameAttrib != nullptr && locationAttrib != nullptr)
					{
						string_view varName = nameAttrib->Value();
						string_view varLocation = locationAttrib->Value();

						string_view varLocationShader;
						string_view varLocationIndex;

						for (size_t i = 0; i < varLocation.size(); i++)
						{
							if (varLocation.at(i) == ShaderVariableLocationSeparator)
							{
								varLocationShader = varLocation.substr(0, i);
								varLocationIndex = varLocation.substr(i + 1, varLocation.size() - (i + 1));
								break;
							}
						}

						ShaderVariable shaderVar = {};
						shaderVar.Name = varName;
						shaderVar.Type = EnumFromString<ShaderVariableType>(ShaderVariableTypeStrings, varTypeString);
						shaderVar.LocationShader = EnumFromString<ShaderType>(ShaderTypeStrings, varLocationShader);
						shaderVar.LocationIndex = SDL_atoi(varLocationIndex.data());

						shader->AddVariable(shaderVar);
					}

					xmlShaderVariable = xmlShaderVariable->NextSiblingElement();
				}
			}

			document.Clear();
			delete[] vsFileData;
			delete[] fsFileData;
			return shader;
		}

		Texture* LoadTextureFromFile(const u8* fileData, size_t fileSize, bool nearestFilter, bool clamp)
		{
			if (fileData == nullptr || fileSize == 0)
			{
				return nullptr;
			}

			ivec2 size{};
			i32 channels{};
			std::unique_ptr<u8[]> rgbaData;

			if (!Misc::ImageHelper::ReadImageFile(fileData, fileSize, size, channels, rgbaData)) { return nullptr; }

			Texture* texture = CurrentBackend->CreateTexture(size.x, size.y, TextureFormat::RGBA8, nearestFilter, clamp);
			if (texture == nullptr) { return nullptr; }

			texture->SetData(0, 0, size.x, size.y, rgbaData.get());
			return texture;
		}
	};

	Renderer::Renderer(RendererBackendType backend) : impl(new Impl())
	{
		impl->SetBackendType(backend);
	}

	void Renderer::CreateInstance(RendererBackendType backend)
	{
		if (RendererInstance == nullptr)
		{
			RendererInstance = new Renderer(backend);
		}
	}

	void Renderer::DeleteInstance()
	{
		if (RendererInstance != nullptr)
		{
			delete RendererInstance;
		}
	}

	Renderer* Renderer::GetInstance()
	{
		assert(RendererInstance != nullptr);
		return RendererInstance;
	}

	bool Renderer::Initialize(SDL_Window* gameWindow)
	{
		return impl->Initialize(gameWindow);
	}

	void Renderer::Destroy()
	{
		impl->Destroy();
	}

	RendererBackendType Renderer::GetType() const
	{
		return impl->CurrentBackend->GetType();
	}

	RectangleF Renderer::GetViewportSize() const
	{
		return impl->CurrentBackend->GetViewportSize();
	}

	void Renderer::Clear(ClearFlags flags, const Color& color, f32 depth, u8 stencil)
	{
		impl->Clear(flags, color, depth, stencil);
	}

	void Renderer::SwapBuffers()
	{
		impl->SwapBuffers();
	}

	void Renderer::SetBlendState(bool enable, BlendFactor srcColor, BlendFactor destColor, BlendFactor srcAlpha, BlendFactor destAlpha)
	{
		impl->CurrentBackend->SetBlendState(enable, srcColor, destColor, srcAlpha, destAlpha);
	}

	void Renderer::SetBlendOperation(BlendOperation op)
	{
		impl->CurrentBackend->SetBlendOperation(op);
	}

	void Renderer::DrawArrays(PrimitiveType type, u32 firstVertex, u32 vertexCount)
	{
		impl->CurrentBackend->DrawArrays(type, firstVertex, vertexCount);
	}

	void Renderer::DrawIndexed(PrimitiveType type, u32 firstIndex, u32 indexCount)
	{
		impl->CurrentBackend->DrawIndexed(type, firstIndex, indexCount);
	}

	VertexBuffer* Renderer::CreateVertexBuffer(size_t size, void* initialData, bool dynamic)
	{
		return impl->CurrentBackend->CreateVertexBuffer(size, initialData, dynamic);
	}

	IndexBuffer* Renderer::CreateIndexBuffer(size_t size, IndexFormat format, void* initialData, bool dynamic)
	{
		return impl->CurrentBackend->CreateIndexBuffer(size, format, initialData, dynamic);
	}

	VertexDesc* Renderer::CreateVertexDesc(const VertexAttrib* attribs, size_t attribCount)
	{
		return impl->CurrentBackend->CreateVertexDesc(attribs, attribCount);
	}

	Shader* Renderer::LoadShader(const u8* vsData, size_t vsSize, const u8* fsData, size_t fsSize)
	{
		return impl->CurrentBackend->LoadShader(vsData, vsSize, fsData, fsSize);
	}

	Shader* Renderer::LoadShaderFromXml(const u8* xmlData, size_t xmlSize)
	{
		return impl->LoadShaderFromXml("", xmlData, xmlSize);
	}

	Shader* Renderer::LoadShaderFromXml(const std::string_view filePath)
	{
		string_view basePath = File::GetParentDirectory(filePath);

		u8* xmlShaderData = nullptr;
		size_t xmlShaderSize = File::ReadAllBytes(filePath, &xmlShaderData);

		if (xmlShaderData == nullptr || xmlShaderSize == 0)
		{
			return nullptr;
		}

		Shader* shader = impl->LoadShaderFromXml(basePath, xmlShaderData, xmlShaderSize);

		delete[] xmlShaderData;
		return shader;
	}

	Texture* Renderer::CreateTexture(u32 width, u32 height, TextureFormat format, bool nearestFilter, bool clamp)
	{
		return impl->CurrentBackend->CreateTexture(width, height, format, nearestFilter, clamp);
	}

	Texture* Renderer::LoadTexture(const u8* fileData, size_t fileSize, bool nearestFilter, bool clamp)
	{
		return impl->LoadTextureFromFile(fileData, fileSize, nearestFilter, clamp);
	}

	Texture* Renderer::LoadTexture(const std::string_view filePath, bool nearestFilter, bool clamp)
	{
		u8* fileData = nullptr;
		size_t fileSize = File::ReadAllBytes(filePath, &fileData);

		if (fileData == nullptr || fileSize == 0)
		{
			return nullptr;
		}

		Texture* texture = impl->LoadTextureFromFile(fileData, fileSize, nearestFilter, clamp);

		delete[] fileData;
		return texture;
	}

	void Renderer::DeleteResource(Resource* resource)
	{
		if (resource == nullptr)
		{
			return;
		}
		impl->CurrentBackend->DeleteResource(resource);
	}

	void Renderer::SetVertexBuffer(const VertexBuffer* buffer)
	{
		impl->CurrentBackend->SetVertexBuffer(buffer);
	}

	void Renderer::SetIndexBuffer(const IndexBuffer* buffer)
	{
		impl->CurrentBackend->SetIndexBuffer(buffer);
	}

	void Renderer::SetVertexDesc(const VertexDesc* desc)
	{
		impl->CurrentBackend->SetVertexDesc(desc);
	}

	void Renderer::SetShader(Shader* shader)
	{
		impl->CurrentBackend->SetShader(shader);
	}

	void Renderer::SetTexture(const Texture* texture, u32 slot)
	{
		impl->CurrentBackend->SetTexture(texture, slot);
	}
}
