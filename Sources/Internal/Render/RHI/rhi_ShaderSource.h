#ifndef __RHI_SHADERSOURCE_H__
#define __RHI_SHADERSOURCE_H__

    #include "rhi_Type.h"
    #include "Base/BaseTypes.h"    
    #include "Base/FastName.h"

namespace DAVA
{
class File;
}

namespace sl
{
class HLSLTree;
}

namespace rhi
{
using DAVA::uint32;
using DAVA::FastName;
struct
ShaderProp
{
    enum Type
    {
        TYPE_FLOAT1,
        TYPE_FLOAT2,
        TYPE_FLOAT3,
        TYPE_FLOAT4,
        TYPE_FLOAT4X4
    };

    enum Precision
    {
        PRECISION_NORMAL,
        PRECISION_HALF,
        PRECISION_LOW
    };
    //    enum Scope  { SCOPE_UNIQUE, SCOPE_SHARED };
    enum Storage
    {
        STORAGE_STATIC,
        STORAGE_DYNAMIC
    };

    FastName uid;
    Type type;
    Precision precision;
    uint32 arraySize;
    //    Scope       scope;
    Storage storage;
    FastName tag;
    uint32 bufferindex;
    uint32 bufferReg;
    uint32 bufferRegCount;
    uint32 isBigArray : 1;
    float defaultValue[16];
};

typedef std::vector<ShaderProp> ShaderPropList;

struct
ShaderSampler
{
    TextureType type;
    FastName uid;
};

typedef std::vector<ShaderSampler> ShaderSamplerList;

class
ShaderSource
{
public:
    ShaderSource(const char* filename = "");
    ~ShaderSource();

    bool Construct(ProgType progType, const char* srcText, const std::vector<std::string>& defines);
    bool Construct(ProgType progType, const char* srcText);
    bool Load(Api api, DAVA::File* in);
    bool Save(Api api, DAVA::File* out) const;

    const std::string& GetSourceCode(Api targetApi) const;
    const ShaderPropList& Properties() const;
    const ShaderSamplerList& Samplers() const;
    const VertexLayout& ShaderVertexLayout() const;
    uint32 ConstBufferCount() const;
    uint32 ConstBufferSize(uint32 bufIndex) const;
    //    ShaderProp::Scope       ConstBufferScope( uint32 bufIndex ) const;
    ShaderProp::Storage ConstBufferStorage(uint32 bufIndex) const;
    BlendState Blending() const;

    void Dump() const;

private:
    void Reset();
    void ProcessMetaData(sl::HLSLTree* ast);

    struct
    buf_t
    {
        //        ShaderProp::Scope   scope;
        ShaderProp::Storage storage;
        FastName tag;
        uint32 regCount;
        std::vector<int> avlRegIndex;
        bool isArray;
    };

    std::string fileName;
    sl::HLSLTree* ast;
    mutable std::string code[RHI_COUNT];

    ProgType type;
    uint32 codeLineCount;
    VertexLayout vertexLayout;
    std::vector<ShaderProp> property;
    std::vector<buf_t> buf;
    std::vector<ShaderSampler> sampler;
    BlendState blending;
};

class
ShaderSourceCache
{
public:
    static const ShaderSource* Get(FastName uid, uint32 srcHash);
    static const ShaderSource* Add(const char* filename, FastName uid, ProgType progType, const char* srcText, const std::vector<std::string>& defines);

    static void Clear();
    static void Save(const char* fileName);
    static void Load(const char* fileName);

private:
    struct
    entry_t
    {
        FastName uid;
        uint32 api;
        uint32 srcHash;
        ShaderSource* src;
    };

    static std::vector<entry_t> Entry;
    static const uint32 FormatVersion;
};

} // namespace rhi

#endif // __RHI_SHADERSOURCE_H__
