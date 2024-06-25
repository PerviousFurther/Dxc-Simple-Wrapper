
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

#pragma comment(lib, "..\DXC\lib\x64\dxcompiler.lib")

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
		span(T* pointer, ::UINT count) : Value{ pointer }, Count{ count } {}

		template<typename...Args>
		span(Args&&...);

		T* const Value;
		::UINT const Count;
	};

	template<typename T>
	struct guard
	{
		~guard() { if (ptr) ptr->Release(); }

		T** put() { return &ptr; }

		T* ptr{ nullptr };
	};


	// @brief Compile Argument.
	// Dirty constants namesapce.
	namespace Arguments
	{
		// TODO: future support?
		//DXCWICA NoVaildation{DXC_ARG_SKIP_VALIDATION};

		DXCWICA Debug{ DXC_ARG_DEBUG };
		DXCWICA NoOptimize{ DXC_ARG_SKIP_OPTIMIZATIONS };
		DXCWICA MatRowMajor{ DXC_ARG_PACK_MATRIX_ROW_MAJOR };
		DXCWICA MatColMajor{ DXC_ARG_PACK_MATRIX_COLUMN_MAJOR };
		DXCWICA WarningIsError{ DXC_ARG_WARNINGS_ARE_ERRORS };
		DXCWICA ResourceMayAlias{ DXC_ARG_RESOURCES_MAY_ALIAS };

		DXCWICA BackwardCompact{ L"-Gec" };
		DXCWICA OutputHash{ L"-Fsh" };
		DXCWICA SlimDbg{ L"-Zs" };
		DXCWICA SeperateReflection{ L"-Qstrip_reflect" };

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
		DXCWICA GenerateSpirvFile{L"-spirv"};
	}

	// @brief Compile parameter.
	namespace Parameters
	{
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
			case V5_0:
				return L"5_0";
			case V5_1:
				return L"5_1";
			case V5_2:
				return L"5_2";
			case V6_0:
				return L"6_0";
			case V6_1:
				return L"6_1";
			case V6_2:
				return L"6_2";
			default: return nullptr;
			}
		}

		struct pair
		{
			~pair()
			{
				if (NotCParameter) ::HeapFree(::GetProcessHeap(), NULL, NotCParameter);
			}

			const wchar_t* Key;

			const wchar_t* Parameter;
			wchar_t* NotCParameter;
		};

		struct generator
		{
			consteval generator(const wchar_t* value) noexcept
				: Key{ value }
			{}
			// WARNING: You have to keep expressing value alive until compile operation is finished.
			// because the api only take the view of giving parameter, 
			// which will cause ub when accessing invaild pointer in compile operation.
			pair operator()(const wchar_t* value) const noexcept { return { Key, value, nullptr }; }
			const wchar_t* Key;
		};

		DXCWIC generator Macro{ L"-D" };
		DXCWIC generator EntryName{ L"-E" };

		DXCWIC generator DebugFilePath{ L"-Fd" };
		DXCWIC generator OutputPath{ L"-Fo" };
		DXCWIC generator HlslVersion{ L"-HV" };
		DXCWIC struct target_generator
		{
			pair operator()(auto...) const noexcept;
			::LPCWSTR Key;
		}
		Target{ L"-T" };

		template<>
		inline pair target_generator::operator()(::D3D12_SHADER_VERSION_TYPE type, Version version) const noexcept
		{
			pair result{ Key };

			auto typeName = ShaderTypeLowerCase(type);
			auto versionName = ShaderTypeLowerCase(type);

			result.NotCParameter = (LPWSTR)::HeapAlloc(::GetProcessHeap(), NULL, ::wcslen(typeName) + ::wcslen(versionName));

			(void)::wsprintfW(result.NotCParameter, L"%s_%s", typeName, versionName);

			return result;
		}
	}

	struct Builder
	{
		friend struct Utils;
		friend struct Compiler;
	public:
		//friend Builder CreateArgumentBuilder(Utils& compiler, ::LPCWSTR source, LPCWSTR entryName,
		//	::D3D12_SHADER_VERSION_TYPE type, Parameters::Version version) noexcept;

		Builder(Builder const& other) :
			m_Path{ other.m_Path }, m_Builder{ other.m_Builder }
		{
			m_Builder->AddRef();
		}

		Builder& operator=(Builder const& other)
		{
			if (m_Builder)
				m_Builder->AddRef();
			m_Builder->AddRef();
			return *this;
		}

		Builder(Builder&& other)
		{
			::MoveMemory(&m_Path, &other.m_Path, sizeof(m_Path));
			::MoveMemory(&m_Builder, &other.m_Builder, sizeof(m_Builder));
		}

		Builder& operator=(Builder&& other)
		{
			if (m_Builder) m_Builder->Release();
			::MoveMemory(&m_Path, &other.m_Path, sizeof(m_Path));
			::MoveMemory(&m_Builder, &other.m_Builder, sizeof(m_Builder));

			return *this;
		}

		~Builder()
		{
			if (m_Builder) m_Builder->Release();
		}

		bool Vaild() const { return m_Builder; }

		// @param value: use dirty flags in namespace Arguments.
		auto&& Add(::LPCWSTR value)
		{
#if DXCW_DEBUG
			// TODO: runtime checking?
#endif
			m_Builder->AddArguments(&value, 1u);
			return *this;
		}

		auto&& Add(Parameters::pair value)
		{
#if DXCW_DEBUG
			EntryNameSet = !::memcmp(Parameters::EntryName.Key, value.Key, sizeof("-E"));
#endif
			LPCWSTR values[]{ value.Key, value.Parameter };
			m_Builder->AddArguments(values, 2u);
			return *this;
		}

		// reserved for extension.
		template<typename T>
		auto&& Add(T&& values);

		auto&& Add(span<::DxcDefine> defines)
		{
			m_Builder->AddDefines(defines.Value, defines.Count);
			return *this;
		}
		::IDxcCompilerArgs* operator->() const { return m_Builder; }
	private:
		Builder() = default;
#if DXCW_DEBUG
		bool EntryNameSet{ false };
		bool DisableVaildate{ false };
#endif // DXCW_DEBUG

		::LPCWSTR m_Path{ nullptr };
		::IDxcCompilerArgs* m_Builder{ nullptr };
	};

	struct Utils
	{
		friend Utils CreateUtils();

		Utils(Utils&& other) { ::MoveMemory(&m_Utils, &other.m_Utils, sizeof(m_Utils)); };
		Utils& operator=(Utils&& other) = delete;

		Utils(Utils const&) = delete;
		Utils& operator=(Utils const&) = delete;

		~Utils() { if (m_Utils) m_Utils->Release(); }

		bool Vaild() const { return m_Utils; }

		auto operator->() const { return m_Utils; }

		// @param source: See this parameter as an file of hlsl. 
		// WARNING: The builder only view at your giving parameter and wont copy.
		[[nodiscard("No need to create builder.")]]
		Builder CreateBuilder(::LPCWSTR source, ::LPCWSTR entryName, ::D3D12_SHADER_VERSION_TYPE type, Parameters::Version version)
		{
			const auto typeStr{ ShaderTypeLowerCase(type) };
			const auto versionStr{ Parameters::VersionString(version) };

			wchar_t profile[16]{};
			::wsprintfW(profile, L"%s_%s", typeStr, versionStr);

			Builder builder{};
			builder.m_Path = source;
			m_Utils->BuildArguments(source, entryName, profile, nullptr, 0u, nullptr, 0u, &builder.m_Builder);
			return builder;
		}

	private:
		Utils() : m_Utils{ nullptr } {}

		::IDxcUtils* m_Utils;
	};

	// Result bundle, 
	// recommand using std::exchange((What you wanna get), nullptr) 
	// or
	// MoveMemory((dst parameter), (What you wanna get), sizeof((What you wanna get))).
	struct CompileResult
	{
		//#define DXCW_MEM_MOVE(name) ::MoveMemory(&name, &other.name, sizeof(name))

		CompileResult() = default;

		CompileResult(CompileResult&& other)
		{
			::MoveMemory(this, &other, sizeof(CompileResult));
		}
		CompileResult& operator=(CompileResult&& other)
		{
			CleanUp();
			::MoveMemory(this, &other, sizeof(CompileResult));
			return *this;
		}

		//#undef DXCW_MEM_MOVE

		~CompileResult() noexcept { CleanUp(); }

		void CleanUp()
		{
			if (Message)
			{
				(void)::HeapFree(::GetProcessHeap(), NULL, Message);
				Message = nullptr;
			}
			if (Pdb)
			{
				Pdb->Release();
				Pdb = nullptr;
			}
			if (ShaderBlob)
			{
				ShaderBlob->Release();
				ShaderBlob = nullptr;
			}
			if (PdbPath)
			{
				(void)::HeapFree(::GetProcessHeap(), NULL, PdbPath);
				PdbPath = nullptr;
			}
			if (DasmCode)
			{
				DasmCode->Release();
				DasmCode = nullptr;
			}
			if (HlslCode)
			{
				HlslCode->Release();
				HlslCode = nullptr;
			}
			if (ShaderReflection)
			{
				ShaderReflection->Release();
				ShaderReflection = nullptr;
			}
			if (RsoBlob)
			{
				RsoBlob->Release();
				RsoBlob = nullptr;
			}
		}

		// this method is reserved for extension method.
		auto Obtain(auto...) const;

		bool Check(::std::nullptr_t) const { return!::strlen(Message); }
		bool Check() const { return SUCCEEDED(Hr); }

		// Operation message.
		// Normally about shader.
		// Sometime will be about vaildation.
		char* Message{ nullptr };
		// Operation last error code.
		HRESULT Hr{ S_OK };

		// Shader byte code.
		::IDxcBlob* ShaderBlob{ nullptr };
		// Pdb.
		::IDxcBlob* Pdb{ nullptr };
		wchar_t* PdbPath{ nullptr };
		// Extra output.
		::IDxcExtraOutputs* Extra{ nullptr };
		// Shader hash.
		::DxcShaderHash Hash{};
		// DasmCode.
		::IDxcBlobEncoding* DasmCode{ nullptr };
		// TODO: havent been verified.
		::IDxcBlobEncoding* HlslCode{ nullptr };
		// Text.
		::IDxcBlobEncoding* Text{ nullptr };
		// Shader reflection.
		::ID3D12ShaderReflection* ShaderReflection{ nullptr };
		// Rootsignature blob.
		::IDxcBlob* RsoBlob{ nullptr };
	};

	struct Compiler
	{
		friend inline Compiler CreateCompiler() noexcept;

		Compiler(const Compiler&) = delete;
		Compiler& operator=(const Compiler&) = delete;

		Compiler(Compiler&& other)
		{
			::MoveMemory(this, &other, sizeof(Compiler));
		}
		Compiler& operator=(Compiler&& other)
		{
			::MoveMemory(this, &other, sizeof(Compiler));
			return *this;
		}

		~Compiler()
		{
			if (m_Compiler)  m_Compiler->Release();
			if (m_Vaildator) m_Vaildator->Release();
		}

		// Check whether the compiler is online.
		bool Vaild() const { return m_Compiler != nullptr; }

		// include is reserved, maybe encapsulate in future.
		[[nodiscard("No need to call Compile().")]]
		CompileResult Compile(Utils& utils, Builder const& info, ::IDxcIncludeHandler* include = nullptr) noexcept
		{
			CompileResult result;

			::BOOL knownCodePage{ false };
			::UINT codePage{ DXC_CP_ACP };
			guard<::IDxcBlobEncoding> encoding{ nullptr };
			result.Hr = utils->LoadFile(info.m_Path, &codePage, encoding.put());

			info.m_Builder->GetArguments();
			if (FAILED(result.Hr)) return result;

			::DxcBuffer buffer{ encoding.ptr->GetBufferPointer(), encoding.ptr->GetBufferSize(), codePage };

			if (!include)
				utils->CreateDefaultIncludeHandler(&include);
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
					::IDxcBlobWide* wide{ nullptr };
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
					::IDxcBlob* blob{ nullptr };
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
					::IDxcBlobEncoding* blob{ nullptr };
					resultDxc.ptr->GetOutput(::DXC_OUT_REFLECTION, IID_PPV_ARGS(&blob), nullptr);
					if (blob)
					{
						::DxcBuffer buffer{ blob->GetBufferPointer(), blob->GetBufferSize() };
						blob->GetEncoding(&knownCodePage, &buffer.Encoding);
						utils->CreateReflection(&buffer, IID_PPV_ARGS(&result.ShaderReflection));
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

		template<typename Transform, typename Utils, typename Builder, typename IncludeHandler>
		[[nodiscard("No need to call CompileAndTransform().")]]
		auto CompileAndTransform(Utils&& utils, Builder&& builder, IncludeHandler&& handler, Transform&& transform)
#if defined(__cpp_noexcept_function_type) && defined(__cpp_lib_type_trait_variable_templates)
			noexcept(::std::is_nothrow_invocable<Transform, CompileResult&&>)
#endif
#if defined(__cpp_lib_type_trait_variable_templates)
			requires::std::invocable<decltype(&Compiler::Compile), Compiler, Utils&&, Builder&&, IncludeHandler&&>
		&& ::std::invocable<Transform, CompileResult&&>
#else
			requires requires{ this->Compile(utils, builder, handler); transform(CompileResult{});  }
#endif
		{ return transform(Compile(builder, handler)); }
	private:
		constexpr Compiler() = default;
	private:
		::IDxcCompiler3* m_Compiler{ nullptr };
		::IDxcValidator2* m_Vaildator;
	};

	[[nodiscard("No need to create compiler.")]]
	inline Utils CreateUtils()
	{
		Utils result;
		::DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&result.m_Utils));
		return result;
	}

	[[nodiscard("No need to create compiler.")]]
	inline Compiler CreateCompiler() noexcept
	{
		Compiler result;
		if (FAILED(::DxcCreateInstance(CLSID_DxcValidator, IID_PPV_ARGS(&result.m_Vaildator))))
		{
			return result;
		}

		if (FAILED(::DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&result.m_Compiler))))
		{
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