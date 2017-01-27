#include "lite_archive.hxx"

#include "lz4.h"

static bool LZ4CompressorDecompress(const std::vector<uint8_t>& in,
                                    std::vector<uint8_t>& out)
{
    int32_t decompressResult = LZ4_decompress_fast(
    reinterpret_cast<const char*>(in.data()),
    reinterpret_cast<char*>(out.data()),
    static_cast<uint32_t>(out.size()));
    if (decompressResult < 0)
    {
        return false;
    }
    return true;
}

lite_archive::lite_archive(const std::string& file)
    : file_name(file)
{
    ifile.open(file_name, std::ios_base::binary);

    l << "archive is opening\n";

    ifile.seekg(-sizeof(footer), std::ios_base::end);

    ifile.read(reinterpret_cast<char*>(&footer), sizeof(footer));

    l << "reading footer\n";

    if (!ifile)
    {
        throw std::runtime_error("can't create lite_archive");
    }

    pack_format::file_info info;
    info.compressedSize = footer.size_compressed;
    info.compressionType = footer.type;
    info.hash = footer.crc32_compressed;
    info.originalSize = footer.size_uncompressed;

    std::string relative = file_name.substr(file_name.find_last_of("/\\") + 1, std::string::npos);
    relative = relative.substr(0, relative.size() - 5); // remove ".dvpl" extension

    info.relativeFilePath = relative;

    file_info.push_back(info);

    l << "open done\n";
}

const std::vector<pack_format::file_info>& lite_archive::get_files_info() const
{
    return file_info;
}

const pack_format::file_info* lite_archive::get_file_info(const std::string& relative) const
{
    if (relative == file_info[0].relativeFilePath)
    {
        return &file_info[0];
    }
    return nullptr;
}

bool lite_archive::has_file(const std::string& relative) const
{
    return relative == file_info[0].relativeFilePath;
}

bool lite_archive::load_file(const std::string& relative, std::vector<uint8_t>& output)
{
    l << "start loading file: " << relative << '\n';
    if (relative != file_info[0].relativeFilePath)
    {
        l << "no such file!\n";
        return false;
    }
    output.resize(footer.size_uncompressed);

    switch (footer.type)
    {
    case 0:
    {
        ifile.read(reinterpret_cast<char*>(output.data()), footer.size_compressed);
        if (!ifile)
        {
            return false;
        }
    }
    break;
    case 1: // Compressor::Type::Lz4:
    case 2: // Compressor::Type::Lz4HC:
    {
        std::vector<uint8_t> in(footer.size_compressed);

        ifile.seekg(0, std::ios_base::beg);
        ifile.read(reinterpret_cast<char*>(in.data()),
                   footer.size_compressed);
        if (!ifile)
        {
            return false;
        }

        if (!LZ4CompressorDecompress(in, output))
        {
            return false;
        }
    }
    break;
    case 3: // Compressor::Type::RFC1951:
    {
        std::vector<uint8_t> packedBuf(footer.size_compressed);

        ifile.seekg(0, std::ios_base::end);
        ifile.read(reinterpret_cast<char*>(packedBuf.data()),
                   footer.size_compressed);
        if (!ifile)
        {
            return false;
        }

        //TODO if (!ZipCompressor().Decompress(packedBuf, output))

        return false;
    }
    break;
    default:
        return false;
    } // end switch
    return true;
}

bool lite_archive::has_meta() const
{
    return false;
}

const pack_meta_data& lite_archive::get_meta() const
{
    throw std::runtime_error("no meta");
}

std::string lite_archive::print_meta() const
{
    return "no meta";
}
