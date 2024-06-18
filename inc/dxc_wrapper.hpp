
// 

#if !defined(DXC_WRAPPER_HEADER)
#define DXC_WRAPPER_HEADER

#if defined(ENABLE_DXCW_DEBUG)
	#define DXCW_DEBUG 1
#else 
	#define DXCW_DEBUG 0
#endif 

#if defined(DXCW_USE_MODUDLE)
	#if !defined(_WIN32)
		#error cross platform is not under consideration when using module.
	#endif // !

	#include "..\DXC\inc\d3d12shader.h"
	// dxc header file.
	#include "..\DXC\inc\dxcapi.h"

	#define DXCW_IMPORT(X) import X
	#define	DXCW_MODULE(X) module X
	#define DXCW_EXPORT    export
#else
	// TODO: shader reflection wasnt be wrapped, finish in future maybe.
	#include "..\DXC\inc\d3d12shader.h"
	// dxc header file.
	#include "..\DXC\inc\dxcapi.h"

	#define DXCW_IMPORT(X)
	#define	DXCW_MODULE(X)
	#define DXCW_EXPORT
#endif

#pragma comment(lib, "dxcompiler.lib")

DXCW_MODULE(dxcw);
DXCW_IMPORT("..\DXC\inc\d3d12shader.h");
DXCW_IMPORT("..\DXC\inc\dxcapi.h");

DXCW_EXPORT namespace dxcw
{
#define DXCWISC inline static constexpr
#define DXCWICA inline constexpr auto
#define DXCWIC inline constexpr

	// Consume an file is an pipeline, thus every entry point name is fixed.
	inline LPCWSTR ShaderTypeToEntryName(D3D12_SHADER_VERSION_TYPE type)
	{
		switch (type)
		{
		case D3D12_SHVER_PIXEL_SHADER: return L"PSmain";
		case D3D12_SHVER_VERTEX_SHADER: return L"VSmain";
		case D3D12_SHVER_GEOMETRY_SHADER: return L"GSmain";
		case D3D12_SHVER_HULL_SHADER: return L"HSmain";
		case D3D12_SHVER_DOMAIN_SHADER: return L"DSmain";
		case D3D12_SHVER_COMPUTE_SHADER: return L"CSmain";
		default: return nullptr;
		}
	}

	inline LPCWSTR ShaderTypeLowerCase(D3D12_SHADER_VERSION_TYPE type)
	{
		switch (type)
		{
		case D3D12_SHVER_PIXEL_SHADER: return L"ps";
		case D3D12_SHVER_VERTEX_SHADER: return L"vs";
		case D3D12_SHVER_GEOMETRY_SHADER: return L"gs";
		case D3D12_SHVER_HULL_SHADER: return L"hs";
		case D3D12_SHVER_DOMAIN_SHADER: return L"ds";
		case D3D12_SHVER_COMPUTE_SHADER: return L"cs";
		default: return nullptr;
		}
	}

	template<typename T>
	struct span
	{
		span(T* pointer, ::UINT count) : Value{ pointer }, Count{count} {}

		template<typename...Args>
		span(Args&&...);

		T* const Value;
		::UINT const Count;
	};

	template<typename T> 
	struct guard 
	{
		~guard() { if(ptr) ptr->Release(); }

		T** put() { return &ptr; }

		T* ptr{nullptr};
	};

	struct CompileResult
	{
		~CompileResult() { if(Message)::HeapFree(::GetProcessHeap(), NULL, Message); }

		char* Message{nullptr};
	};

	// @brief Compile Argument.
	// Dirty constants namesapce.
	namespace Arguments
	{
		// TODO: future support?
		//DXCWICA NoVaildation{DXC_ARG_SKIP_VALIDATION};

		DXCWICA Debug       { DXC_ARG_DEBUG };
		DXCWICA NoOptimize  { DXC_ARG_SKIP_OPTIMIZATIONS };
		DXCWICA MatRowMajor { DXC_ARG_PACK_MATRIX_ROW_MAJOR };
		DXCWICA MatColMajor { DXC_ARG_PACK_MATRIX_COLUMN_MAJOR };
		DXCWICA WarningIsError{ DXC_ARG_WARNINGS_ARE_ERRORS };
		DXCWICA ResourceMayAlias{ DXC_ARG_RESOURCES_MAY_ALIAS };

		DXCWICA BackwardCompact    { L"-Gec" };
		DXCWICA OutputHash         { L"-Fsh" };
		DXCWICA SlimDbg            { L"-Zs" };
		DXCWICA SeperateReflection { L"-Qstrip_reflect" };

		inline constexpr struct
		{
			DXCWICA operator()(::UINT level) const noexcept
			{
				if (level == 0) return DXC_ARG_OPTIMIZATION_LEVEL0;
				if (level == 1) return DXC_ARG_OPTIMIZATION_LEVEL1;
				if (level == 2) return DXC_ARG_OPTIMIZATION_LEVEL2;
				if (level == 3) return DXC_ARG_OPTIMIZATION_LEVEL3;
				else return DXC_ARG_OPTIMIZATION_LEVEL0;
			}
		}
		// Call optimize(optimize level) to get argument.
		Optimize{};

		enum Version 
		{
			V5_0,
			V5_1,
			V5_2,
			V6_0,
			V6_1,
			V6_2,
		};

		inline const wchar_t* VersionString(Version version) 
		{
			switch (version)
			{
			case dxcw::Arguments::V5_0:
				return L"5_0";
			case dxcw::Arguments::V5_1:
				return L"5_1";
			case dxcw::Arguments::V5_2:
				return L"5_2";
			case dxcw::Arguments::V6_0:
				return L"6_0";
			case dxcw::Arguments::V6_1:
				return L"6_1";
			case dxcw::Arguments::V6_2:
				return L"6_2";
			default: return nullptr;
			}
		}
	}

	// @brief Compile parameter.
	namespace Parameters 
	{
		struct pair 
		{
			const wchar_t* Key;
			const wchar_t* Parameter;
		};

		struct generator 
		{
			consteval generator(const wchar_t* value) noexcept
				: Key{value}
			{}
			// WARNING: You have to keep expressing value alive until compile operation is finished.
			// because the api only take the view of giving parameter, 
			// which will cause ub when accessing invaild pointer in compile operation.
			constexpr pair operator()(const wchar_t* value) const noexcept { return { Key, value }; }
			const wchar_t* Key;
		};

		DXCWIC generator Macro     { L"-D" };
		DXCWIC generator EntryName { L"-E" };

		DXCWIC generator DebugFilePath { L"-Fd" };
		DXCWIC generator OutputPath    { L"-Fo" };
		DXCWIC generator HlslVersion   { L"-HV" };
		DXCWIC generator Target        { L"-T"  };
	}

	struct Compiler;

	struct Builder 
	{
	public:
		friend struct Compiler;
		friend Builder CreateArgumentBuilder(Compiler& compiler, ::LPCWSTR source, LPCWSTR entryName, 
			::D3D12_SHADER_VERSION_TYPE type, Arguments::Version version) noexcept;

		~Builder() 
		{ if(m_Builder) m_Builder->Release(); }

		bool Vaild() const { return m_Builder; }

		auto&& Add(::LPCWSTR value) 
		{
			m_Builder->AddArguments(&value, 1u);
			return *this;
		}

		auto&& Add(Parameters::pair value) 
		{
			LPCWSTR values[]{ value.Key, value.Parameter };
			m_Builder->AddArguments(values, 2u);
			return *this;
		}

		// reserved for extension.
		template<typename T>
		auto&& Add(T&& values);

		auto&& Add(span<::DxcDefine> defines) 
		{
		
			return *this;
		}
		::IDxcCompilerArgs* operator->() const { return m_Builder; }
	private:
		Builder() = default;

		::LPCWSTR m_Path;
		::IDxcCompilerArgs* m_Builder{};
	};

	struct Compiler 
	{
		friend inline Compiler CreateCompiler() noexcept;

		Compiler(const Compiler&) = delete;
		Compiler& operator=(const Compiler&) = delete;

		Compiler(Compiler&& other)
		{ ::MoveMemory(this, &other, sizeof(Compiler)); }
		Compiler& operator=(Compiler&& other) 
		{ ::MoveMemory(this, &other, sizeof(Compiler)); }

		~Compiler() 
		{
			if(m_Compiler)  m_Compiler->Release();
			if(m_Vaildator) m_Vaildator->Release();
			if(m_Utils)     m_Utils->Release();
		}

		bool Vaild() const { return m_Compiler != nullptr; }

		auto Utils() const { return m_Utils; }
		auto Compile(Builder const& info, ::IDxcIncludeHandler* include = nullptr)
		{	
			struct result 
			{
				~result() 
				{ 
					if (Message)
						::HeapFree(::GetProcessHeap(), NULL, Message); 
					if (Pdb)
						Pdb->Release();
					if (ShaderBlob)
						ShaderBlob->Release();
					if(PdbPath)
						::HeapFree(::GetProcessHeap(), NULL, PdbPath);
					if (DasmCode)
						DasmCode->Release();
					if (HlslCode)
						HlslCode->Release();
					if (ShaderReflection)
						ShaderReflection->Release();
					if (RsoBlob)
						RsoBlob->Release();
				}

				char*   Message;
				HRESULT Hr;

				::IDxcBlob* ShaderBlob    {nullptr};
				::IDxcBlob* Pdb           {nullptr};
				wchar_t* PdbPath          {nullptr};
				::IDxcExtraOutputs* Extra {nullptr};
				::DxcShaderHash Hash      {};
				// DasmCode.
				::IDxcBlobEncoding* DasmCode{nullptr};
				// TODO: havent been verified.
				::IDxcBlobEncoding* HlslCode{nullptr};
				// Text.
				::IDxcBlobEncoding* Text    {nullptr};
				// Shader reflection.
				::ID3D12ShaderReflection* ShaderReflection{nullptr};
				// Rootsignature blob.
				::IDxcBlob* RsoBlob{ nullptr }; 
			} result{};
		#if DXCW_DEBUG
			if (!info.EntryNameSetted) return false;
		#endif // DXCW_DEBUG

			::BOOL knownCodePage{ false };
			::UINT codePage{ DXC_CP_ACP };
			guard<::IDxcBlobEncoding> encoding{nullptr};
			result.Hr = m_Utils->LoadFile(info.m_Path, &codePage, encoding.put());

			info.m_Builder->GetArguments();
			if (FAILED(result.Hr)) return result;

			::DxcBuffer buffer{ encoding.ptr->GetBufferPointer(), encoding.ptr->GetBufferSize(), codePage };

			if (!include)
				m_Utils->CreateDefaultIncludeHandler(&include);
			else
				include->AddRef();

			guard _{ include };
			guard<::IDxcResult> resultDxc;
			result.Hr = m_Compiler->Compile(&buffer, 
				info->GetArguments(), info->GetCount(), include, 
				IID_PPV_ARGS(resultDxc.put()));

			if (FAILED(result.Hr))
			{
				if (resultDxc.ptr->HasOutput(DXC_OUT_ERRORS))
				{
					guard<::IDxcBlobEncoding> error;
					resultDxc.ptr->GetErrorBuffer(error.put());

					auto size = error.ptr->GetBufferSize();
					result.Message = static_cast<char*>(::HeapAlloc(::GetProcessHeap(), NULL, error.ptr->GetBufferSize()));
					(void)::MoveMemory(result.Message, error.ptr->GetBufferPointer(), size);
				}

				return result;
			}

			auto numOfOuput = resultDxc.ptr->GetNumOutputs();
			for (auto i{ 0u }; i < numOfOuput; i++) 
			{
				auto type = resultDxc.ptr->GetOutputByIndex(i);

				switch (type)
				{
				case DXC_OUT_OBJECT:
					resultDxc.ptr->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&result.ShaderBlob), nullptr);
					break;
				case DXC_OUT_PDB:
				{
					::IDxcBlobWide* wide{nullptr};
					resultDxc.ptr->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&result.Pdb), &wide);
					if (wide) 
					{
						result.PdbPath = (wchar_t*)::HeapAlloc(::GetProcessHeap(), NULL, wide->GetBufferSize());
						::MoveMemory(result.PdbPath, wide->GetBufferPointer(), wide->GetBufferSize());
						wide->Release();
					}
				}break;
				case DXC_OUT_SHADER_HASH:
				{
					::IDxcBlob* blob{nullptr};
					resultDxc.ptr->GetOutput(::DXC_OUT_SHADER_HASH, IID_PPV_ARGS(&blob), nullptr);
					if (blob) 
					{
						result.Hash = *static_cast<::DxcShaderHash*>(blob->GetBufferPointer());
						blob->Release();
					}
				}break;
				case DXC_OUT_DISASSEMBLY:
					resultDxc.ptr->GetOutput(::DXC_OUT_DISASSEMBLY, IID_PPV_ARGS(&result.DasmCode), nullptr);
				break;
				case DXC_OUT_HLSL:
					resultDxc.ptr->GetOutput(::DXC_OUT_HLSL, IID_PPV_ARGS(&result.HlslCode), nullptr);
				break;
				case DXC_OUT_TEXT:
					resultDxc.ptr->GetOutput(::DXC_OUT_TEXT, IID_PPV_ARGS(&result.Text), nullptr);
				break;
				case DXC_OUT_REFLECTION:
				{
					::IDxcBlobEncoding* blob{nullptr};
					resultDxc.ptr->GetOutput(::DXC_OUT_REFLECTION, IID_PPV_ARGS(&blob), nullptr);
					if (blob) 
					{
						::DxcBuffer buffer{ blob->GetBufferPointer(), blob->GetBufferSize() };
						blob->GetEncoding(&knownCodePage, &buffer.Encoding);
						m_Utils->CreateReflection(&buffer, IID_PPV_ARGS(&result.ShaderReflection));
					}
				}break;
				case DXC_OUT_ROOT_SIGNATURE:
					resultDxc.ptr->GetOutput(::DXC_OUT_REFLECTION, IID_PPV_ARGS(&result.ShaderBlob), nullptr);
				break;
				case DXC_OUT_EXTRA_OUTPUTS:
					resultDxc.ptr->GetOutput(::DXC_OUT_EXTRA_OUTPUTS, IID_PPV_ARGS(&result.Extra), nullptr);
				break;
				default:break;
				}
			}

			guard<::IDxcOperationResult> vaildateResult;
		#if DXCW_DEBUG
			result.Hr = m_Vaildator->ValidateWithDebug(result.ShaderBlob, NULL, nullptr, vaildateResult.put());
		#else
			result.Hr = m_Vaildator->Validate(result.ShaderBlob, NULL, vaildateResult.put());
		#endif
			if (FAILED(result.Hr)) 
			{
				result.ShaderBlob->Release(); result.ShaderBlob = nullptr; 

				guard<::IDxcBlobEncoding> error;
				vaildateResult.ptr->GetErrorBuffer(error.put());
				if (error.ptr) 
				{
					auto size = error.ptr->GetBufferSize();
					result.Message = (char*)::HeapAlloc(::GetProcessHeap(), NULL, size);
					::MoveMemory(result.Message, error.ptr->GetBufferPointer(), size);
				}
			}

			return result;
		}
	private:
		constexpr Compiler() = default;
	private:
		::IDxcCompiler3* m_Compiler {nullptr};
		::IDxcUtils*     m_Utils;
		::IDxcValidator2* m_Vaildator;
	};

	inline Builder CreateArgumentBuilder(Compiler& compiler, ::LPCWSTR source, LPCWSTR entryName, 
		::D3D12_SHADER_VERSION_TYPE type, Arguments::Version version) noexcept
	{
		const auto typeStr{ ShaderTypeLowerCase(type) };
		const auto versionStr{ Arguments::VersionString(version) };

		wchar_t profile[16]{};
		wsprintfW(profile, L"%s_%s", typeStr, versionStr);

		Builder builder{};
		builder.m_Path = source;
		compiler.Utils()->BuildArguments(source, entryName, profile, nullptr, 0u, nullptr, 0u, &builder.m_Builder);

		return builder;
	}

	inline Compiler CreateCompiler() noexcept
	{
		Compiler result;

		if (FAILED(::DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&result.m_Utils))))
			return result;

		if (FAILED(::DxcCreateInstance(CLSID_DxcValidator, IID_PPV_ARGS(&result.m_Vaildator))))
		{
			result.m_Utils->Release();
			return result;
		}

		if (FAILED(::DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&result.m_Compiler))))
		{
			result.m_Utils->Release();
			result.m_Vaildator->Release();

			return result;
		}

		return result;
	}

#undef DXCWISC 
#undef DXCWICA
#undef DXCWIC
}

#undef DXCW_IMPORT
#undef DXCW_MODULE
#undef DXCW_EXPORT

#endif